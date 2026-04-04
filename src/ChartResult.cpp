#include "ChartResult.h"

QVariantMap ChartResult::toVariantMap() const
{
    return {
        {QStringLiteral("yearPillar"), yearPillar},
        {QStringLiteral("monthPillar"), monthPillar},
        {QStringLiteral("dayPillar"), dayPillar},
        {QStringLiteral("hourPillar"), hourPillar},
        {QStringLiteral("description"), description}
    };
}
