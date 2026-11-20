//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#pragma once

#include <QSGGeometry>
#include <QSGGeometryNode>

#include <cstdint>

QT_FORWARD_DECLARE_CLASS(QQuickWindow)
QT_FORWARD_DECLARE_CLASS(QSGMaterial)

namespace QAccelPlot {
class DataPointStore;
class DataTextureMaterial;
} // namespace QAccelPlot

namespace QAccelPlot::Internal {

/// \brief What a series that draws one record per data point draws in a frame.
struct RecordFrame {
    QQuickWindow* window{nullptr}; ///< \brief Window the series renders in.
    int drawnCount{0};             ///< \brief Records drawn.
    /// \brief \c false removes the node: no records are drawn, or an axis, the plot rectangle, or the window is missing.
    bool drawable{false};
    int floatsPerRecord{0};   ///< \brief Floats of one record in the data texture.
    int verticesPerRecord{0}; ///< \brief Vertices the shader draws one record with.
    const char* typeName{""}; ///< \brief Series class the software-backend warning names.
};

/// \brief Returns an empty triangle node of \a attributes that owns its geometry and \a material.
///
/// For series whose shader derives every vertex from its index and reads the data points from the
/// data texture. The vertex buffer is in static GPU memory, as it only changes with \c reservedRecordCount().
QSGGeometryNode* createRecordNode(const QSGGeometry::AttributeSet& attributes, QSGMaterial* material);

/// \brief Returns how many data points the vertex buffer of a record node holds room for, given \a drawnCount drawn ones.
///
/// It is the next power of two, at least 256, so appending one data point at a time reallocates the
/// buffer only when the count crosses one.
int reservedRecordCount(int drawnCount);

/// \brief Returns whether \a frame is drawn; warns once through \a softwareWarned when the backend cannot run custom shaders.
bool recordFrameDrawn(const RecordFrame& frame, bool& softwareWarned);

/// \brief Sizes the vertex buffer of \a node for \c reservedRecordCount() records of \a frame.
void allocateRecordVertices(QSGGeometryNode& node, const RecordFrame& frame);

/// \brief Uploads the render data of \a store to \a material unless \a uploadedRevision is its render revision.
///
/// Set \a newMaterial for a material that has never been uploaded to.
void uploadRecordData(DataTextureMaterial& material, const RecordFrame& frame, const DataPointStore& store, std::uint64_t& uploadedRevision, bool newMaterial);

/// \brief Returns the node that draws \a frame with a \a Material, reusing \a oldNode, or \c nullptr when \a frame is not drawn.
///
/// Deletes \a oldNode when returning \c nullptr. Otherwise sizes the vertex buffer, uploads the render data of
/// \a store when it changed since \a uploadedRevision, and passes the material to \a updateMaterial to set its uniforms.
template <typename Material, typename UpdateMaterial>
QSGGeometryNode* updateRecordNode(
    QSGNode* oldNode, const RecordFrame& frame, const DataPointStore& store, std::uint64_t& uploadedRevision, UpdateMaterial&& updateMaterial)
{
    static auto softwareWarned = false;
    if (!recordFrameDrawn(frame, softwareWarned)) {
        delete oldNode;
        return nullptr;
    }
    auto* node = oldNode ? static_cast<QSGGeometryNode*>(oldNode) : createRecordNode(Material::attributeSet(), new Material);
    allocateRecordVertices(*node, frame);
    auto& material = static_cast<Material&>(*node->material());
    uploadRecordData(material, frame, store, uploadedRevision, !oldNode);
    updateMaterial(material);
    node->markDirty(QSGNode::DirtyMaterial);
    return node;
}

} // namespace QAccelPlot::Internal
