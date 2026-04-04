#include "BirthInfo.h"

QVariantMap BirthInfo::toVariantMap() const
{
    return {
        {QStringLiteral("birthDate"), birthDate},
        {QStringLiteral("birthTime"), birthTime},
        {QStringLiteral("gender"), gender}
    };
}
