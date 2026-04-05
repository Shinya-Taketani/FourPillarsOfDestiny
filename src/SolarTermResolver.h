#pragma once

#include <QString>

#include "BirthInfo.h"

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
};
