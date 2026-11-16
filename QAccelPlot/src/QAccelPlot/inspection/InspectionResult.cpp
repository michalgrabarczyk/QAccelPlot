//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: MIT
//
#include "QAccelPlot/inspection/InspectionResult.hpp"

namespace QAccelPlot {

bool InspectionDataPoint::valid() const
{
    return status == InspectionStatus::Ready;
}

bool InspectionSummary::valid() const
{
    return status == InspectionStatus::Ready;
}

bool InspectionBracket::valid() const
{
    return status == InspectionStatus::Ready;
}

bool InspectionPage::valid() const
{
    return status == InspectionStatus::Ready;
}

bool InspectionRecord::valid() const
{
    return status == InspectionStatus::Ready;
}

} // namespace QAccelPlot
