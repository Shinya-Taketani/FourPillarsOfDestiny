#pragma once

#include <QString>

#include "BirthInfo.h"
#include "SolarTermDataSource.h"

struct SolarTermResolution
{
    bool isImplemented;
    bool canDetermineMonthPillar;
    QString monthPillar;
    QString statusMessage;
    QString dataQuality;
    QString adoptable;
    QString dataNotes;
};

struct SolarTermDifferenceResolution
{
    bool isImplemented;
    bool canDetermineDifference;
    QString birthDateTimeIso;
    QString referenceTermName;
    QString referenceDirection;
    QString referenceDateTimeIso;
    qint64 differenceMinutes;
    qint64 absoluteDifferenceMinutes;
    QString statusMessage;
};

class SolarTermResolver
{
public:
    SolarTermResolution resolveMonthPillar(const BirthInfo &birthInfo, const QString &yearPillar) const;
    SolarTermDifferenceResolution resolveNearestSolarTermDifference(const BirthInfo &birthInfo) const;
    SolarTermDifferenceResolution resolvePreviousSolarTermDifference(const BirthInfo &birthInfo) const;
    SolarTermDifferenceResolution resolveNextSolarTermDifference(const BirthInfo &birthInfo) const;

private:
    SolarTermDataSource m_dataSource;
};
