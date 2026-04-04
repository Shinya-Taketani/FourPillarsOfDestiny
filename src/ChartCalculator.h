#pragma once

#include "BirthInfo.h"
#include "ChartResult.h"

class ChartCalculator
{
public:
    ChartResult calculate(const BirthInfo &birthInfo) const;
};
