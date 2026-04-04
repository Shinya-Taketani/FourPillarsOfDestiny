#include "InterpretationResult.h"

QVariantMap InterpretationResult::toVariantMap() const
{
    return {
        {QStringLiteral("summaryText"), summaryText},
        {QStringLiteral("detailText"), detailText},
        {QStringLiteral("cautionText"), cautionText}
    };
}
