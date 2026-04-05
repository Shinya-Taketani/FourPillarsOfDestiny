#pragma once

#include "BirthInfo.h"
#include "ChartResult.h"

class ChartCalculator
{
public:
    ChartResult calculate(const BirthInfo &birthInfo) const;

private:
    QString calculateYearPillar(const BirthInfo &birthInfo) const;
    QString calculateMonthPillar(const BirthInfo &birthInfo) const;
    QString calculateDayPillar(const BirthInfo &birthInfo) const;
    QString calculateHourPillar(const BirthInfo &birthInfo) const;
    QString buildDescription(
        const BirthInfo &birthInfo,
        const QString &yearPillar,
        const QString &monthPillar,
        const QString &dayPillar,
        const QString &hourPillar
    ) const;
};
