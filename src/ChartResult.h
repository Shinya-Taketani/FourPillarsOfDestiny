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

    static ChartResult fromVariantMap(const QVariantMap &map);
    QVariantMap toVariantMap() const;
};
