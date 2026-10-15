//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#pragma once

#include <QtGlobal>

#include <cstdint>
#include <optional>
#include <vector>

namespace QAccelPlot {

/// \brief Holds the data points of one series and derives everything that depends only on them.
///
/// A data point is a fixed number of coordinate columns, each tied to the X or Y dimension, plus
/// optional float attributes and an optional integer category. The store keeps the coordinates at
/// the precision they were given in and derives:
///
/// - the extents of the valid coordinates under one of three range rules;
/// - the float render data uploaded to the GPU. Float input is its own render data. Double input is
///   uploaded relative to a render origin per dimension, which moves to the viewport center when the
///   viewport gets too far from it. Logarithmic dimensions are never shifted.
///
/// Render data is rebuilt lazily, the first time it is read after something it depends on changed,
/// so even its const readers may rebuild it. The store is not thread-safe: use it from the series'
/// thread, or from \c updatePaintNode() while that thread is blocked.
class DataPointStore {
public:
    /// \brief Plot dimension a coordinate column belongs to.
    enum class Dimension { X, Y };

    /// \brief Which coordinates count toward the extents.
    ///
    /// A coordinate is valid when it is finite and, on a logarithmic dimension, positive.
    enum class RangeRule {
        EachCoordinate, ///< \brief Every valid coordinate counts on its own.
        WholeDataPoint, ///< \brief A data point counts only when all its coordinates are valid.
        SkipOnNaN,      ///< \brief A data point with a NaN coordinate is skipped; otherwise each valid coordinate counts.
    };

    /// \brief Shape of a data point.
    struct Layout {
        std::vector<Dimension> columns{Dimension::X, Dimension::Y}; ///< \brief Coordinate columns in data point order.
        RangeRule rangeRule{RangeRule::EachCoordinate};             ///< \brief Rule for the extents.
        int attributeCount{0}; ///< \brief Float attributes per data point; interleaved after the coordinates in render data.
    };

    /// \brief Visible range of one dimension.
    struct Viewport {
        double min; ///< \brief Lower viewport bound.
        double max; ///< \brief Upper viewport bound.
    };

    /// \brief Scale and viewport of one dimension, as its axis currently shows it.
    struct DimensionState {
        bool logScale{false};             ///< \brief Whether the dimension is logarithmic.
        std::optional<Viewport> viewport; ///< \brief The axis viewport, or \c std::nullopt without an axis.
    };

    /// \brief Smallest and largest valid coordinate in one dimension.
    struct Extent {
        double min; ///< \brief Smallest valid coordinate.
        double max; ///< \brief Largest valid coordinate.
    };

    /// \brief Extents in both dimensions; a dimension without a valid coordinate has none.
    struct Extents {
        std::optional<Extent> x; ///< \brief Extent of the X columns.
        std::optional<Extent> y; ///< \brief Extent of the Y columns.
    };

    /// \brief Read-only view over interleaved data points in float or double precision.
    ///
    /// Only valid until the next change of the store.
    struct View {
        const double* precise{nullptr}; ///< \brief Double data points, or \c nullptr when \c floats is set.
        const float* floats{nullptr};   ///< \brief Float data points, or \c nullptr when \c precise is set.
        int stride{0};                  ///< \brief Values per data point in the set buffer.
        int count{0};                   ///< \brief Number of data points.

        /// \brief Returns value \a column of data point \a index without narrowing double data.
        [[nodiscard]] double value(int index, int column) const noexcept;
    };

    /// \brief Read-only view over the float attributes of the stored data points.
    struct AttributeView {
        const float* data{nullptr}; ///< \brief First attribute of the first data point, or \c nullptr without attributes.
        int stride{0};              ///< \brief Floats between the attributes of consecutive data points.
    };

    /// \brief Creates an empty store whose data points have \a layout.
    explicit DataPointStore(Layout layout);

    /// \brief Returns the data point layout.
    [[nodiscard]] const Layout& layout() const noexcept;
    /// \brief Clears the store and switches to \a layout.
    void setLayout(Layout layout);
    /// \brief Returns the number of coordinate columns per data point.
    [[nodiscard]] int columnCount() const noexcept;

    /// \brief Replaces the data points with \a count double data points moved from \a coordinates.
    ///
    /// \a attributes is empty, or holds \c attributeCount floats per data point. The sizes must match.
    void setPrecise(std::vector<double>&& coordinates, int count, std::vector<float>&& attributes = {});
    /// \brief Replaces the data points with \a count double data points copied from \a coordinates.
    void assignPrecise(const double* coordinates, int count);
    /// \brief Replaces the data points with \a count float data points moved from \a coordinates.
    ///
    /// Without attributes the moved buffer becomes the render data as is. With attributes, it is
    /// interleaved with them once.
    void setFloat(std::vector<float>&& coordinates, int count, std::vector<float>&& attributes = {});
    /// \brief Replaces the data points with \a count float data points copied from \a coordinates, reusing the allocation.
    void assignFloat(const float* coordinates, int count);
    /// \brief Replaces the float attributes: empty, or \c attributeCount floats per data point.
    void setAttributes(std::vector<float>&& attributes);
    /// \brief Replaces the categories: empty, or one per data point.
    void setCategories(std::vector<int>&& categories);
    /// \brief Appends one double data point of \c columnCount() coordinates; promotes float data first.
    ///
    /// The render data grows in place when nothing else it depends on has changed.
    void append(const double* coordinates);
    /// \brief Converts float data points to double, so later updates keep their precision. Does nothing for double data.
    void promoteToPrecise();
    /// \brief Removes all data points, attributes, and categories.
    void clear();

    /// \brief Returns the number of data points.
    [[nodiscard]] int count() const noexcept;
    /// \brief Returns whether the data points are stored as doubles.
    [[nodiscard]] bool isPrecise() const noexcept;
    /// \brief Returns whether the data points carry float attributes.
    [[nodiscard]] bool hasAttributes() const noexcept;
    /// \brief Returns whether the data points carry categories.
    [[nodiscard]] bool hasCategories() const noexcept;
    /// \brief Returns the stored data points.
    [[nodiscard]] View view() const noexcept;
    /// \brief Returns coordinate \a column of stored data point \a index.
    [[nodiscard]] double value(int index, int column) const noexcept;
    /// \brief Returns the stored float attributes.
    [[nodiscard]] AttributeView attributes() const noexcept;
    /// \brief Returns attribute \a which of data point \a index; NaN without attributes.
    [[nodiscard]] float attribute(int index, int which = 0) const noexcept;
    /// \brief Returns the stored categories: empty, or one per data point.
    [[nodiscard]] const std::vector<int>& categories() const noexcept;

    /// \brief Returns the extents of data points [\a first, \a last) under the layout's range rule.
    ///
    /// \a last of -1 means the end. \a logScaleX and \a logScaleY decide which coordinates are valid.
    [[nodiscard]] Extents extents(bool logScaleX, bool logScaleY, int first = 0, int last = -1) const;

    /// \brief Draws \a count data points moved from \a dataPoints instead of the stored ones.
    ///
    /// Used by a running transition for the frame it shows. \a dataPoints holds \c columnCount()
    /// coordinates per data point, followed by \c attributeCount attributes when the stored data
    /// points have attributes. The stored data points, their extents, and \c view() are unchanged.
    /// Returns the previous drawn buffer, so a caller drawing frame after frame reuses its allocation.
    /// Replacing the stored data points with ones of another stride draws them again.
    std::vector<double> setDrawn(std::vector<double>&& dataPoints, int count);
    /// \brief Draws the stored data points again.
    void clearDrawn();
    /// \brief Returns whether drawn data points replace the stored ones.
    [[nodiscard]] bool hasDrawn() const noexcept;
    /// \brief Returns the data points that are drawn: the drawn ones, or the stored ones.
    ///
    /// With \c skipInvalid() set, invalid data points are left out.
    [[nodiscard]] View drawnView() const;

    /// \brief Leaves invalid data points out of the render data and \c drawnView().
    ///
    /// A data point is invalid when any coordinate is invalid on the current scale.
    void setSkipInvalid(bool skip);
    /// \brief Returns whether invalid data points are left out of the render data.
    [[nodiscard]] bool skipInvalid() const noexcept;

    /// \brief Updates the scale and viewport of \a dimension.
    ///
    /// Marks the render data stale when the scale changes, or when the viewport moves too far from
    /// the render origin to keep float precision.
    void setDimensionState(Dimension dimension, const DimensionState& state);
    /// \brief Returns the scale and viewport last given for \a dimension.
    [[nodiscard]] const DimensionState& dimensionState(Dimension dimension) const noexcept;

    /// \brief Returns the float render data, rebuilding it first when stale.
    ///
    /// Each data point is its coordinates relative to \c renderOrigin(), then its attributes.
    [[nodiscard]] const std::vector<float>& renderData() const;
    /// \brief Returns the floats per data point in the render data.
    [[nodiscard]] int renderStride() const noexcept;
    /// \brief Returns the number of data points in the render data.
    [[nodiscard]] int renderCount() const;
    /// \brief Returns the render origin of \a dimension, rebuilding the render data first when stale.
    ///
    /// It is 0 for float data and logarithmic dimensions.
    [[nodiscard]] double renderOrigin(Dimension dimension) const;
    /// \brief Returns a number that changes whenever the render data changes.
    [[nodiscard]] std::uint64_t renderRevision() const noexcept;

private:
    struct Origin {
        double value{0.0};
        bool settled{false};
    };

    // Valid data points of the drawn or stored ones, kept only while some are invalid.
    struct Compacted {
        std::vector<double> precise;
        std::vector<float> floats;
        std::vector<float> attributes;
        int count{0};
        bool active{false};
        bool valid{false};
    };

    [[nodiscard]] View sourceView() const noexcept;
    [[nodiscard]] bool usesOrigin() const noexcept;
    [[nodiscard]] bool dataPointValid(const View& view, int index) const noexcept;
    [[nodiscard]] Origin& origin(Dimension dimension) const noexcept;
    [[nodiscard]] double originValue(Dimension dimension) const noexcept;
    void storeFloatCoordinates(std::vector<float>&& coordinates, std::vector<float>&& attributes);
    void dataPointsChanged();
    void invalidateRender();
    void ensureCompacted() const;
    void ensureRenderData() const;
    void chooseOrigins(const View& source) const;
    void writeRenderData(const View& source, const float* attributes, int attributeStride) const;

    Layout layout_;
    int count_{0};
    bool isPrecise_{false};
    bool hasAttributes_{false};
    std::vector<double> precise_;   // columnCount() doubles per data point
    std::vector<float> floats_;     // renderStride() floats per data point; the render data of float input
    std::vector<float> attributes_; // attributeCount floats per data point of double input
    std::vector<int> categories_;

    std::vector<double> drawn_; // renderStride() doubles per data point
    int drawnCount_{0};
    bool hasDrawn_{false};

    bool skipInvalid_{false};
    // Lazily derived from the members above, also by const readers.
    mutable Compacted compacted_;

    DimensionState x_;
    DimensionState y_;
    mutable Origin originX_;
    mutable Origin originY_;
    mutable bool renderLogScaleX_{false};
    mutable bool renderLogScaleY_{false};
    mutable std::vector<float> renderData_;
    mutable bool renderValid_{false};
    std::uint64_t renderRevision_{0};
};

} // namespace QAccelPlot
