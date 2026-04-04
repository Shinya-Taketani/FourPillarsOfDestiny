#pragma once

#include <QJsonObject>
#include <QString>

#include "BirthInfo.h"
#include "ChartResult.h"
#include "InterpretationResult.h"

struct SavedChartRecord
{
    BirthInfo birthInfo;
    ChartResult chartResult;
    InterpretationResult interpretationResult;
    QString savedAt;

    static bool fromJsonObject(const QJsonObject &jsonObject, SavedChartRecord *outRecord);
    QJsonObject toJsonObject() const;
};
