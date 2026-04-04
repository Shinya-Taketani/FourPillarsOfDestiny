#include "SavedChartRecord.h"

#include <QJsonObject>
#include <QVariantMap>

bool SavedChartRecord::fromJsonObject(const QJsonObject &jsonObject, SavedChartRecord *outRecord)
{
    if (!jsonObject.contains(QStringLiteral("birthInfo"))
        || !jsonObject.contains(QStringLiteral("chartResult"))
        || !jsonObject.contains(QStringLiteral("interpretationResult"))
        || !jsonObject.contains(QStringLiteral("savedAt"))) {
        return false;
    }

    if (!outRecord) {
        return false;
    }

    outRecord->birthInfo = BirthInfo::fromVariantMap(jsonObject.value(QStringLiteral("birthInfo")).toObject().toVariantMap());
    outRecord->chartResult = ChartResult::fromVariantMap(jsonObject.value(QStringLiteral("chartResult")).toObject().toVariantMap());
    outRecord->interpretationResult = InterpretationResult::fromVariantMap(
        jsonObject.value(QStringLiteral("interpretationResult")).toObject().toVariantMap()
    );
    outRecord->savedAt = jsonObject.value(QStringLiteral("savedAt")).toString();

    return true;
}

QJsonObject SavedChartRecord::toJsonObject() const
{
    return {
        {QStringLiteral("schemaVersion"), 1},
        {QStringLiteral("savedAt"), savedAt},
        {QStringLiteral("birthInfo"), QJsonObject::fromVariantMap(birthInfo.toVariantMap())},
        {QStringLiteral("chartResult"), QJsonObject::fromVariantMap(chartResult.toVariantMap())},
        {QStringLiteral("interpretationResult"), QJsonObject::fromVariantMap(interpretationResult.toVariantMap())}
    };
}
