#pragma once

#include <QString>
#include <QVariantMap>

struct InterpretationResult
{
    QString summaryText;
    QString detailText;
    QString cautionText;

    static InterpretationResult fromVariantMap(const QVariantMap &map);
    QVariantMap toVariantMap() const;
};
