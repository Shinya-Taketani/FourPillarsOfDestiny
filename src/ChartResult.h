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

    static ChartResult fromVariantMap(const QVariantMap &map);
    QVariantMap toVariantMap() const;
};
