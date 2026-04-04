#include "InterpretationResult.h"

InterpretationResult InterpretationResult::fromVariantMap(const QVariantMap &map)
{
    return {
        map.value(QStringLiteral("summaryText")).toString(),
        map.value(QStringLiteral("detailText")).toString(),
        map.value(QStringLiteral("cautionText")).toString()
    };
}

QVariantMap InterpretationResult::toVariantMap() const
{
    return {
        {QStringLiteral("summaryText"), summaryText},
        {QStringLiteral("detailText"), detailText},
        {QStringLiteral("cautionText"), cautionText}
    };
}
