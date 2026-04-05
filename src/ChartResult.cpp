#include "ChartResult.h"

ChartResult ChartResult::fromVariantMap(const QVariantMap &map)
{
    return {
        map.value(QStringLiteral("yearPillar")).toString(),
        map.value(QStringLiteral("monthPillar")).toString(),
        map.value(QStringLiteral("dayPillar")).toString(),
        map.value(QStringLiteral("hourPillar")).toString(),
        map.value(QStringLiteral("description")).toString(),
        map.value(QStringLiteral("monthPillarStatusMessage")).toString(),
        map.value(QStringLiteral("tenGods")).toMap(),
        map.value(QStringLiteral("hiddenStems")).toMap(),
        map.value(QStringLiteral("fiveElements")).toMap(),
        map.value(QStringLiteral("fiveElementDistributionStatusMessage")).toString()
    };
}

QVariantMap ChartResult::toVariantMap() const
{
    return {
        {QStringLiteral("yearPillar"), yearPillar},
        {QStringLiteral("monthPillar"), monthPillar},
        {QStringLiteral("dayPillar"), dayPillar},
        {QStringLiteral("hourPillar"), hourPillar},
        {QStringLiteral("description"), description},
        {QStringLiteral("monthPillarStatusMessage"), monthPillarStatusMessage},
        {QStringLiteral("tenGods"), tenGods},
        {QStringLiteral("hiddenStems"), hiddenStems},
        {QStringLiteral("fiveElements"), fiveElements},
        {QStringLiteral("fiveElementDistributionStatusMessage"), fiveElementDistributionStatusMessage}
    };
}
