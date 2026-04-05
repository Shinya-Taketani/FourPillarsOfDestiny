#pragma once

#include <QString>
#include <QVariantMap>

struct ChartResult
{
    QString yearPillar;
    QString monthPillar;
    QString dayPillar;
    QString hourPillar;
    QString description;
    QString monthPillarStatusMessage;
    QVariantMap tenGods;
    QVariantMap hiddenStems;
    QVariantMap fiveElements;
    QString fiveElementDistributionStatusMessage;
    QVariantMap seasonalEvaluation;
    QString seasonalEvaluationStatusMessage;
    QVariantMap strengthEvaluation;
    QString strengthEvaluationStatusMessage;
    QVariantMap climateEvaluation;
    QString climateEvaluationStatusMessage;
    QVariantMap usefulGodCandidates;
    QString usefulGodCandidatesStatusMessage;

    static ChartResult fromVariantMap(const QVariantMap &map);
    QVariantMap toVariantMap() const;
};
