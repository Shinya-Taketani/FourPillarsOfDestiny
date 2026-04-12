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
    QString sourceType;
    QString sourceTitle;
    QString sourceAuthor;
    QString sourceUrl;
    QString confidence;
    QString adoptable;
    QString notes;
};

struct SolarTermYearData
{
    bool dataSourceAvailable;
    bool hasYearData;
    int year;
    QList<SolarTermEntry> entries;
    QString statusMessage;
    QString sourceQuality;
    QString adoptable;
    QString yearStatus;
    QString notes;
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
