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
        map.value(QStringLiteral("fiveElementDistributionStatusMessage")).toString(),
        map.value(QStringLiteral("seasonalEvaluation")).toMap(),
        map.value(QStringLiteral("seasonalEvaluationStatusMessage")).toString(),
        map.value(QStringLiteral("strengthEvaluation")).toMap(),
        map.value(QStringLiteral("strengthEvaluationStatusMessage")).toString(),
        map.value(QStringLiteral("climateEvaluation")).toMap(),
        map.value(QStringLiteral("climateEvaluationStatusMessage")).toString()
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
        {QStringLiteral("fiveElementDistributionStatusMessage"), fiveElementDistributionStatusMessage},
        {QStringLiteral("seasonalEvaluation"), seasonalEvaluation},
        {QStringLiteral("seasonalEvaluationStatusMessage"), seasonalEvaluationStatusMessage},
        {QStringLiteral("strengthEvaluation"), strengthEvaluation},
        {QStringLiteral("strengthEvaluationStatusMessage"), strengthEvaluationStatusMessage},
        {QStringLiteral("climateEvaluation"), climateEvaluation},
        {QStringLiteral("climateEvaluationStatusMessage"), climateEvaluationStatusMessage}
    };
}
