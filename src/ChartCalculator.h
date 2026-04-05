#pragma once

#include "BirthInfo.h"
#include "ChartResult.h"
#include "SolarTermResolver.h"

class ChartCalculator
{
public:
    ChartResult calculate(const BirthInfo &birthInfo) const;

private:
    QString calculateYearPillar(const BirthInfo &birthInfo) const;
    SolarTermResolution calculateMonthPillarResolution(
        const BirthInfo &birthInfo,
        const QString &yearPillar
    ) const;
    QString calculateDayPillar(const BirthInfo &birthInfo) const;
    QString calculateHourPillar(const BirthInfo &birthInfo, const QString &dayPillar) const;
    QVariantMap calculateTenGods(
        const QString &yearPillar,
        const QString &monthPillar,
        const QString &dayPillar,
        const QString &hourPillar
    ) const;
    QVariantMap calculateHiddenStems(
        const QString &yearPillar,
        const QString &monthPillar,
        const QString &dayPillar,
        const QString &hourPillar
    ) const;
    QVariantMap calculateFiveElements(
        const QString &yearPillar,
        const QString &monthPillar,
        const QString &dayPillar,
        const QString &hourPillar,
        const QVariantMap &hiddenStems,
        QString *statusMessage
    ) const;
    QVariantMap calculateSeasonalEvaluation(
        const QString &monthPillar,
        const QString &dayPillar,
        QString *statusMessage
    ) const;
    QVariantMap calculateStrengthEvaluation(
        const QString &dayPillar,
        const QVariantMap &fiveElements,
        const QVariantMap &seasonalEvaluation,
        QString *statusMessage
    ) const;
    QString buildDescription(
        const BirthInfo &birthInfo,
        const QString &yearPillar,
        const SolarTermResolution &monthResolution,
        const QString &dayPillar,
        const QString &hourPillar
    ) const;

    SolarTermResolver m_solarTermResolver;
};
