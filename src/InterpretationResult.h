#pragma once

#include <QString>
#include <QVariantMap>

struct InterpretationResult
{
    QString summaryText;
    QString detailText;
    QString cautionText;

    QVariantMap toVariantMap() const;
};
