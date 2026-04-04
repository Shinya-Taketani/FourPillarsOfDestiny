#pragma once

#include <QString>
#include <QStringList>
#include <QVariantMap>

struct BirthInfo
{
    QString birthDate;
    QString birthTime;
    QString gender;

    bool isValid() const;
    QStringList validationErrors() const;
    static BirthInfo fromVariantMap(const QVariantMap &map);
    QVariantMap toVariantMap() const;
};
