#pragma once

#include <QString>
#include <QVariantList>
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
    QVariantMap patternCandidates;
    QString patternCandidatesStatusMessage;
    QVariantList majorFortunes;
    QString majorFortunesStatusMessage;
    QVariantList annualFortunes;
    QString annualFortunesStatusMessage;

    static ChartResult fromVariantMap(const QVariantMap &map);
    QVariantMap toVariantMap() const;
};
