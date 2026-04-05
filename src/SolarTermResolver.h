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
};

class SolarTermResolver
{
public:
    SolarTermResolution resolveMonthPillar(const BirthInfo &birthInfo) const;

private:
    SolarTermDataSource m_dataSource;
};
