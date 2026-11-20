

# File RecordNode.hpp

[**File List**](files.md) **>** [**internal**](dir_70e6e0d61970c92b37b608a046280901.md) **>** [**RecordNode.hpp**](RecordNode_8hpp.md)

[Go to the documentation of this file](RecordNode_8hpp.md)


```C++
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

struct RecordFrame {
    QQuickWindow* window{nullptr}; 
    int drawnCount{0};             
    bool drawable{false};
    int floatsPerRecord{0};   
    int verticesPerRecord{0}; 
    const char* typeName{""}; 
};

QSGGeometryNode* createRecordNode(const QSGGeometry::AttributeSet& attributes, QSGMaterial* material);

int reservedRecordCount(int drawnCount);

bool recordFrameDrawn(const RecordFrame& frame, bool& softwareWarned);

void allocateRecordVertices(QSGGeometryNode& node, const RecordFrame& frame);

void uploadRecordData(DataTextureMaterial& material, const RecordFrame& frame, const DataPointStore& store, std::uint64_t& uploadedRevision, bool newMaterial);

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
```


