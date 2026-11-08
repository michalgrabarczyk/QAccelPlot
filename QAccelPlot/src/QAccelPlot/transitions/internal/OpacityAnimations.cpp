//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/transitions/internal/OpacityAnimations.hpp"

#include <QEasingCurve>
#include <QMetaObject>
#include <QMetaProperty>
#include <QObject>
#include <QQmlListReference>
#include <QVariant>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

namespace QAccelPlot::Internal {

namespace {

// Qt Quick animations are private classes, so they are told apart by class name and read through their properties.
constexpr auto kAbstractAnimation = "QQuickAbstractAnimation";
constexpr auto kSequentialAnimation = "QQuickSequentialAnimation";
constexpr auto kParallelAnimation = "QQuickParallelAnimation";
constexpr auto kPauseAnimation = "QQuickPauseAnimation";
constexpr auto kPropertyAnimation = "QQuickPropertyAnimation";
constexpr auto kPropertyAction = "QQuickPropertyAction";
constexpr auto kSpringAnimation = "QQuickSpringAnimation";
constexpr auto kSmoothedAnimation = "QQuickSmoothedAnimation";

/// \brief Most loops read of one animation, so a huge count stays cheap.
constexpr auto kMaxLoops = 1000;

/// \brief Returns the QML type name of \a object: the name of its first Qt Quick class without the \c QQuick prefix.
QString typeName(const QObject& object)
{
    const auto prefix = QStringLiteral("QQuick");
    for (const auto* meta = object.metaObject(); meta; meta = meta->superClass()) {
        const auto name = QString::fromLatin1(meta->className());
        if (name.startsWith(prefix)) {
            return name.mid(prefix.size());
        }
    }
    return QString::fromLatin1(object.metaObject()->className());
}

/// \brief Reads \a property as \a className declares it, ignoring redeclarations in subclasses.
QVariant readDeclared(const QObject& object, const char* className, const char* property)
{
    for (const auto* meta = object.metaObject(); meta; meta = meta->superClass()) {
        if (qstrcmp(meta->className(), className) == 0) {
            return meta->property(meta->indexOfProperty(property)).read(&object);
        }
    }
    return {};
}

/// \brief Returns the opacity in \a value, or \c std::nullopt when it is unset or not a number.
std::optional<double> opacityValue(const QVariant& value, bool& isNumber)
{
    isNumber = true;
    if (!value.isValid()) {
        return std::nullopt;
    }
    auto ok = false;
    const auto number = value.toDouble(&ok);
    isNumber = ok;
    return ok ? std::optional<double>{number} : std::nullopt;
}

/// \brief Returns \c true when \a animation names \c "opacity" in \c property or \c properties.
bool animatesOpacity(const QObject& animation)
{
    const auto opacity = QStringLiteral("opacity");
    if (animation.property("property").toString().trimmed() == opacity) {
        return true;
    }
    const auto properties = animation.property("properties").toString().split(QLatin1Char(','));
    return std::any_of(properties.cbegin(), properties.cend(), [&opacity](const QString& name) { return name.trimmed() == opacity; });
}

/// \brief How an animation places its children in time.
enum class Grouping {
    None,       ///< \brief No children.
    Sequential, ///< \brief Children play one after another.
    Parallel,   ///< \brief Children play together.
};

/// \brief One animation of the tree, read once.
struct Node {
    Grouping grouping{Grouping::None};
    int loops{1};
    double duration{0.0};               ///< \brief Duration of one loop in milliseconds.
    std::optional<OpacitySegment> fade; ///< \brief The opacity change of one loop, starting at 0.
    std::vector<std::size_t> children;  ///< \brief Indices of the children, in declaration order.

    [[nodiscard]] double total() const
    {
        return duration * loops;
    }
};

/// \brief Reads a tree of Qt Quick animations into nodes, then places their opacity changes in time.
///
/// Both passes use explicit stacks, so deep trees cannot overflow the call stack.
class Reader {
public:
    explicit Reader(QStringList* problems)
        : problems_(problems)
    {
    }

    /// \brief Reads the tree under \a root and returns the index of its node.
    std::size_t add(const QObject& root)
    {
        const auto rootIndex = nodes_.size();
        auto pending = std::vector<std::pair<const QObject*, std::optional<std::size_t>>>{{&root, std::nullopt}};
        while (!pending.empty()) {
            const auto [object, parent] = pending.back();
            pending.pop_back();
            const auto index = nodes_.size();
            auto children = QList<const QObject*>{};
            nodes_.push_back(readNode(*object, children));
            if (parent) {
                nodes_[*parent].children.push_back(index);
            }
            // Pushed in reverse, so the children are read, and indexed, in declaration order.
            for (auto child = children.crbegin(); child != children.crend(); ++child) {
                pending.emplace_back(*child, index);
            }
        }
        // Children have higher indices than their parent, so a reverse pass measures them first.
        for (auto index = nodes_.size(); index-- > rootIndex;) {
            measure(nodes_[index]);
        }
        return rootIndex;
    }

    /// \brief Returns the duration of node \a index, with its loops.
    [[nodiscard]] double total(const std::size_t index) const
    {
        return nodes_[index].total();
    }

    /// \brief Appends the opacity changes of node \a root to \a segments, in declaration order.
    void place(const std::size_t root, std::vector<OpacitySegment>& segments) const
    {
        auto pending = std::vector<std::pair<std::size_t, double>>{{root, 0.0}};
        while (!pending.empty()) {
            const auto [index, offset] = pending.back();
            pending.pop_back();
            const auto& node = nodes_[index];
            auto starts = std::vector<std::pair<std::size_t, double>>{};
            for (auto loop = 0; loop < node.loops; ++loop) {
                const auto loopStart = offset + loop * node.duration;
                if (node.fade) {
                    auto segment = *node.fade;
                    segment.start = loopStart;
                    segments.push_back(std::move(segment));
                }
                auto childStart = loopStart;
                for (const auto child : node.children) {
                    starts.emplace_back(child, childStart);
                    if (node.grouping == Grouping::Sequential) {
                        childStart += nodes_[child].total();
                    }
                }
            }
            // Pushed in reverse, so changes that start together keep their declaration order.
            pending.insert(pending.end(), starts.crbegin(), starts.crend());
        }
    }

private:
    Node readNode(const QObject& animation, QList<const QObject*>& children)
    {
        auto node = Node{};
        if (!animation.inherits(kAbstractAnimation)) {
            report(QStringLiteral("%1 is not an animation and is ignored").arg(typeName(animation)));
            return node;
        }
        node.loops = loopCount(animation);
        // Spring and smoothed animations derive from NumberAnimation, but follow their target instead of a timeline.
        const auto followsTarget = animation.inherits(kSpringAnimation) || animation.inherits(kSmoothedAnimation);
        if (animation.inherits(kSequentialAnimation) || animation.inherits(kParallelAnimation)) {
            node.grouping = animation.inherits(kSequentialAnimation) ? Grouping::Sequential : Grouping::Parallel;
            const auto list = QQmlListReference(const_cast<QObject*>(&animation), "animations");
            for (auto index = qsizetype{0}; index < list.count(); ++index) {
                if (const auto* child = list.at(index)) {
                    children.append(child);
                }
            }
        } else if (animation.inherits(kPauseAnimation)) {
            node.duration = std::max(animation.property("duration").toDouble(), 0.0);
        } else if (animation.inherits(kPropertyAnimation) && !followsTarget) {
            node.duration = std::max(animation.property("duration").toDouble(), 0.0);
            node.fade = propertyAnimationFade(animation, node.duration);
        } else if (animation.inherits(kPropertyAction)) {
            node.fade = propertyActionFade(animation);
        } else {
            report(QStringLiteral("%1 cannot play in a transition and is ignored").arg(typeName(animation)));
        }
        return node;
    }

    int loopCount(const QObject& animation)
    {
        const auto loops = animation.property("loops").toInt();
        // Qt stores Animation.Infinite as a negative count.
        if (loops < 0) {
            report(QStringLiteral("%1 with infinite loops plays once in a transition").arg(typeName(animation)));
            return 1;
        }
        return std::clamp(loops, 1, kMaxLoops);
    }

    std::optional<OpacitySegment> propertyAnimationFade(const QObject& animation, const double duration)
    {
        if (!animatesOpacity(animation)) {
            reportOtherProperty(animation);
            return std::nullopt;
        }
        auto fromIsNumber = true;
        auto toIsNumber = true;
        const auto from = opacityValue(readDeclared(animation, kPropertyAnimation, "from"), fromIsNumber);
        const auto to = opacityValue(readDeclared(animation, kPropertyAnimation, "to"), toIsNumber);
        if (!fromIsNumber || !toIsNumber) {
            report(QStringLiteral("%1 of \"opacity\" needs numbers for from and to; it only adds its duration").arg(typeName(animation)));
            return std::nullopt;
        }
        return OpacitySegment{0.0, duration, from, to.value_or(1.0), animation.property("easing").value<QEasingCurve>()};
    }

    std::optional<OpacitySegment> propertyActionFade(const QObject& action)
    {
        if (!animatesOpacity(action)) {
            reportOtherProperty(action);
            return std::nullopt;
        }
        auto isNumber = true;
        const auto value = opacityValue(action.property("value"), isNumber);
        if (!isNumber) {
            report(QStringLiteral("%1 of \"opacity\" needs a number for value and is ignored").arg(typeName(action)));
            return std::nullopt;
        }
        return OpacitySegment{0.0, 0.0, std::nullopt, value.value_or(1.0), QEasingCurve{}};
    }

    void measure(Node& node) const
    {
        if (node.grouping == Grouping::None) {
            return;
        }
        auto duration = 0.0;
        for (const auto child : node.children) {
            const auto childTotal = nodes_[child].total();
            duration = node.grouping == Grouping::Sequential ? duration + childTotal : std::max(duration, childTotal);
        }
        node.duration = duration;
    }

    void reportOtherProperty(const QObject& animation)
    {
        report(QStringLiteral("%1 does not animate \"opacity\", the only property a transition animates; it only adds its duration").arg(typeName(animation)));
    }

    void report(const QString& problem)
    {
        if (problems_) {
            problems_->append(problem);
        }
    }

    QStringList* problems_;
    std::vector<Node> nodes_;
};

} // namespace

OpacityTimeline readOpacityAnimations(const QList<QObject*>& animations, QStringList* problems)
{
    auto reader = Reader{problems};
    auto segments = std::vector<OpacitySegment>{};
    auto length = 0.0;
    for (const auto* animation : animations) {
        if (animation) {
            const auto root = reader.add(*animation);
            reader.place(root, segments);
            length = std::max(length, reader.total(root));
        }
    }
    return OpacityTimeline{std::move(segments), length};
}

} // namespace QAccelPlot::Internal
