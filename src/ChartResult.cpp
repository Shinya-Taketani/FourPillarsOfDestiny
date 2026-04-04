#include "ChartResult.h"

ChartResult ChartResult::fromVariantMap(const QVariantMap &map)
{
    return {
        map.value(QStringLiteral("yearPillar")).toString(),
        map.value(QStringLiteral("monthPillar")).toString(),
        map.value(QStringLiteral("dayPillar")).toString(),
        map.value(QStringLiteral("hourPillar")).toString(),
        map.value(QStringLiteral("description")).toString()
    };
}

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
