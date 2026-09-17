//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/series/SpatialGrid.hpp"

#include <QtTest/QtTest>

#include <cstdlib>
#include <new>

namespace {
thread_local bool measureAllocations = false;
thread_local std::size_t allocatedBytes = 0;

class AllocationMeasurement {
public:
    AllocationMeasurement()
    {
        allocatedBytes = 0;
        measureAllocations = true;
    }

    ~AllocationMeasurement()
    {
        measureAllocations = false;
    }
};
}

// Confine allocation instrumentation to this test executable.
void* operator new(const std::size_t size)
{
    if (measureAllocations) {
        allocatedBytes += size;
    }
    if (auto* memory = std::malloc(size == 0 ? 1 : size)) {
        return memory;
    }
    throw std::bad_alloc{};
}

void operator delete(void* memory) noexcept
{
    std::free(memory);
}

void operator delete(void* memory, std::size_t) noexcept
{
    std::free(memory);
}

class SpatialGridMemoryTest : public QObject {
    Q_OBJECT
private slots:
    void overlappingRectanglesHaveBoundedMemory();
};

void SpatialGridMemoryTest::overlappingRectanglesHaveBoundedMemory()
{
    constexpr auto count = 1024;
    auto data = std::vector<float>(count * 4);
    for (auto i = 0; i < count; ++i) {
        data[i * 4 + 2] = 1;
        data[i * 4 + 3] = 1;
    }
    data[(count - 1) * 4 + 2] = 0.01f;
    data[(count - 1) * 4 + 3] = 0.01f;
    auto grid = QAccelPlot::SpatialGrid{};
    {
        const auto measurement = AllocationMeasurement{};
        grid.build(data.data(), count);
    }
    const auto bytes = allocatedBytes;

    QCOMPARE(grid.query(0.005f, 0.005f), count - 1);
    QCOMPARE(grid.query(0.75f, 0.75f), count - 2);
    QVERIFY2(bytes < 512 * 1024, qPrintable(QStringLiteral("Index build allocated %1 bytes for 1024 rectangles").arg(bytes)));
    grid.build(nullptr, 0);
    QCOMPARE(grid.query(0.25f, 0.25f), -1);
}

QTEST_APPLESS_MAIN(SpatialGridMemoryTest)
#include "tst_spatial_grid_memory.moc"
