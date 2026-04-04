#include "BirthInfo.h"

#include <QRegularExpression>

bool BirthInfo::isValid() const
{
    return validationErrors().isEmpty();
}

QStringList BirthInfo::validationErrors() const
{
    QStringList errors;

    static const QRegularExpression datePattern(QStringLiteral("^\\d{4}-\\d{2}-\\d{2}$"));
    static const QRegularExpression timePattern(QStringLiteral("^\\d{2}:\\d{2}$"));

    if (birthDate.trimmed().isEmpty()) {
        errors.append(QStringLiteral("生年月日を入力してください。"));
    } else if (!datePattern.match(birthDate.trimmed()).hasMatch()) {
        errors.append(QStringLiteral("生年月日は yyyy-MM-dd 形式で入力してください。"));
    }

    if (birthTime.trimmed().isEmpty()) {
        errors.append(QStringLiteral("出生時刻を入力してください。"));
    } else if (!timePattern.match(birthTime.trimmed()).hasMatch()) {
        errors.append(QStringLiteral("出生時刻は HH:mm 形式で入力してください。"));
    }

    if (gender.trimmed().isEmpty()) {
        errors.append(QStringLiteral("性別を選択してください。"));
    }

    return errors;
}

QVariantMap BirthInfo::toVariantMap() const
{
    return {
        {QStringLiteral("birthDate"), birthDate},
        {QStringLiteral("birthTime"), birthTime},
        {QStringLiteral("gender"), gender}
    };
}
