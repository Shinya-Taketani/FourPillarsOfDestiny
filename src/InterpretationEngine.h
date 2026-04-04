#pragma once

#include "ChartResult.h"
#include "InterpretationResult.h"

class InterpretationEngine
{
public:
    InterpretationResult interpret(const ChartResult &chartResult) const;
};
