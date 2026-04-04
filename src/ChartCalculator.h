#pragma once

#include <QVariantMap>

#include "BirthInfo.h"

class ChartCalculator
{
public:
    QVariantMap calculate(const BirthInfo &birthInfo) const;
};
