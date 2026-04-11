#pragma once

#include <QDateTime>
#include <QList>
#include <QString>

struct SolarTermEntry
{
    int year;
    QString termName;
    QString atIsoString;
    QDateTime atDateTime;
    QString timeZoneOffsetText;
};

struct SolarTermYearData
{
    bool dataSourceAvailable;
    bool hasYearData;
    int year;
    QList<SolarTermEntry> entries;
    QString statusMessage;
};

class SolarTermDataSource
{
public:
    explicit SolarTermDataSource(QString dataFilePath = QString());

    SolarTermYearData loadYearData(int year) const;
    QString dataFilePath() const;
    void setDataFilePath(const QString &dataFilePath);

private:
    QString m_dataFilePath;
};
