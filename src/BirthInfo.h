#pragma once

#include <QString>
#include <QVariantMap>

struct BirthInfo
{
    QString birthDate;
    QString birthTime;
    QString gender;

    QVariantMap toVariantMap() const;
};
