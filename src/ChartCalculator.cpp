#include "ChartCalculator.h"

#include <QDate>
#include <QTime>

namespace {
enum class FiveElement
{
    Wood = 0,
    Fire = 1,
    Earth = 2,
    Metal = 3,
    Water = 4
};

enum class Season
{
    Spring = 0,
    Summer = 1,
    Autumn = 2,
    Winter = 3
};

QString heavenlyStemAt(int index)
{
    static const QStringList stems{
        QStringLiteral("甲"),
        QStringLiteral("乙"),
        QStringLiteral("丙"),
        QStringLiteral("丁"),
        QStringLiteral("戊"),
        QStringLiteral("己"),
        QStringLiteral("庚"),
        QStringLiteral("辛"),
        QStringLiteral("壬"),
        QStringLiteral("癸")
    };

    return stems.at(index);
}

QString earthlyBranchAt(int index)
{
    static const QStringList branches{
        QStringLiteral("子"),
        QStringLiteral("丑"),
        QStringLiteral("寅"),
        QStringLiteral("卯"),
        QStringLiteral("辰"),
        QStringLiteral("巳"),
        QStringLiteral("午"),
        QStringLiteral("未"),
        QStringLiteral("申"),
        QStringLiteral("酉"),
        QStringLiteral("戌"),
        QStringLiteral("亥")
    };

    return branches.at(index);
}

int positiveModulo(int value, int divisor)
{
    const int remainder = value % divisor;
    return remainder < 0 ? remainder + divisor : remainder;
}

int heavenlyStemIndex(const QString &pillar)
{
    if (pillar.isEmpty()) {
        return -1;
    }

    static const QStringList stems{
        QStringLiteral("甲"),
        QStringLiteral("乙"),
        QStringLiteral("丙"),
        QStringLiteral("丁"),
        QStringLiteral("戊"),
        QStringLiteral("己"),
        QStringLiteral("庚"),
        QStringLiteral("辛"),
        QStringLiteral("壬"),
        QStringLiteral("癸")
    };

    return stems.indexOf(pillar.left(1));
}

int earthlyBranchIndex(const QString &pillar)
{
    if (pillar.size() < 2) {
        return -1;
    }

    static const QStringList branches{
        QStringLiteral("子"),
        QStringLiteral("丑"),
        QStringLiteral("寅"),
        QStringLiteral("卯"),
        QStringLiteral("辰"),
        QStringLiteral("巳"),
        QStringLiteral("午"),
        QStringLiteral("未"),
        QStringLiteral("申"),
        QStringLiteral("酉"),
        QStringLiteral("戌"),
        QStringLiteral("亥")
    };

    return branches.indexOf(pillar.mid(1, 1));
}

QStringList hiddenStemsForBranchIndex(int branchIndex)
{
    switch (branchIndex) {
    case 0:
        return {QStringLiteral("癸")};
    case 1:
        return {QStringLiteral("己"), QStringLiteral("癸"), QStringLiteral("辛")};
    case 2:
        return {QStringLiteral("甲"), QStringLiteral("丙"), QStringLiteral("戊")};
    case 3:
        return {QStringLiteral("乙")};
    case 4:
        return {QStringLiteral("戊"), QStringLiteral("乙"), QStringLiteral("癸")};
    case 5:
        return {QStringLiteral("丙"), QStringLiteral("戊"), QStringLiteral("庚")};
    case 6:
        return {QStringLiteral("丁"), QStringLiteral("己")};
    case 7:
        return {QStringLiteral("己"), QStringLiteral("丁"), QStringLiteral("乙")};
    case 8:
        return {QStringLiteral("庚"), QStringLiteral("壬"), QStringLiteral("戊")};
    case 9:
        return {QStringLiteral("辛")};
    case 10:
        return {QStringLiteral("戊"), QStringLiteral("辛"), QStringLiteral("丁")};
    case 11:
        return {QStringLiteral("壬"), QStringLiteral("甲")};
    default:
        return {QStringLiteral("未対応")};
    }
}

QStringList hiddenStemsForPillar(const QString &pillar)
{
    return hiddenStemsForBranchIndex(earthlyBranchIndex(pillar));
}

bool isYangStem(int stemIndex)
{
    return stemIndex >= 0 && stemIndex % 2 == 0;
}

FiveElement fiveElementForStem(int stemIndex)
{
    switch (stemIndex) {
    case 0:
    case 1:
        return FiveElement::Wood;
    case 2:
    case 3:
        return FiveElement::Fire;
    case 4:
    case 5:
        return FiveElement::Earth;
    case 6:
    case 7:
        return FiveElement::Metal;
    case 8:
    case 9:
        return FiveElement::Water;
    default:
        return FiveElement::Wood;
    }
}

FiveElement fiveElementForBranch(int branchIndex)
{
    switch (branchIndex) {
    case 0:
    case 11:
        return FiveElement::Water;
    case 2:
    case 3:
        return FiveElement::Wood;
    case 5:
    case 6:
        return FiveElement::Fire;
    case 8:
    case 9:
        return FiveElement::Metal;
    case 1:
    case 4:
    case 7:
    case 10:
        return FiveElement::Earth;
    default:
        return FiveElement::Earth;
    }
}

QString fiveElementKey(FiveElement element)
{
    switch (element) {
    case FiveElement::Wood:
        return QStringLiteral("wood");
    case FiveElement::Fire:
        return QStringLiteral("fire");
    case FiveElement::Earth:
        return QStringLiteral("earth");
    case FiveElement::Metal:
        return QStringLiteral("metal");
    case FiveElement::Water:
        return QStringLiteral("water");
    }

    return QStringLiteral("earth");
}

void incrementFiveElementCount(QVariantMap *counts, FiveElement element)
{
    const QString key = fiveElementKey(element);
    const int currentValue = counts->value(key).toInt();
    counts->insert(key, currentValue + 1);
}

Season seasonForBranch(int branchIndex)
{
    switch (branchIndex) {
    case 2:
    case 3:
    case 4:
        return Season::Spring;
    case 5:
    case 6:
    case 7:
        return Season::Summer;
    case 8:
    case 9:
    case 10:
        return Season::Autumn;
    case 11:
    case 0:
    case 1:
        return Season::Winter;
    default:
        return Season::Spring;
    }
}

QString seasonLabel(Season season)
{
    switch (season) {
    case Season::Spring:
        return QStringLiteral("春");
    case Season::Summer:
        return QStringLiteral("夏");
    case Season::Autumn:
        return QStringLiteral("秋");
    case Season::Winter:
        return QStringLiteral("冬");
    }

    return QStringLiteral("春");
}

FiveElement dominantElementForSeason(Season season)
{
    switch (season) {
    case Season::Spring:
        return FiveElement::Wood;
    case Season::Summer:
        return FiveElement::Fire;
    case Season::Autumn:
        return FiveElement::Metal;
    case Season::Winter:
        return FiveElement::Water;
    }

    return FiveElement::Wood;
}

QString seasonalSuitability(FiveElement dayElement, Season season)
{
    const int relation = positiveModulo(
        static_cast<int>(dominantElementForSeason(season)) - static_cast<int>(dayElement),
        5
    );

    switch (relation) {
    case 0:
    case 4:
        return QStringLiteral("有利");
    case 1:
        return QStringLiteral("中立");
    case 2:
    case 3:
        return QStringLiteral("不利");
    default:
        return QStringLiteral("中立");
    }
}

FiveElement generatingElementFor(FiveElement element)
{
    return static_cast<FiveElement>(positiveModulo(static_cast<int>(element) - 1, 5));
}

FiveElement generatedElementFrom(FiveElement element)
{
    return static_cast<FiveElement>((static_cast<int>(element) + 1) % 5);
}

FiveElement controllingElementFor(FiveElement element)
{
    return static_cast<FiveElement>(positiveModulo(static_cast<int>(element) + 3, 5));
}

QString tenGodName(int dayStemIndex, int targetStemIndex)
{
    if (dayStemIndex < 0 || targetStemIndex < 0) {
        return QStringLiteral("未対応");
    }

    if (dayStemIndex == targetStemIndex) {
        return QStringLiteral("日主");
    }

    const int elementRelation = positiveModulo(
        static_cast<int>(fiveElementForStem(targetStemIndex)) - static_cast<int>(fiveElementForStem(dayStemIndex)),
        5
    );
    const bool samePolarity = isYangStem(dayStemIndex) == isYangStem(targetStemIndex);

    switch (elementRelation) {
    case 0:
        return samePolarity ? QStringLiteral("比肩") : QStringLiteral("劫財");
    case 1:
        return samePolarity ? QStringLiteral("食神") : QStringLiteral("傷官");
    case 2:
        return samePolarity ? QStringLiteral("偏財") : QStringLiteral("正財");
    case 3:
        return samePolarity ? QStringLiteral("偏官") : QStringLiteral("正官");
    case 4:
        return samePolarity ? QStringLiteral("偏印") : QStringLiteral("印綬");
    default:
        return QStringLiteral("未対応");
    }
}

int firstHourStemIndexForDayStem(int dayStemIndex)
{
    switch (dayStemIndex) {
    case 0:
    case 5:
        return 0;
    case 1:
    case 6:
        return 2;
    case 2:
    case 7:
        return 4;
    case 3:
    case 8:
        return 6;
    case 4:
    case 9:
        return 8;
    default:
        return -1;
    }
}

QString temperatureLabelForBranch(int branchIndex)
{
    switch (branchIndex) {
    case 2:
    case 3:
        return QStringLiteral("やや暖");
    case 4:
        return QStringLiteral("中庸");
    case 5:
        return QStringLiteral("暖");
    case 6:
        return QStringLiteral("熱");
    case 7:
        return QStringLiteral("暖");
    case 8:
    case 9:
        return QStringLiteral("涼");
    case 10:
        return QStringLiteral("中庸");
    case 11:
    case 0:
    case 1:
        return QStringLiteral("寒");
    default:
        return QStringLiteral("中庸");
    }
}

QString moistureLabelForBranch(int branchIndex)
{
    switch (branchIndex) {
    case 2:
    case 3:
        return QStringLiteral("やや湿");
    case 4:
        return QStringLiteral("やや湿");
    case 5:
        return QStringLiteral("やや乾");
    case 6:
        return QStringLiteral("乾");
    case 7:
        return QStringLiteral("中庸");
    case 8:
    case 9:
        return QStringLiteral("やや乾");
    case 10:
        return QStringLiteral("やや乾");
    case 11:
        return QStringLiteral("やや湿");
    case 0:
        return QStringLiteral("湿");
    case 1:
        return QStringLiteral("やや湿");
    default:
        return QStringLiteral("中庸");
    }
}
}

ChartResult ChartCalculator::calculate(const BirthInfo &birthInfo) const
{
    const QString yearPillar = calculateYearPillar(birthInfo);
    const SolarTermResolution monthResolution = calculateMonthPillarResolution(birthInfo, yearPillar);
    const QString dayPillar = calculateDayPillar(birthInfo);
    const QString hourPillar = calculateHourPillar(birthInfo, dayPillar);
    const QVariantMap tenGods = calculateTenGods(
        yearPillar,
        monthResolution.monthPillar,
        dayPillar,
        hourPillar
    );
    const QVariantMap hiddenStems = calculateHiddenStems(
        yearPillar,
        monthResolution.monthPillar,
        dayPillar,
        hourPillar
    );
    QString fiveElementDistributionStatusMessage;
    const QVariantMap fiveElements = calculateFiveElements(
        yearPillar,
        monthResolution.monthPillar,
        dayPillar,
        hourPillar,
        hiddenStems,
        &fiveElementDistributionStatusMessage
    );
    QString seasonalEvaluationStatusMessage;
    const QVariantMap seasonalEvaluation = calculateSeasonalEvaluation(
        monthResolution.monthPillar,
        dayPillar,
        &seasonalEvaluationStatusMessage
    );
    QString strengthEvaluationStatusMessage;
    const QVariantMap strengthEvaluation = calculateStrengthEvaluation(
        dayPillar,
        fiveElements,
        seasonalEvaluation,
        &strengthEvaluationStatusMessage
    );
    QString climateEvaluationStatusMessage;
    const QVariantMap climateEvaluation = calculateClimateEvaluation(
        monthResolution.monthPillar,
        seasonalEvaluation,
        &climateEvaluationStatusMessage
    );
    const QString description = buildDescription(
        birthInfo,
        yearPillar,
        monthResolution,
        dayPillar,
        hourPillar
    );

    return {
        yearPillar,
        monthResolution.monthPillar,
        dayPillar,
        hourPillar,
        description,
        monthResolution.statusMessage,
        tenGods,
        hiddenStems,
        fiveElements,
        fiveElementDistributionStatusMessage,
        seasonalEvaluation,
        seasonalEvaluationStatusMessage,
        strengthEvaluation,
        strengthEvaluationStatusMessage,
        climateEvaluation,
        climateEvaluationStatusMessage
    };
}

QString ChartCalculator::calculateYearPillar(const BirthInfo &birthInfo) const
{
    const QDate birthDate = QDate::fromString(birthInfo.birthDate, QStringLiteral("yyyy-MM-dd"));
    if (!birthDate.isValid()) {
        return QStringLiteral("年柱未計算");
    }

    // 1984 年を甲子年の基準として暦年ベースで算出する。
    // 立春基準の年切り替えは未実装。
    const int offset = positiveModulo(birthDate.year() - 1984, 60);
    return heavenlyStemAt(offset % 10) + earthlyBranchAt(offset % 12);
}

SolarTermResolution ChartCalculator::calculateMonthPillarResolution(
    const BirthInfo &birthInfo,
    const QString &yearPillar
) const
{
    return m_solarTermResolver.resolveMonthPillar(birthInfo, yearPillar);
}

QString ChartCalculator::calculateDayPillar(const BirthInfo &birthInfo) const
{
    const QDate birthDate = QDate::fromString(birthInfo.birthDate, QStringLiteral("yyyy-MM-dd"));
    if (!birthDate.isValid()) {
        return QStringLiteral("日柱未計算");
    }

    // 1984-02-02 を丙寅日とする最小基準で暦日ベース算出する。
    // 地方時補正や日界の厳密判定は未実装。
    const QDate referenceDate(1984, 2, 2);
    const int dayIndex = positiveModulo(2 + referenceDate.daysTo(birthDate), 60);
    return heavenlyStemAt(dayIndex % 10) + earthlyBranchAt(dayIndex % 12);
}

QString ChartCalculator::calculateHourPillar(const BirthInfo &birthInfo, const QString &dayPillar) const
{
    const QTime birthTime = QTime::fromString(birthInfo.birthTime, QStringLiteral("HH:mm"));
    if (!birthTime.isValid()) {
        return QStringLiteral("時柱未計算");
    }

    const int minutes = birthTime.hour() * 60 + birthTime.minute();
    const int branchIndex = positiveModulo(((minutes + 60) / 120), 12);
    const int dayStemIndex = heavenlyStemIndex(dayPillar);
    const int firstStemIndex = firstHourStemIndexForDayStem(dayStemIndex);

    if (firstStemIndex < 0) {
        return QStringLiteral("時柱未対応");
    }

    return heavenlyStemAt((firstStemIndex + branchIndex) % 10) + earthlyBranchAt(branchIndex);
}

QVariantMap ChartCalculator::calculateTenGods(
    const QString &yearPillar,
    const QString &monthPillar,
    const QString &dayPillar,
    const QString &hourPillar
) const
{
    const int dayStemIndex = heavenlyStemIndex(dayPillar);
    const int yearStemIndex = heavenlyStemIndex(yearPillar);
    const int monthStemIndex = heavenlyStemIndex(monthPillar);
    const int hourStemIndex = heavenlyStemIndex(hourPillar);

    const QString yearTenGod = dayStemIndex >= 0 && yearStemIndex >= 0
        ? tenGodName(dayStemIndex, yearStemIndex)
        : QStringLiteral("未対応");
    const QString monthTenGod = dayStemIndex >= 0 && monthStemIndex >= 0
        ? tenGodName(dayStemIndex, monthStemIndex)
        : QStringLiteral("未対応");
    const QString dayTenGod = dayStemIndex >= 0
        ? QStringLiteral("日主")
        : QStringLiteral("未対応");
    const QString hourTenGod = dayStemIndex >= 0 && hourStemIndex >= 0
        ? tenGodName(dayStemIndex, hourStemIndex)
        : QStringLiteral("未対応");

    return {
        {QStringLiteral("yearPillar"), yearTenGod},
        {QStringLiteral("monthPillar"), monthTenGod},
        {QStringLiteral("dayPillar"), dayTenGod},
        {QStringLiteral("hourPillar"), hourTenGod}
    };
}

QVariantMap ChartCalculator::calculateHiddenStems(
    const QString &yearPillar,
    const QString &monthPillar,
    const QString &dayPillar,
    const QString &hourPillar
) const
{
    return {
        {QStringLiteral("yearPillar"), hiddenStemsForPillar(yearPillar)},
        {QStringLiteral("monthPillar"), hiddenStemsForPillar(monthPillar)},
        {QStringLiteral("dayPillar"), hiddenStemsForPillar(dayPillar)},
        {QStringLiteral("hourPillar"), hiddenStemsForPillar(hourPillar)}
    };
}

QVariantMap ChartCalculator::calculateFiveElements(
    const QString &yearPillar,
    const QString &monthPillar,
    const QString &dayPillar,
    const QString &hourPillar,
    const QVariantMap &hiddenStems,
    QString *statusMessage
) const
{
    QVariantMap counts{
        {QStringLiteral("wood"), 0},
        {QStringLiteral("fire"), 0},
        {QStringLiteral("earth"), 0},
        {QStringLiteral("metal"), 0},
        {QStringLiteral("water"), 0}
    };
    QStringList excludedPillars;

    const auto accumulatePillar = [&](const QString &pillarKey, const QString &label, const QString &pillar) {
        const int stemIndex = heavenlyStemIndex(pillar);
        const int branchIndex = earthlyBranchIndex(pillar);

        if (stemIndex < 0 || branchIndex < 0) {
            excludedPillars << label;
            return;
        }

        incrementFiveElementCount(&counts, fiveElementForStem(stemIndex));
        incrementFiveElementCount(&counts, fiveElementForBranch(branchIndex));

        const QStringList pillarHiddenStems = hiddenStems.value(pillarKey).toStringList();
        for (const QString &hiddenStem : pillarHiddenStems) {
            const int hiddenStemIndex = heavenlyStemIndex(hiddenStem);
            if (hiddenStemIndex < 0) {
                continue;
            }

            incrementFiveElementCount(&counts, fiveElementForStem(hiddenStemIndex));
        }
    };

    accumulatePillar(QStringLiteral("yearPillar"), QStringLiteral("年柱"), yearPillar);
    accumulatePillar(QStringLiteral("monthPillar"), QStringLiteral("月柱"), monthPillar);
    accumulatePillar(QStringLiteral("dayPillar"), QStringLiteral("日柱"), dayPillar);
    accumulatePillar(QStringLiteral("hourPillar"), QStringLiteral("時柱"), hourPillar);

    if (statusMessage) {
        if (excludedPillars.isEmpty()) {
            *statusMessage = QStringLiteral("四柱天干・地支・蔵干を単純件数で集計しています。");
        } else {
            *statusMessage = QStringLiteral("%1 が未対応のため、その柱由来の五行を除外して集計しています。")
                                 .arg(excludedPillars.join(QStringLiteral("、")));
        }
    }

    return counts;
}

QVariantMap ChartCalculator::calculateSeasonalEvaluation(
    const QString &monthPillar,
    const QString &dayPillar,
    QString *statusMessage
) const
{
    const int monthBranchIndex = earthlyBranchIndex(monthPillar);
    const int dayStemIndex = heavenlyStemIndex(dayPillar);

    if (monthBranchIndex < 0) {
        if (statusMessage) {
            *statusMessage = QStringLiteral("月柱未対応のため、季節評価は未対応です。");
        }

        return {
            {QStringLiteral("monthBranch"), QStringLiteral("未対応")},
            {QStringLiteral("season"), QStringLiteral("未対応")},
            {QStringLiteral("suitability"), QStringLiteral("未対応")}
        };
    }

    if (dayStemIndex < 0) {
        if (statusMessage) {
            *statusMessage = QStringLiteral("日干を取得できないため、季節適性は未対応です。");
        }

        return {
            {QStringLiteral("monthBranch"), earthlyBranchAt(monthBranchIndex)},
            {QStringLiteral("season"), seasonLabel(seasonForBranch(monthBranchIndex))},
            {QStringLiteral("suitability"), QStringLiteral("未対応")}
        };
    }

    const Season season = seasonForBranch(monthBranchIndex);
    const QString suitability = seasonalSuitability(fiveElementForStem(dayStemIndex), season);

    if (statusMessage) {
        *statusMessage = QStringLiteral("月支ベースの季節評価による最小判定です。");
    }

    return {
        {QStringLiteral("monthBranch"), earthlyBranchAt(monthBranchIndex)},
        {QStringLiteral("season"), seasonLabel(season)},
        {QStringLiteral("suitability"), suitability}
    };
}

QVariantMap ChartCalculator::calculateStrengthEvaluation(
    const QString &dayPillar,
    const QVariantMap &fiveElements,
    const QVariantMap &seasonalEvaluation,
    QString *statusMessage
) const
{
    const int dayStemIndex = heavenlyStemIndex(dayPillar);
    const QString seasonalSuitabilityLabel = seasonalEvaluation.value(QStringLiteral("suitability")).toString();

    if (dayStemIndex < 0) {
        if (statusMessage) {
            *statusMessage = QStringLiteral("日干を取得できないため、暫定強弱評価は未対応です。");
        }

        return {
            {QStringLiteral("label"), QStringLiteral("未対応")},
            {QStringLiteral("reason"), QStringLiteral("日干を取得できません。")},
            {QStringLiteral("score"), 0}
        };
    }

    if (seasonalSuitabilityLabel == QStringLiteral("未対応")) {
        if (statusMessage) {
            *statusMessage = QStringLiteral("月柱未対応のため、暫定強弱評価は未対応です。");
        }

        return {
            {QStringLiteral("label"), QStringLiteral("未対応")},
            {QStringLiteral("reason"), QStringLiteral("月支による季節評価を取得できません。")},
            {QStringLiteral("score"), 0}
        };
    }

    const FiveElement dayElement = fiveElementForStem(dayStemIndex);
    const FiveElement supportiveElement = generatingElementFor(dayElement);
    const FiveElement drainingElement = generatedElementFrom(dayElement);
    const FiveElement controllingElement = controllingElementFor(dayElement);

    const int selfCount = fiveElements.value(fiveElementKey(dayElement)).toInt();
    const int supportiveCount = fiveElements.value(fiveElementKey(supportiveElement)).toInt();
    const int drainingCount = fiveElements.value(fiveElementKey(drainingElement)).toInt();
    const int controllingCount = fiveElements.value(fiveElementKey(controllingElement)).toInt();

    int score = (selfCount + supportiveCount) - (drainingCount + controllingCount);
    if (seasonalSuitabilityLabel == QStringLiteral("有利")) {
        score += 1;
    } else if (seasonalSuitabilityLabel == QStringLiteral("不利")) {
        score -= 1;
    }

    QString label = QStringLiteral("中立寄り");
    if (score >= 2) {
        label = QStringLiteral("やや強め");
    } else if (score <= -2) {
        label = QStringLiteral("やや弱め");
    }

    const QString reason = QStringLiteral(
        "同五行 %1 件、生扶五行 %2 件、泄耗・剋の側 %3 件、季節評価 %4 をもとにした暫定判定です。"
    ).arg(selfCount).arg(supportiveCount).arg(drainingCount + controllingCount).arg(seasonalSuitabilityLabel);

    if (statusMessage) {
        *statusMessage = QStringLiteral("五行分布と季節評価を使った暫定的な強弱評価です。");
    }

    return {
        {QStringLiteral("label"), label},
        {QStringLiteral("reason"), reason},
        {QStringLiteral("score"), score}
    };
}

QVariantMap ChartCalculator::calculateClimateEvaluation(
    const QString &monthPillar,
    const QVariantMap &seasonalEvaluation,
    QString *statusMessage
) const
{
    const int monthBranchIndex = earthlyBranchIndex(monthPillar);
    const QString season = seasonalEvaluation.value(QStringLiteral("season")).toString();

    if (monthBranchIndex < 0) {
        if (statusMessage) {
            *statusMessage = QStringLiteral("月柱未対応のため、寒暖・乾湿評価は未対応です。");
        }

        return {
            {QStringLiteral("monthBranch"), QStringLiteral("未対応")},
            {QStringLiteral("temperature"), QStringLiteral("未対応")},
            {QStringLiteral("moisture"), QStringLiteral("未対応")},
            {QStringLiteral("note"), QStringLiteral("月支を取得できません。")}
        };
    }

    const QString temperature = temperatureLabelForBranch(monthBranchIndex);
    const QString moisture = moistureLabelForBranch(monthBranchIndex);
    const QString monthBranch = earthlyBranchAt(monthBranchIndex);
    const QString note = QStringLiteral(
        "月支 %1 と季節 %2 をもとにした調候前提の簡易評価です。"
    ).arg(monthBranch, season.isEmpty() ? QStringLiteral("未対応") : season);

    if (statusMessage) {
        *statusMessage = QStringLiteral("月支ベースの寒暖・乾湿に関する最小評価です。");
    }

    return {
        {QStringLiteral("monthBranch"), monthBranch},
        {QStringLiteral("temperature"), temperature},
        {QStringLiteral("moisture"), moisture},
        {QStringLiteral("note"), note}
    };
}

QString ChartCalculator::buildDescription(
    const BirthInfo &birthInfo,
    const QString &yearPillar,
    const SolarTermResolution &monthResolution,
    const QString &dayPillar,
    const QString &hourPillar
) const
{
    QStringList lines;

    lines << QStringLiteral("一般四柱推命の共通計算基盤の途中実装です。")
          << QStringLiteral("年柱は暦年ベースで計算しています。立春基準の年切り替えは未対応です。")
          << QStringLiteral("月柱は節入り判定責務を分離済みです。")
          << monthResolution.statusMessage
          << QStringLiteral("日柱は暦日ベースの最小実装です。地方時補正や日界の厳密判定は未対応です。")
          << QStringLiteral("時柱は日干と出生時刻から最小実装で計算しています。")
          << QStringLiteral("通変星は日干を基準に、年干・月干・時干の天干どうしのみ最小実装しています。")
          << QStringLiteral("蔵干は各支に対応する一般的な一覧のみ最小実装しています。")
          << QStringLiteral("五行分布は四柱天干・地支・蔵干を単純件数で最小集計しています。")
          << QStringLiteral("季節評価は月支ベースで春夏秋冬と日干適性を最小判定しています。")
          << QStringLiteral("強弱評価は五行分布と季節評価を使った暫定ラベルです。正式な身強身弱判定ではありません。")
          << QStringLiteral("寒暖・乾湿評価は月支ベースの調候前提情報を最小実装しています。");

    if (!birthInfo.birthDate.isEmpty() || !birthInfo.birthTime.isEmpty() || !birthInfo.gender.isEmpty()) {
        lines << QStringLiteral("入力値: %1 / %2 / %3")
                     .arg(birthInfo.birthDate.isEmpty() ? QStringLiteral("未入力") : birthInfo.birthDate)
                     .arg(birthInfo.birthTime.isEmpty() ? QStringLiteral("未入力") : birthInfo.birthTime)
                     .arg(birthInfo.gender.isEmpty() ? QStringLiteral("未入力") : birthInfo.gender);
    }

    lines << QStringLiteral("計算結果: %1 / %2 / %3 / %4")
                 .arg(yearPillar)
                 .arg(monthResolution.monthPillar)
                 .arg(dayPillar)
                 .arg(hourPillar);

    return lines.join(QLatin1Char('\n'));
}
