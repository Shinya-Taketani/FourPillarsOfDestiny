#include "SavedChartRecord.h"

#include <QJsonObject>

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
