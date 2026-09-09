//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QColor>
#include <QObject>
#include <QtQml/qqmlregistration.h>

namespace QAccelPlot {

/// \brief Configuration object that controls the appearance of the plot grid.
///
/// Exposed as a CONSTANT property on \c PlotView. Major grid lines are drawn
/// at the same positions as the X/Y axis ticks; sub-grid lines are drawn between them.
///
/// \sa PlotView
class Grid : public QObject {
    Q_OBJECT
    QML_ANONYMOUS
    /// \brief Width in pixels of the major grid lines. Default: 2.
    Q_PROPERTY(qreal lineWidth READ lineWidth WRITE setLineWidth NOTIFY lineWidthChanged)
    /// \brief Width in pixels of the sub-grid lines. Default: 1.
    Q_PROPERTY(qreal subGridLineWidth READ subGridLineWidth WRITE setSubGridLineWidth NOTIFY subGridLineWidthChanged)
    /// \brief Color of the major grid lines. Default: \c #8c8c8c.
    Q_PROPERTY(QColor gridColor READ gridColor WRITE setGridColor NOTIFY gridColorChanged)
    /// \brief Color of the sub-grid lines. Default: \c #b4b4b4.
    Q_PROPERTY(QColor subGridColor READ subGridColor WRITE setSubGridColor NOTIFY subGridColorChanged)
    /// \brief Whether major grid lines are visible. Default: \c true.
    Q_PROPERTY(bool gridVisible READ gridVisible WRITE setGridVisible NOTIFY gridVisibleChanged)
    /// \brief Whether sub-grid lines are visible. Default: \c true.
    Q_PROPERTY(bool subGridVisible READ subGridVisible WRITE setSubGridVisible NOTIFY subGridVisibleChanged)
    /// \brief Whether horizontal major grid lines are visible. Default: \c true.
    Q_PROPERTY(bool gridHorizontalLinesVisible READ gridHorizontalLinesVisible WRITE setGridHorizontalLinesVisible NOTIFY gridHorizontalLinesVisibleChanged)
    /// \brief Whether vertical major grid lines are visible. Default: \c true.
    Q_PROPERTY(bool gridVerticalLinesVisible READ gridVerticalLinesVisible WRITE setGridVerticalLinesVisible NOTIFY gridVerticalLinesVisibleChanged)
    /// \brief Whether horizontal sub-grid lines are visible. Default: \c true.
    Q_PROPERTY(bool subGridHorizontalLinesVisible READ subGridHorizontalLinesVisible WRITE setSubGridHorizontalLinesVisible NOTIFY
            subGridHorizontalLinesVisibleChanged)
    /// \brief Whether vertical sub-grid lines are visible. Default: \c true.
    Q_PROPERTY(bool subGridVerticalLinesVisible READ subGridVerticalLinesVisible WRITE setSubGridVerticalLinesVisible NOTIFY subGridVerticalLinesVisibleChanged)

public:
    /// \brief Constructs a Grid with the given \a parent.
    explicit Grid(QObject* parent = nullptr);

    /// \brief Returns the major grid line width.
    qreal lineWidth() const;
    /// \brief Sets the major grid line width to \a w.
    void setLineWidth(qreal w);

    /// \brief Returns the sub-grid line width.
    qreal subGridLineWidth() const;
    /// \brief Sets the sub-grid line width to \a w.
    void setSubGridLineWidth(qreal w);

    /// \brief Returns the major grid line color.
    QColor gridColor() const;
    /// \brief Sets the major grid line color to \a c.
    void setGridColor(const QColor& c);

    /// \brief Returns the sub-grid line color.
    QColor subGridColor() const;
    /// \brief Sets the sub-grid line color to \a c.
    void setSubGridColor(const QColor& c);

    /// \brief Returns \c true if major grid lines are visible.
    bool gridVisible() const;
    /// \brief Sets major grid line visibility to \a on.
    void setGridVisible(bool on);

    /// \brief Returns \c true if sub-grid lines are visible.
    bool subGridVisible() const;
    /// \brief Sets sub-grid line visibility to \a on.
    void setSubGridVisible(bool on);

    /// \brief Returns \c true if horizontal major grid lines are visible.
    bool gridHorizontalLinesVisible() const;
    /// \brief Sets horizontal major grid line visibility to \a on.
    void setGridHorizontalLinesVisible(bool on);

    /// \brief Returns \c true if vertical major grid lines are visible.
    bool gridVerticalLinesVisible() const;
    /// \brief Sets vertical major grid line visibility to \a on.
    void setGridVerticalLinesVisible(bool on);

    /// \brief Returns \c true if horizontal sub-grid lines are visible.
    bool subGridHorizontalLinesVisible() const;
    /// \brief Sets horizontal sub-grid line visibility to \a on.
    void setSubGridHorizontalLinesVisible(bool on);

    /// \brief Returns \c true if vertical sub-grid lines are visible.
    bool subGridVerticalLinesVisible() const;
    /// \brief Sets vertical sub-grid line visibility to \a on.
    void setSubGridVerticalLinesVisible(bool on);

signals:
    /// \brief Emitted when the lineWidth property changes.
    void lineWidthChanged();
    /// \brief Emitted when the subGridLineWidth property changes.
    void subGridLineWidthChanged();
    /// \brief Emitted when the gridColor property changes.
    void gridColorChanged();
    /// \brief Emitted when the subGridColor property changes.
    void subGridColorChanged();
    /// \brief Emitted when the gridVisible property changes.
    void gridVisibleChanged();
    /// \brief Emitted when the subGridVisible property changes.
    void subGridVisibleChanged();
    /// \brief Emitted when the gridHorizontalLinesVisible property changes.
    void gridHorizontalLinesVisibleChanged();
    /// \brief Emitted when the gridVerticalLinesVisible property changes.
    void gridVerticalLinesVisibleChanged();
    /// \brief Emitted when the subGridHorizontalLinesVisible property changes.
    void subGridHorizontalLinesVisibleChanged();
    /// \brief Emitted when the subGridVerticalLinesVisible property changes.
    void subGridVerticalLinesVisibleChanged();

private:
    qreal lineWidth_{2.0};
    qreal subGridLineWidth_{1.0};
    QColor gridColor_{"#8c8c8c"};
    QColor subGridColor_{"#b4b4b4"};
    bool gridVisible_{true};
    bool subGridVisible_{true};
    bool gridHorizontalLinesVisible_{true};
    bool gridVerticalLinesVisible_{true};
    bool subGridHorizontalLinesVisible_{true};
    bool subGridVerticalLinesVisible_{true};
};

} // namespace QAccelPlot
