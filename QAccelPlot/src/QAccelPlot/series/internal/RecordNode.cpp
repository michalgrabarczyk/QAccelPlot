//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/series/internal/RecordNode.hpp"

#include "QAccelPlot/QAccelPlotLogging.hpp"
#include "QAccelPlot/materials/DataTextureMaterial.hpp"
#include "QAccelPlot/series/DataPointStore.hpp"
#include "QAccelPlot/series/internal/SeriesSupport.hpp"

#include <QSGMaterial>

#include <cstddef>
#include <cstring>

namespace QAccelPlot::Internal {

QSGGeometryNode* createRecordNode(const QSGGeometry::AttributeSet& attributes, QSGMaterial* material)
{
    auto* geometry = new QSGGeometry(attributes, 0);
    geometry->setDrawingMode(QSGGeometry::DrawTriangles);
    geometry->setVertexDataPattern(QSGGeometry::StaticPattern);
    auto* node = new QSGGeometryNode;
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    node->setMaterial(material);
    node->setFlag(QSGNode::OwnsMaterial);
    return node;
}

int reservedRecordCount(const int drawnCount)
{
    constexpr auto kMinReserved = 256;
    auto reserved = kMinReserved;
    while (reserved < drawnCount) {
        reserved *= 2;
    }
    return reserved;
}

bool recordFrameDrawn(const RecordFrame& frame, bool& softwareWarned)
{
    if (!frame.drawable) {
        return false;
    }
    if (supportsCustomShaderRendering(frame.window)) {
        return true;
    }
    if (!softwareWarned) {
        qCWarning(lcQAccelPlot) << frame.typeName
                                << "custom rendering requires a hardware scene graph backend. Skipping updatePaintNode on the software backend.";
        softwareWarned = true;
    }
    return false;
}

void allocateRecordVertices(QSGGeometryNode& node, const RecordFrame& frame)
{
    auto* geometry = node.geometry();
    const auto vertexCount = reservedRecordCount(frame.drawnCount) * frame.verticesPerRecord;
    if (geometry->vertexCount() == vertexCount) {
        return;
    }
    geometry->allocate(vertexCount);
    // The shader derives everything from the vertex index; zeroing only avoids uploading uninitialized memory.
    std::memset(geometry->vertexData(), 0, static_cast<std::size_t>(vertexCount) * static_cast<std::size_t>(geometry->sizeOfVertex()));
    node.markDirty(QSGNode::DirtyGeometry);
}

void uploadRecordData(
    DataTextureMaterial& material, const RecordFrame& frame, const DataPointStore& store, std::uint64_t& uploadedRevision, const bool newMaterial)
{
    if (store.renderRevision() == uploadedRevision && !newMaterial && material.dataTexture) {
        return;
    }
    material.uploadTexture(frame.window, store.renderData().data(), frame.drawnCount * frame.floatsPerRecord);
    uploadedRevision = store.renderRevision();
}

} // namespace QAccelPlot::Internal
