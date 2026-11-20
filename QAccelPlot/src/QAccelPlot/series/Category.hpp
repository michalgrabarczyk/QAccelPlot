//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QColor>
#include <QObject>
#include <QString>

#if __has_include(<QtQmlIntegration/qqmlintegration.h>)
#include <QtQmlIntegration/qqmlintegration.h>
#else
#include <QtQml/qqmlregistration.h>
#endif

namespace QAccelPlot {

/// \brief One category of a \c BarSeries or \c RectangleSeries: a value, a name, and a fill color.
///
/// Data points carry the category \c value as their \c category. The \c name labels the category
/// in the legend and in \c barAt() / \c rectangleAt(), and QML data can use it instead of the value.
///
/// \code
/// QAccelPlot.RectangleSeries {
///     categories: [
///         QAccelPlot.Category { value: 0; name: "Running"; color: "seagreen" },
///         QAccelPlot.Category { value: 1; name: "Alarm"; color: "firebrick" }
///     ]
/// }
/// \endcode
///
/// \sa BarSeries::categories, RectangleSeries::categories
class Category : public QObject {
    Q_OBJECT
    QML_NAMED_ELEMENT(Category)

    /// \brief Value that data points carry as their \c category. Default: 0.
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
    /// \brief Name for the legend, \c categoryName, and QML data. Default: empty.
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    /// \brief Fill color. Unset by default: each series that lists the category uses its own \c color.
    ///
    /// Reads an invalid color while unset; assign \c undefined to unset it.
    Q_PROPERTY(QColor color READ color WRITE setColor RESET resetColor NOTIFY colorChanged)

public:
    /// \brief Constructs a category with value 0, no name, and no color, owned by \a parent.
    explicit Category(QObject* parent = nullptr);
    /// \brief Constructs a category with \a value, \a name, and \a color, owned by \a parent.
    Category(int value, QString name, const QColor& color, QObject* parent = nullptr);

    /// \brief Returns the value data points carry.
    int value() const;
    /// \brief Sets the value data points carry to \a value.
    void setValue(int value);

    /// \brief Returns the name.
    QString name() const;
    /// \brief Sets the name to \a name.
    void setName(const QString& name);

    /// \brief Returns the fill color, or an invalid color when the series \c color applies.
    QColor color() const;
    /// \brief Sets the fill color to \a color. An invalid color uses the series \c color.
    void setColor(const QColor& color);
    /// \brief Unsets the fill color, so the series \c color applies again.
    void resetColor();

signals:
    /// \brief Emitted when the value property changes.
    void valueChanged();
    /// \brief Emitted when the name property changes.
    void nameChanged();
    /// \brief Emitted when the color property changes.
    void colorChanged();

private:
    int value_{0};
    QString name_;
    QColor color_;
};

} // namespace QAccelPlot
