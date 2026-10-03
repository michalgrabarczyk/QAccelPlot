//
// SPDX-FileCopyrightText: 2026 Michal Grabarczyk
// SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
//
// This file is also available under a separate commercial license.
// See COMMERCIAL-LICENSING.md for contact information.
//
#include "QAccelPlot/inspection/InspectionResult.hpp"

namespace QAccelPlot {

bool InspectionSample::valid() const
{
    return status == InspectionStatus::Ready;
}

qreal InspectionSample::x() const
{
    return position.x();
}

qreal InspectionSample::y() const
{
    return position.y();
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
