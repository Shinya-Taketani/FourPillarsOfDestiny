#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

#include "BirthInfo.h"

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString appTitle READ appTitle CONSTANT)

public:
    explicit AppController(QObject *parent = nullptr);

    QString appTitle() const;

    Q_INVOKABLE QString helloMessage() const;
    Q_INVOKABLE void setBirthInfo(const QString &birthDate, const QString &birthTime, const QString &gender);
    Q_INVOKABLE QVariantMap currentBirthInfo() const;
    Q_INVOKABLE QVariantMap mockChartResult() const;

private:
    BirthInfo m_birthInfo;
};
