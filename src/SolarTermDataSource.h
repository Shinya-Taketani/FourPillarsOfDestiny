#pragma once

#include <QJsonArray>
#include <QString>

struct SolarTermYearData
{
    bool dataSourceAvailable;
    bool hasYearData;
    int year;
    QJsonArray entries;
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
