#include "ChartCalculator.h"
#include "SolarTermDataSource.h"

#include <cmath>
#include <QDate>
#include <QDateTime>
#include <QSet>
#include <algorithm>
#include <QTime>
#include <QTimeZone>

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

QString yearPillarForGregorianYear(int year)
{
    const int offset = ((year - 1984) % 60 + 60) % 60;
    return heavenlyStemAt(offset % 10) + earthlyBranchAt(offset % 12);
}

int positiveModulo(int value, int divisor)
{
    const int remainder = value % divisor;
    return remainder < 0 ? remainder + divisor : remainder;
}

QDateTime birthDateTimeForCalculation(
    const BirthInfo &birthInfo,
    const QTimeZone &timeZone = QTimeZone::systemTimeZone()
)
{
    const QDate birthDate = QDate::fromString(birthInfo.birthDate, QStringLiteral("yyyy-MM-dd"));
    if (!birthDate.isValid()) {
        return {};
    }

    QTime birthTime = QTime::fromString(birthInfo.birthTime, QStringLiteral("HH:mm"));
    if (!birthTime.isValid()) {
        birthTime = QTime(0, 0);
    }

    return QDateTime(birthDate, birthTime, timeZone);
}

QDate dayBoundaryAdjustedDate(const BirthInfo &birthInfo)
{
    const QDate birthDate = QDate::fromString(birthInfo.birthDate, QStringLiteral("yyyy-MM-dd"));
    if (!birthDate.isValid()) {
        return {};
    }

    const QTime birthTime = QTime::fromString(birthInfo.birthTime, QStringLiteral("HH:mm"));
    if (birthTime.isValid() && birthTime >= QTime(23, 0)) {
        return birthDate.addDays(1);
    }

    return birthDate;
}

QDateTime solarTermDateTimeForTerm(const SolarTermYearData &yearData, const QString &termName)
{
    for (const SolarTermEntry &entry : yearData.entries) {
        if (entry.termName == termName && entry.atDateTime.isValid()) {
            return entry.atDateTime;
        }
    }

    return {};
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

QString twelvePhaseName(int dayStemIndex, int targetBranchIndex)
{
    if (dayStemIndex < 0 || targetBranchIndex < 0) {
        return QStringLiteral("未対応");
    }

    static const QStringList phases{
        QStringLiteral("長生"),
        QStringLiteral("沐浴"),
        QStringLiteral("冠帯"),
        QStringLiteral("建禄"),
        QStringLiteral("帝旺"),
        QStringLiteral("衰"),
        QStringLiteral("病"),
        QStringLiteral("死"),
        QStringLiteral("墓"),
        QStringLiteral("絶"),
        QStringLiteral("胎"),
        QStringLiteral("養")
    };
    static const int startingBranchIndexes[]{
        11, 6, 2, 9, 2, 9, 5, 0, 8, 3
    };

    // 一般四柱推命の共通基盤として、各日干の長生起点から十二運を求める。
    const int startingBranchIndex = startingBranchIndexes[dayStemIndex];
    const int phaseIndex = isYangStem(dayStemIndex)
        ? positiveModulo(targetBranchIndex - startingBranchIndex, 12)
        : positiveModulo(startingBranchIndex - targetBranchIndex, 12);
    return phases.at(phaseIndex);
}

QString tenGodForFortunePillar(const QString &dayPillar, const QString &targetPillar)
{
    return tenGodName(
        heavenlyStemIndex(dayPillar),
        heavenlyStemIndex(targetPillar)
    );
}

QString twelvePhaseForFortunePillar(const QString &dayPillar, const QString &targetPillar)
{
    return twelvePhaseName(
        heavenlyStemIndex(dayPillar),
        earthlyBranchIndex(targetPillar)
    );
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

QString fiveElementDisplayName(FiveElement element)
{
    switch (element) {
    case FiveElement::Wood:
        return QStringLiteral("木");
    case FiveElement::Fire:
        return QStringLiteral("火");
    case FiveElement::Earth:
        return QStringLiteral("土");
    case FiveElement::Metal:
        return QStringLiteral("金");
    case FiveElement::Water:
        return QStringLiteral("水");
    }

    return QStringLiteral("土");
}

QString patternNameForTenGod(const QString &tenGod)
{
    if (tenGod == QStringLiteral("正官")) {
        return QStringLiteral("正官格");
    }
    if (tenGod == QStringLiteral("偏官")) {
        return QStringLiteral("偏官格");
    }
    if (tenGod == QStringLiteral("正財")) {
        return QStringLiteral("正財格");
    }
    if (tenGod == QStringLiteral("偏財")) {
        return QStringLiteral("偏財格");
    }
    if (tenGod == QStringLiteral("食神")) {
        return QStringLiteral("食神格");
    }
    if (tenGod == QStringLiteral("傷官")) {
        return QStringLiteral("傷官格");
    }
    if (tenGod == QStringLiteral("印綬")) {
        return QStringLiteral("印綬格");
    }
    if (tenGod == QStringLiteral("偏印")) {
        return QStringLiteral("偏印格");
    }

    return QString();
}

QString pillarAtOffset(const QString &pillar, int offset)
{
    const int stemIndex = heavenlyStemIndex(pillar);
    const int branchIndex = earthlyBranchIndex(pillar);
    if (stemIndex < 0 || branchIndex < 0) {
        return QStringLiteral("未対応");
    }

    return heavenlyStemAt(positiveModulo(stemIndex + offset, 10))
        + earthlyBranchAt(positiveModulo(branchIndex + offset, 12));
}

int signedMajorFortuneOffset(const QString &direction, int sequenceIndex)
{
    if (sequenceIndex < 0) {
        return 0;
    }

    const int baseOffset = sequenceIndex + 1;
    return direction == QStringLiteral("逆行") ? -baseOffset : baseOffset;
}

struct PillarReference
{
    QString label;
    QString pillar;
};

QList<PillarReference> natalPillarReferences(
    const QString &yearPillar,
    const QString &monthPillar,
    const QString &dayPillar,
    const QString &hourPillar
)
{
    return {
        {QStringLiteral("年柱"), yearPillar},
        {QStringLiteral("月柱"), monthPillar},
        {QStringLiteral("日柱"), dayPillar},
        {QStringLiteral("時柱"), hourPillar}
    };
}

bool branchesAreInClash(int leftBranchIndex, int rightBranchIndex)
{
    if (leftBranchIndex < 0 || rightBranchIndex < 0) {
        return false;
    }

    return positiveModulo(leftBranchIndex - rightBranchIndex, 12) == 6;
}

bool branchesAreInHarm(int leftBranchIndex, int rightBranchIndex)
{
    if (leftBranchIndex < 0 || rightBranchIndex < 0) {
        return false;
    }

    const QSet<QPair<int, int>> harmPairs{
        qMakePair(0, 7),
        qMakePair(1, 6),
        qMakePair(2, 5),
        qMakePair(3, 4),
        qMakePair(8, 11),
        qMakePair(9, 10)
    };
    const auto normalizedPair = qMakePair(
        std::min(leftBranchIndex, rightBranchIndex),
        std::max(leftBranchIndex, rightBranchIndex)
    );
    return harmPairs.contains(normalizedPair);
}

bool branchesAreInBreak(int leftBranchIndex, int rightBranchIndex)
{
    if (leftBranchIndex < 0 || rightBranchIndex < 0) {
        return false;
    }

    const QSet<QPair<int, int>> breakPairs{
        qMakePair(0, 9),
        qMakePair(3, 6),
        qMakePair(4, 1),
        qMakePair(7, 10),
        qMakePair(2, 11),
        qMakePair(5, 8)
    };
    const auto normalizedPair = qMakePair(
        std::min(leftBranchIndex, rightBranchIndex),
        std::max(leftBranchIndex, rightBranchIndex)
    );
    return breakPairs.contains(normalizedPair);
}

bool branchesAreInPunishment(int leftBranchIndex, int rightBranchIndex)
{
    if (leftBranchIndex < 0 || rightBranchIndex < 0) {
        return false;
    }

    if (leftBranchIndex == rightBranchIndex) {
        return leftBranchIndex == 4
            || leftBranchIndex == 6
            || leftBranchIndex == 9
            || leftBranchIndex == 11;
    }

    const QSet<QPair<int, int>> punishmentPairs{
        qMakePair(2, 5),
        qMakePair(2, 8),
        qMakePair(5, 8),
        qMakePair(1, 7),
        qMakePair(1, 10),
        qMakePair(7, 10),
        qMakePair(0, 3)
    };
    const auto normalizedPair = qMakePair(
        std::min(leftBranchIndex, rightBranchIndex),
        std::max(leftBranchIndex, rightBranchIndex)
    );
    return punishmentPairs.contains(normalizedPair);
}

bool stemsAreCombinationCandidates(int leftStemIndex, int rightStemIndex)
{
    if (leftStemIndex < 0 || rightStemIndex < 0) {
        return false;
    }

    return positiveModulo(leftStemIndex - rightStemIndex, 10) == 5;
}

QString joinLabelsOrFallback(const QStringList &labels)
{
    return labels.isEmpty() ? QStringLiteral("なし") : labels.join(QStringLiteral("・"));
}

int threeHarmonyGroupIndex(int branchIndex)
{
    switch (branchIndex) {
    case 8:
    case 0:
    case 4:
        return 0;
    case 11:
    case 3:
    case 7:
        return 1;
    case 2:
    case 6:
    case 10:
        return 2;
    case 5:
    case 9:
    case 1:
        return 3;
    default:
        return -1;
    }
}

int directionalCombinationGroupIndex(int branchIndex)
{
    switch (branchIndex) {
    case 11:
    case 0:
    case 1:
        return 0;
    case 2:
    case 3:
    case 4:
        return 1;
    case 5:
    case 6:
    case 7:
        return 2;
    case 8:
    case 9:
    case 10:
        return 3;
    default:
        return -1;
    }
}

QVariantMap calculatePillarRelation(
    const QString &targetPillar,
    const QList<PillarReference> &natalPillars
)
{
    // 行運干支と原命式四柱を比較し、最小関係候補だけを抽出する。
    const int targetStemIndex = heavenlyStemIndex(targetPillar);
    const int targetBranchIndex = earthlyBranchIndex(targetPillar);

    if (targetStemIndex < 0 || targetBranchIndex < 0) {
        return {
            {QStringLiteral("sameStemMatches"), QStringList{}},
            {QStringLiteral("sameBranchMatches"), QStringList{}},
            {QStringLiteral("clashBranches"), QStringList{}},
            {QStringLiteral("harmBranches"), QStringList{}},
            {QStringLiteral("breakBranches"), QStringList{}},
            {QStringLiteral("punishmentBranches"), QStringList{}},
            {QStringLiteral("stemCombinationCandidates"), QStringList{}},
            {QStringLiteral("threeHarmonyCandidates"), QStringList{}},
            {QStringLiteral("directionalCombinationCandidates"), QStringList{}},
            {QStringLiteral("relationSummary"), QStringLiteral("命式との関係候補は未対応です。")},
            {QStringLiteral("relationNote"), QStringLiteral("行運干支を取得できないため、原命式との関係候補を比較できません。")}
        };
    }

    QStringList sameStemMatches;
    QStringList sameBranchMatches;
    QStringList clashBranches;
    QStringList harmBranches;
    QStringList breakBranches;
    QStringList punishmentBranches;
    QStringList stemCombinationCandidates;
    QStringList threeHarmonyCandidates;
    QStringList directionalCombinationCandidates;
    const int targetThreeHarmonyGroupIndex = threeHarmonyGroupIndex(targetBranchIndex);
    const int targetDirectionalGroupIndex = directionalCombinationGroupIndex(targetBranchIndex);

    for (const PillarReference &natalPillar : natalPillars) {
        const int natalStemIndex = heavenlyStemIndex(natalPillar.pillar);
        const int natalBranchIndex = earthlyBranchIndex(natalPillar.pillar);
        if (natalStemIndex < 0 || natalBranchIndex < 0) {
            continue;
        }

        if (natalStemIndex == targetStemIndex) {
            sameStemMatches << natalPillar.label;
        }
        if (natalBranchIndex == targetBranchIndex) {
            sameBranchMatches << natalPillar.label;
        }
        if (branchesAreInClash(targetBranchIndex, natalBranchIndex)) {
            clashBranches << natalPillar.label;
        }
        if (branchesAreInHarm(targetBranchIndex, natalBranchIndex)) {
            harmBranches << natalPillar.label;
        }
        if (branchesAreInBreak(targetBranchIndex, natalBranchIndex)) {
            breakBranches << natalPillar.label;
        }
        if (branchesAreInPunishment(targetBranchIndex, natalBranchIndex)) {
            punishmentBranches << natalPillar.label;
        }
        if (stemsAreCombinationCandidates(targetStemIndex, natalStemIndex)) {
            stemCombinationCandidates << natalPillar.label;
        }
        if (targetThreeHarmonyGroupIndex >= 0
            && targetThreeHarmonyGroupIndex == threeHarmonyGroupIndex(natalBranchIndex)) {
            threeHarmonyCandidates << natalPillar.label;
        }
        if (targetDirectionalGroupIndex >= 0
            && targetDirectionalGroupIndex == directionalCombinationGroupIndex(natalBranchIndex)) {
            directionalCombinationCandidates << natalPillar.label;
        }
    }

    QStringList summaryParts;
    if (!sameStemMatches.isEmpty()) {
        summaryParts << QStringLiteral("同干: %1").arg(joinLabelsOrFallback(sameStemMatches));
    }
    if (!sameBranchMatches.isEmpty()) {
        summaryParts << QStringLiteral("同支: %1").arg(joinLabelsOrFallback(sameBranchMatches));
    }
    if (!clashBranches.isEmpty()) {
        summaryParts << QStringLiteral("冲候補: %1").arg(joinLabelsOrFallback(clashBranches));
    }
    if (!harmBranches.isEmpty()) {
        summaryParts << QStringLiteral("害候補: %1").arg(joinLabelsOrFallback(harmBranches));
    }
    if (!breakBranches.isEmpty()) {
        summaryParts << QStringLiteral("破候補: %1").arg(joinLabelsOrFallback(breakBranches));
    }
    if (!punishmentBranches.isEmpty()) {
        summaryParts << QStringLiteral("刑候補: %1").arg(joinLabelsOrFallback(punishmentBranches));
    }
    if (!stemCombinationCandidates.isEmpty()) {
        summaryParts << QStringLiteral("干合候補: %1").arg(joinLabelsOrFallback(stemCombinationCandidates));
    }
    if (!threeHarmonyCandidates.isEmpty()) {
        summaryParts << QStringLiteral("三合候補: %1").arg(joinLabelsOrFallback(threeHarmonyCandidates));
    }
    if (!directionalCombinationCandidates.isEmpty()) {
        summaryParts << QStringLiteral("方合候補: %1").arg(joinLabelsOrFallback(directionalCombinationCandidates));
    }
    if (summaryParts.isEmpty()) {
        summaryParts << QStringLiteral("該当なし");
    }

    return {
        {QStringLiteral("sameStemMatches"), sameStemMatches},
        {QStringLiteral("sameBranchMatches"), sameBranchMatches},
        {QStringLiteral("clashBranches"), clashBranches},
        {QStringLiteral("harmBranches"), harmBranches},
        {QStringLiteral("breakBranches"), breakBranches},
        {QStringLiteral("punishmentBranches"), punishmentBranches},
        {QStringLiteral("stemCombinationCandidates"), stemCombinationCandidates},
        {QStringLiteral("threeHarmonyCandidates"), threeHarmonyCandidates},
        {QStringLiteral("directionalCombinationCandidates"), directionalCombinationCandidates},
        {QStringLiteral("relationSummary"), summaryParts.join(QStringLiteral(" / "))},
        {QStringLiteral("relationNote"), QStringLiteral("吉凶断定ではなく、原命式との同干・同支・冲・害・破・刑・干合・三合・方合の候補を並べた参考表示です。")}
    };
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
    QString usefulGodCandidatesStatusMessage;
    const QVariantMap usefulGodCandidates = calculateUsefulGodCandidates(
        dayPillar,
        fiveElements,
        seasonalEvaluation,
        strengthEvaluation,
        climateEvaluation,
        &usefulGodCandidatesStatusMessage
    );
    QString patternCandidatesStatusMessage;
    const QVariantMap patternCandidates = calculatePatternCandidates(
        monthResolution.monthPillar,
        dayPillar,
        tenGods,
        hiddenStems,
        fiveElements,
        strengthEvaluation,
        &patternCandidatesStatusMessage
    );
    QString majorFortuneDirectionStatusMessage;
    const QVariantMap majorFortuneDirection = calculateMajorFortuneDirection(
        yearPillar,
        birthInfo,
        &majorFortuneDirectionStatusMessage
    );
    QString solarTermDifferencePreparationStatusMessage;
    const QVariantMap solarTermDifferencePreparation = calculateSolarTermDifferencePreparation(
        birthInfo,
        majorFortuneDirection,
        &solarTermDifferencePreparationStatusMessage
    );
    QString majorFortunesStatusMessage;
    const QVariantList majorFortunes = calculateMajorFortunes(
        birthInfo,
        yearPillar,
        monthResolution.monthPillar,
        dayPillar,
        hourPillar,
        majorFortuneDirection,
        solarTermDifferencePreparation,
        &majorFortunesStatusMessage
    );
    QString annualFortunesStatusMessage;
    const QVariantList annualFortunes = calculateAnnualFortunes(
        birthInfo,
        yearPillar,
        monthResolution.monthPillar,
        dayPillar,
        hourPillar,
        &annualFortunesStatusMessage
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
        climateEvaluationStatusMessage,
        usefulGodCandidates,
        usefulGodCandidatesStatusMessage,
        patternCandidates,
        patternCandidatesStatusMessage,
        majorFortunes,
        majorFortunesStatusMessage,
        annualFortunes,
        annualFortunesStatusMessage,
        majorFortuneDirection,
        majorFortuneDirectionStatusMessage,
        solarTermDifferencePreparation,
        solarTermDifferencePreparationStatusMessage
    };
}

QString ChartCalculator::calculateYearPillar(const BirthInfo &birthInfo) const
{
    const QDate birthDate = QDate::fromString(birthInfo.birthDate, QStringLiteral("yyyy-MM-dd"));
    if (!birthDate.isValid()) {
        return QStringLiteral("年柱未計算");
    }

    SolarTermDataSource dataSource;
    const SolarTermYearData yearData = dataSource.loadYearData(birthDate.year());
    const QDateTime lichunDateTime = solarTermDateTimeForTerm(yearData, QStringLiteral("立春"));

    if (!lichunDateTime.isValid()) {
        return yearPillarForGregorianYear(birthDate.year());
    }

    const QDateTime birthDateTime = birthDateTimeForCalculation(birthInfo, lichunDateTime.timeZone());
    if (!birthDateTime.isValid()) {
        return QStringLiteral("年柱未計算");
    }

    return birthDateTime < lichunDateTime
        ? yearPillarForGregorianYear(birthDate.year() - 1)
        : yearPillarForGregorianYear(birthDate.year());
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
    const QDate dayCalculationDate = dayBoundaryAdjustedDate(birthInfo);
    if (!dayCalculationDate.isValid()) {
        return QStringLiteral("日柱未計算");
    }

    // このプロジェクトでは前日 23:00 開始の日界を採用する。
    const QDate referenceDate(1984, 2, 2);
    const int dayIndex = positiveModulo(2 + referenceDate.daysTo(dayCalculationDate), 60);
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

QVariantMap ChartCalculator::calculateUsefulGodCandidates(
    const QString &dayPillar,
    const QVariantMap &fiveElements,
    const QVariantMap &seasonalEvaluation,
    const QVariantMap &strengthEvaluation,
    const QVariantMap &climateEvaluation,
    QString *statusMessage
) const
{
    const int dayStemIndex = heavenlyStemIndex(dayPillar);
    const QString seasonalSuitability = seasonalEvaluation.value(QStringLiteral("suitability")).toString();
    const QString strengthLabel = strengthEvaluation.value(QStringLiteral("label")).toString();
    const QString temperature = climateEvaluation.value(QStringLiteral("temperature")).toString();
    const QString moisture = climateEvaluation.value(QStringLiteral("moisture")).toString();

    if (dayStemIndex < 0 || seasonalSuitability == QStringLiteral("未対応")
        || temperature == QStringLiteral("未対応") || moisture == QStringLiteral("未対応")) {
        if (statusMessage) {
            *statusMessage = QStringLiteral("月柱または日干の情報不足のため、用神候補は未対応です。");
        }

        return {
            {QStringLiteral("candidates"), QStringList{QStringLiteral("未対応")}},
            {QStringLiteral("reason"), QStringLiteral("季節評価または寒暖・乾湿評価を十分に取得できません。")},
            {QStringLiteral("note"), QStringLiteral("暫定候補を算出できません。")}
        };
    }

    struct CandidateScore
    {
        FiveElement element;
        int score;
    };

    CandidateScore candidateScores[]{
        {FiveElement::Wood, 0},
        {FiveElement::Fire, 0},
        {FiveElement::Earth, 0},
        {FiveElement::Metal, 0},
        {FiveElement::Water, 0}
    };

    const int woodCount = fiveElements.value(QStringLiteral("wood")).toInt();
    const int fireCount = fiveElements.value(QStringLiteral("fire")).toInt();
    const int earthCount = fiveElements.value(QStringLiteral("earth")).toInt();
    const int metalCount = fiveElements.value(QStringLiteral("metal")).toInt();
    const int waterCount = fiveElements.value(QStringLiteral("water")).toInt();
    const int maxCount = std::max({woodCount, fireCount, earthCount, metalCount, waterCount});

    for (CandidateScore &candidate : candidateScores) {
        const int count = fiveElements.value(fiveElementKey(candidate.element)).toInt();
        candidate.score += maxCount - count;
    }

    QStringList reasons;
    reasons << QStringLiteral("五行分布の不足傾向を基準にしています。");

    if (temperature == QStringLiteral("寒")) {
        candidateScores[static_cast<int>(FiveElement::Fire)].score += 3;
        candidateScores[static_cast<int>(FiveElement::Wood)].score += 1;
        reasons << QStringLiteral("寒さを和らげる方向として火を上げています。");
    } else if (temperature == QStringLiteral("涼")) {
        candidateScores[static_cast<int>(FiveElement::Fire)].score += 2;
        reasons << QStringLiteral("涼寄りのため火をやや上げています。");
    } else if (temperature == QStringLiteral("暖")) {
        candidateScores[static_cast<int>(FiveElement::Water)].score += 2;
        reasons << QStringLiteral("暖寄りのため水をやや上げています。");
    } else if (temperature == QStringLiteral("熱")) {
        candidateScores[static_cast<int>(FiveElement::Water)].score += 3;
        reasons << QStringLiteral("熱を冷ます方向として水を上げています。");
    }

    if (moisture == QStringLiteral("乾")) {
        candidateScores[static_cast<int>(FiveElement::Water)].score += 3;
        reasons << QStringLiteral("乾きが強いため水を上げています。");
    } else if (moisture == QStringLiteral("やや乾")) {
        candidateScores[static_cast<int>(FiveElement::Water)].score += 2;
        reasons << QStringLiteral("やや乾き寄りのため水をやや上げています。");
    } else if (moisture == QStringLiteral("湿")) {
        candidateScores[static_cast<int>(FiveElement::Fire)].score += 2;
        candidateScores[static_cast<int>(FiveElement::Earth)].score += 2;
        reasons << QStringLiteral("湿りを動かす方向として火と土を上げています。");
    } else if (moisture == QStringLiteral("やや湿")) {
        candidateScores[static_cast<int>(FiveElement::Fire)].score += 1;
        candidateScores[static_cast<int>(FiveElement::Earth)].score += 1;
        reasons << QStringLiteral("やや湿り寄りのため火と土を補助候補にしています。");
    }

    const FiveElement dayElement = fiveElementForStem(dayStemIndex);
    const FiveElement supportiveElement = generatingElementFor(dayElement);
    const FiveElement drainingElement = generatedElementFrom(dayElement);
    const FiveElement controllingElement = controllingElementFor(dayElement);

    if (strengthLabel == QStringLiteral("やや弱め")) {
        candidateScores[static_cast<int>(dayElement)].score += 2;
        candidateScores[static_cast<int>(supportiveElement)].score += 2;
        reasons << QStringLiteral("やや弱め判定のため日干を助ける側を上げています。");
    } else if (strengthLabel == QStringLiteral("やや強め")) {
        candidateScores[static_cast<int>(drainingElement)].score += 2;
        candidateScores[static_cast<int>(controllingElement)].score += 2;
        reasons << QStringLiteral("やや強め判定のため洩らす側と抑える側を上げています。");
    } else {
        reasons << QStringLiteral("強弱は中立寄りとして極端な補正を避けています。");
    }

    std::sort(std::begin(candidateScores), std::end(candidateScores), [](const CandidateScore &left, const CandidateScore &right) {
        if (left.score == right.score) {
            return static_cast<int>(left.element) < static_cast<int>(right.element);
        }
        return left.score > right.score;
    });

    QStringList candidates;
    for (int index = 0; index < 3; ++index) {
        candidates << fiveElementDisplayName(candidateScores[index].element);
    }

    if (statusMessage) {
        *statusMessage = QStringLiteral("五行分布・季節・寒暖乾湿・暫定強弱から作った断定しない暫定候補です。");
    }

    return {
        {QStringLiteral("candidates"), candidates},
        {QStringLiteral("reason"), reasons.join(QStringLiteral(" "))},
        {QStringLiteral("note"), QStringLiteral("用神を断定せず、補いたい方向の参考候補を表示しています。")}
    };
}

QVariantMap ChartCalculator::calculatePatternCandidates(
    const QString &monthPillar,
    const QString &dayPillar,
    const QVariantMap &tenGods,
    const QVariantMap &hiddenStems,
    const QVariantMap &fiveElements,
    const QVariantMap &strengthEvaluation,
    QString *statusMessage
) const
{
    const int monthBranchIndex = earthlyBranchIndex(monthPillar);
    const int dayStemIndex = heavenlyStemIndex(dayPillar);
    const QString monthTenGod = tenGods.value(QStringLiteral("monthPillar")).toString();
    const QStringList monthHiddenStems = hiddenStems.value(QStringLiteral("monthPillar")).toStringList();

    if (monthBranchIndex < 0 || dayStemIndex < 0 || monthTenGod == QStringLiteral("未対応")) {
        if (statusMessage) {
            *statusMessage = QStringLiteral("月柱または日干の情報不足のため、格局候補は未対応です。");
        }

        return {
            {QStringLiteral("candidates"), QStringList{QStringLiteral("未対応")}},
            {QStringLiteral("reason"), QStringLiteral("月干通変星または月令参照に必要な情報を取得できません。")},
            {QStringLiteral("note"), QStringLiteral("格局候補を暫定表示できません。")}
        };
    }

    QStringList candidates;
    QStringList reasons;

    const QString primaryPattern = patternNameForTenGod(monthTenGod);
    if (!primaryPattern.isEmpty()) {
        candidates << primaryPattern;
        reasons << QStringLiteral("月干通変星 %1 を月柱の主候補として参照しています。").arg(monthTenGod);
    }

    for (const QString &hiddenStem : monthHiddenStems) {
        const int hiddenStemIndex = heavenlyStemIndex(hiddenStem);
        if (hiddenStemIndex < 0) {
            continue;
        }

        const QString hiddenTenGod = tenGodName(dayStemIndex, hiddenStemIndex);
        const QString hiddenPattern = patternNameForTenGod(hiddenTenGod);
        if (hiddenPattern.isEmpty() || candidates.contains(hiddenPattern)) {
            continue;
        }

        candidates << hiddenPattern;
        reasons << QStringLiteral("月支蔵干 %1 の通変星 %2 を月令の参考候補にしています。")
                       .arg(hiddenStem, hiddenTenGod);

        if (candidates.size() >= 3) {
            break;
        }
    }

    if (candidates.isEmpty()) {
        if (statusMessage) {
            *statusMessage = QStringLiteral("月干通変星から格局候補を作れないため、格局候補は未対応です。");
        }

        return {
            {QStringLiteral("candidates"), QStringList{QStringLiteral("未対応")}},
            {QStringLiteral("reason"), QStringLiteral("月干通変星と月支蔵干から対象となる格局候補を抽出できません。")},
            {QStringLiteral("note"), QStringLiteral("現段階では参考候補を出せません。")}
        };
    }

    const QString strengthLabel = strengthEvaluation.value(QStringLiteral("label")).toString();
    if (!strengthLabel.isEmpty() && strengthLabel != QStringLiteral("未対応")) {
        reasons << QStringLiteral("暫定強弱評価 %1 を補助情報として併記しています。").arg(strengthLabel);
    }

    int dominantCount = -1;
    QString dominantElement;
    const QStringList elementKeys{
        QStringLiteral("wood"),
        QStringLiteral("fire"),
        QStringLiteral("earth"),
        QStringLiteral("metal"),
        QStringLiteral("water")
    };
    const QVariantMap elementNames{
        {QStringLiteral("wood"), QStringLiteral("木")},
        {QStringLiteral("fire"), QStringLiteral("火")},
        {QStringLiteral("earth"), QStringLiteral("土")},
        {QStringLiteral("metal"), QStringLiteral("金")},
        {QStringLiteral("water"), QStringLiteral("水")}
    };

    for (const QString &key : elementKeys) {
        const int count = fiveElements.value(key).toInt();
        if (count > dominantCount) {
            dominantCount = count;
            dominantElement = elementNames.value(key).toString();
        }
    }

    if (!dominantElement.isEmpty()) {
        reasons << QStringLiteral("五行分布では %1 が比較的多く、候補理由の補助情報としています。").arg(dominantElement);
    }

    if (statusMessage) {
        *statusMessage = QStringLiteral("月干通変星と月令の簡易参照から作った断定しない暫定候補です。");
    }

    return {
        {QStringLiteral("candidates"), candidates.mid(0, 3)},
        {QStringLiteral("reason"), reasons.join(QStringLiteral(" "))},
        {QStringLiteral("note"), QStringLiteral("格局を断定せず、月柱中心の参考候補を表示しています。")}
    };
}

QVariantList ChartCalculator::calculateMajorFortunes(
    const BirthInfo &birthInfo,
    const QString &yearPillar,
    const QString &monthPillar,
    const QString &dayPillar,
    const QString &hourPillar,
    const QVariantMap &majorFortuneDirection,
    const QVariantMap &solarTermDifferencePreparation,
    QString *statusMessage
) const
{
    if (earthlyBranchIndex(monthPillar) < 0 || heavenlyStemIndex(monthPillar) < 0) {
        if (statusMessage) {
            *statusMessage = QStringLiteral("月柱未対応のため、大運一覧は未対応です。");
        }

        return {
            QVariantMap{
                {QStringLiteral("index"), 0},
                {QStringLiteral("startAge"), -1},
                {QStringLiteral("endAge"), -1},
                {QStringLiteral("label"), QStringLiteral("未対応")},
                {QStringLiteral("pillar"), QStringLiteral("未対応")},
                {QStringLiteral("tenGod"), QStringLiteral("未対応")},
                {QStringLiteral("twelvePhase"), QStringLiteral("未対応")},
                {QStringLiteral("sameStemMatches"), QStringList{}},
                {QStringLiteral("sameBranchMatches"), QStringList{}},
                {QStringLiteral("clashBranches"), QStringList{}},
                {QStringLiteral("harmBranches"), QStringList{}},
                {QStringLiteral("breakBranches"), QStringList{}},
                {QStringLiteral("punishmentBranches"), QStringList{}},
                {QStringLiteral("stemCombinationCandidates"), QStringList{}},
                {QStringLiteral("threeHarmonyCandidates"), QStringList{}},
                {QStringLiteral("directionalCombinationCandidates"), QStringList{}},
                {QStringLiteral("relationSummary"), QStringLiteral("命式との関係候補は未対応です。")},
                {QStringLiteral("relationNote"), QStringLiteral("月柱を取得できないため、原命式との関係候補を比較できません。")},
                {QStringLiteral("note"), QStringLiteral("月柱を取得できないため、大運表示骨格を生成できません。")}
            }
        };
    }

    int startAge = solarTermDifferencePreparation.value(QStringLiteral("calculatedStartAge")).toInt();
    QString startAgeBasisNote = solarTermDifferencePreparation.value(QStringLiteral("note")).toString();
    const QString direction = majorFortuneDirection.value(QStringLiteral("direction")).toString();
    QString statusMessageSuffix = QStringLiteral(
        "正節の節入り差を 3 日 = 1 年で換算し、端数は切り上げる採用仕様です。"
    );

    if (startAge < 0) {
        if (statusMessage) {
            *statusMessage = QStringLiteral("節入り差または順逆を確定できないため、大運開始年齢を計算できません。");
        }

        return {
            QVariantMap{
                {QStringLiteral("index"), 0},
                {QStringLiteral("startAge"), -1},
                {QStringLiteral("endAge"), -1},
                {QStringLiteral("label"), QStringLiteral("未対応")},
                {QStringLiteral("pillar"), QStringLiteral("未対応")},
                {QStringLiteral("tenGod"), QStringLiteral("未対応")},
                {QStringLiteral("twelvePhase"), QStringLiteral("未対応")},
                {QStringLiteral("sameStemMatches"), QStringList{}},
                {QStringLiteral("sameBranchMatches"), QStringList{}},
                {QStringLiteral("clashBranches"), QStringList{}},
                {QStringLiteral("harmBranches"), QStringList{}},
                {QStringLiteral("breakBranches"), QStringList{}},
                {QStringLiteral("punishmentBranches"), QStringList{}},
                {QStringLiteral("stemCombinationCandidates"), QStringList{}},
                {QStringLiteral("threeHarmonyCandidates"), QStringList{}},
                {QStringLiteral("directionalCombinationCandidates"), QStringList{}},
                {QStringLiteral("relationSummary"), QStringLiteral("命式との関係候補は未対応です。")},
                {QStringLiteral("relationNote"), QStringLiteral("順逆または正節データを確定できないため、原命式との関係候補を比較できません。")},
                {QStringLiteral("note"), QStringLiteral("順逆または正節データを確定できないため、起運年齢を計算できません。")}
            }
        };
    }

    QVariantList fortunes;
    const QList<PillarReference> natalPillars = natalPillarReferences(
        yearPillar,
        monthPillar,
        dayPillar,
        hourPillar
    );
    for (int index = 0; index < 8; ++index) {
        const int rangeStartAge = startAge + index * 10;
        const int endAge = rangeStartAge + 9;
        const int pillarOffset = signedMajorFortuneOffset(direction, index);
        const QString fortunePillar = pillarAtOffset(monthPillar, pillarOffset);
        const QString fortuneTenGod = tenGodForFortunePillar(dayPillar, fortunePillar);
        const QString fortuneTwelvePhase = twelvePhaseForFortunePillar(dayPillar, fortunePillar);
        const QVariantMap relation = calculatePillarRelation(fortunePillar, natalPillars);
        const QString traitsNote = (fortuneTenGod == QStringLiteral("未対応")
                || fortuneTwelvePhase == QStringLiteral("未対応"))
            ? QStringLiteral(" 通変星と十二運は日干または行運干支を取得できないため未対応です。")
            : QStringLiteral(" 通変星と十二運は日干基準の最小本実装です。");
        QVariantMap majorFortune{
            {QStringLiteral("index"), index + 1},
            {QStringLiteral("startAge"), rangeStartAge},
            {QStringLiteral("endAge"), endAge},
            {QStringLiteral("label"), QStringLiteral("%1〜%2歳").arg(rangeStartAge).arg(endAge)},
            {QStringLiteral("pillar"), fortunePillar},
            {QStringLiteral("tenGod"), fortuneTenGod},
            {QStringLiteral("twelvePhase"), fortuneTwelvePhase},
            {QStringLiteral("note"), QStringLiteral(
                "起運年齢は %1 順逆は %2 を参照した確定計算で、大運干支は月柱から %3 方向へ %4 として順逆反映済みです。%5 %6"
            ).arg(
                startAgeBasisNote,
                direction.isEmpty() ? QStringLiteral("未対応") : direction,
                direction == QStringLiteral("逆行") ? QStringLiteral("前へ戻る") : QStringLiteral("先へ進む"),
                QString::number(pillarOffset),
                relation.value(QStringLiteral("relationNote")).toString(),
                traitsNote
            )}
        };
        majorFortune.insert(
            QStringLiteral("relationSummary"),
            relation.value(QStringLiteral("relationSummary")).toString().isEmpty()
                ? QStringLiteral("該当なし")
                : relation.value(QStringLiteral("relationSummary")).toString()
        );
        majorFortune.insert(
            QStringLiteral("sameStemMatches"),
            relation.value(QStringLiteral("sameStemMatches")).toStringList()
        );
        majorFortune.insert(
            QStringLiteral("sameBranchMatches"),
            relation.value(QStringLiteral("sameBranchMatches")).toStringList()
        );
        majorFortune.insert(
            QStringLiteral("clashBranches"),
            relation.value(QStringLiteral("clashBranches")).toStringList()
        );
        majorFortune.insert(
            QStringLiteral("stemCombinationCandidates"),
            relation.value(QStringLiteral("stemCombinationCandidates")).toStringList()
        );
        majorFortune.insert(
            QStringLiteral("relationNote"),
            QStringLiteral("吉凶断定ではなく、原命式との関係候補の参考表示です。")
        );
        fortunes.append(majorFortune);
    }

    if (statusMessage) {
        *statusMessage = QStringLiteral(
            "月柱起点で順逆反映済みの大運表示です。起運年齢は節入り差ベースの採用仕様で、%1 通変星と十二運に加えて、原命式との同干・同支・冲・害・破・刑・干合・三合・方合候補を最小表示しています。"
        ).arg(statusMessageSuffix);
    }

    return fortunes;
}

QVariantList ChartCalculator::calculateAnnualFortunes(
    const BirthInfo &birthInfo,
    const QString &yearPillar,
    const QString &monthPillar,
    const QString &dayPillar,
    const QString &hourPillar,
    QString *statusMessage
) const
{
    const QDate birthDate = QDate::fromString(birthInfo.birthDate, QStringLiteral("yyyy-MM-dd"));
    if (!birthDate.isValid()) {
        if (statusMessage) {
            *statusMessage = QStringLiteral("生年月日を取得できないため、流年一覧は未対応です。");
        }

        return {
            QVariantMap{
                {QStringLiteral("year"), 0},
                {QStringLiteral("pillar"), QStringLiteral("未対応")},
                {QStringLiteral("tenGod"), QStringLiteral("未対応")},
                {QStringLiteral("twelvePhase"), QStringLiteral("未対応")},
                {QStringLiteral("sameStemMatches"), QStringList{}},
                {QStringLiteral("sameBranchMatches"), QStringList{}},
                {QStringLiteral("clashBranches"), QStringList{}},
                {QStringLiteral("harmBranches"), QStringList{}},
                {QStringLiteral("breakBranches"), QStringList{}},
                {QStringLiteral("punishmentBranches"), QStringList{}},
                {QStringLiteral("stemCombinationCandidates"), QStringList{}},
                {QStringLiteral("threeHarmonyCandidates"), QStringList{}},
                {QStringLiteral("directionalCombinationCandidates"), QStringList{}},
                {QStringLiteral("relationSummary"), QStringLiteral("命式との関係候補は未対応です。")},
                {QStringLiteral("relationNote"), QStringLiteral("生年月日を取得できないため、原命式との関係候補を比較できません。")},
                {QStringLiteral("note"), QStringLiteral("生年月日を取得できないため、流年表示骨格を生成できません。")}
            }
        };
    }

    QVariantList fortunes;
    const int startYear = birthDate.year();
    const QList<PillarReference> natalPillars = natalPillarReferences(
        yearPillar,
        monthPillar,
        dayPillar,
        hourPillar
    );
    for (int offset = 0; offset < 12; ++offset) {
        const int year = startYear + offset;
        const QString pillar = heavenlyStemAt(positiveModulo(year - 1984, 60) % 10)
            + earthlyBranchAt(positiveModulo(year - 1984, 60) % 12);
        const QString fortuneTenGod = tenGodForFortunePillar(dayPillar, pillar);
        const QString fortuneTwelvePhase = twelvePhaseForFortunePillar(dayPillar, pillar);
        const QVariantMap relation = calculatePillarRelation(pillar, natalPillars);
        const QString traitsNote = (fortuneTenGod == QStringLiteral("未対応")
                || fortuneTwelvePhase == QStringLiteral("未対応"))
            ? QStringLiteral(" 通変星と十二運は日干または行運干支を取得できないため未対応です。")
            : QStringLiteral(" 通変星と十二運は日干基準の最小本実装です。");
        fortunes.append(QVariantMap{
            {QStringLiteral("year"), year},
            {QStringLiteral("pillar"), pillar},
            {QStringLiteral("tenGod"), fortuneTenGod},
            {QStringLiteral("twelvePhase"), fortuneTwelvePhase},
            {QStringLiteral("sameStemMatches"), relation.value(QStringLiteral("sameStemMatches")).toStringList()},
            {QStringLiteral("sameBranchMatches"), relation.value(QStringLiteral("sameBranchMatches")).toStringList()},
            {QStringLiteral("clashBranches"), relation.value(QStringLiteral("clashBranches")).toStringList()},
            {QStringLiteral("harmBranches"), relation.value(QStringLiteral("harmBranches")).toStringList()},
            {QStringLiteral("breakBranches"), relation.value(QStringLiteral("breakBranches")).toStringList()},
            {QStringLiteral("punishmentBranches"), relation.value(QStringLiteral("punishmentBranches")).toStringList()},
            {QStringLiteral("stemCombinationCandidates"), relation.value(QStringLiteral("stemCombinationCandidates")).toStringList()},
            {QStringLiteral("threeHarmonyCandidates"), relation.value(QStringLiteral("threeHarmonyCandidates")).toStringList()},
            {QStringLiteral("directionalCombinationCandidates"), relation.value(QStringLiteral("directionalCombinationCandidates")).toStringList()},
            {QStringLiteral("relationSummary"), relation.value(QStringLiteral("relationSummary")).toString()},
            {QStringLiteral("relationNote"), relation.value(QStringLiteral("relationNote")).toString()},
            {QStringLiteral("note"), QStringLiteral(
                "流年解釈は未実装のため、西暦年から求めた干支と原命式との最小関係候補の参考表示です。%1 %2"
            ).arg(
                relation.value(QStringLiteral("relationNote")).toString(),
                traitsNote
            )}
        });
    }

    if (statusMessage) {
        *statusMessage = QStringLiteral(
            "出生年から 12 年分を並べた流年表示の仮骨格です。流年解釈は未実装ですが、通変星と十二運に加えて原命式との同干・同支・冲・害・破・刑・干合・三合・方合候補を最小表示しています。"
        );
    }

    return fortunes;
}

QVariantMap ChartCalculator::calculateMajorFortuneDirection(
    const QString &yearPillar,
    const BirthInfo &birthInfo,
    QString *statusMessage
) const
{
    const int yearStemIndex = heavenlyStemIndex(yearPillar);
    if (yearStemIndex < 0) {
        if (statusMessage) {
            *statusMessage = QStringLiteral("年干を取得できないため、一般ルールによる順逆判定は未対応です。");
        }

        return {
            {QStringLiteral("direction"), QStringLiteral("未対応")},
            {QStringLiteral("reason"), QStringLiteral("年干を取得できないため、陰陽判定ができません。")},
            {QStringLiteral("note"), QStringLiteral("一般四柱推命の順逆ルールを適用できないため未対応です。")}
        };
    }

    if (birthInfo.gender != QStringLiteral("男性") && birthInfo.gender != QStringLiteral("女性")) {
        if (statusMessage) {
            *statusMessage = QStringLiteral("性別入力が未設定のため、一般ルールによる順逆判定は未対応です。");
        }

        return {
            {QStringLiteral("direction"), QStringLiteral("未対応")},
            {QStringLiteral("reason"), QStringLiteral("性別入力が未設定または未対応値のため、順逆を判定できません。")},
            {QStringLiteral("note"), QStringLiteral("一般四柱推命では性別と年干陰陽の組み合わせを使うため未対応です。")}
        };
    }

    const bool yangYearStem = isYangStem(yearStemIndex);
    const QString yearStem = yearPillar.left(1);
    const QString yinYangLabel = yangYearStem ? QStringLiteral("陽") : QStringLiteral("陰");
    const bool isForward = (birthInfo.gender == QStringLiteral("男性") && yangYearStem)
        || (birthInfo.gender == QStringLiteral("女性") && !yangYearStem);

    if (statusMessage) {
        *statusMessage = QStringLiteral(
            "一般四柱推命の共通基盤として、男性+陽年干 / 女性+陰年干を順行、男性+陰年干 / 女性+陽年干を逆行とする実判定です。"
        );
    }

    return {
        {QStringLiteral("direction"), isForward ? QStringLiteral("順行") : QStringLiteral("逆行")},
        {QStringLiteral("reason"), QStringLiteral(
            "年干 %1 は %2、性別は %3 のため、一般ルールにより %4 と判定しています。"
        ).arg(
            yearStem,
            yinYangLabel,
            birthInfo.gender,
            isForward ? QStringLiteral("順行") : QStringLiteral("逆行")
        )},
        {QStringLiteral("note"), QStringLiteral("泰山流固有ルールではなく、一般四柱推命の共通ルールによる実判定です。")}
    };
}

QVariantMap ChartCalculator::calculateSolarTermDifferencePreparation(
    const BirthInfo &birthInfo,
    const QVariantMap &majorFortuneDirection,
    QString *statusMessage
) const
{
    const QString direction = majorFortuneDirection.value(QStringLiteral("direction")).toString();
    SolarTermDifferenceResolution resolution;
    QString selectionNote;

    if (direction == QStringLiteral("順行")) {
        resolution = m_solarTermResolver.resolveNextSolarTermDifference(birthInfo);
        selectionNote = QStringLiteral("順行のため、出生日時より後の次の正節を参照しています。");
    } else if (direction == QStringLiteral("逆行")) {
        resolution = m_solarTermResolver.resolvePreviousSolarTermDifference(birthInfo);
        selectionNote = QStringLiteral("逆行のため、出生日時より前の前の正節を参照しています。");
    } else {
        resolution = m_solarTermResolver.resolveNearestSolarTermDifference(birthInfo);
        selectionNote = QStringLiteral("順逆が未確定のため、最寄り節入りを暫定参照しています。");
    }

    if (statusMessage) {
        *statusMessage = resolution.statusMessage;
    }

    if (!resolution.canDetermineDifference) {
        return {
            {QStringLiteral("canDetermineDifference"), false},
            {QStringLiteral("birthDateTime"), resolution.birthDateTimeIso.isEmpty()
                 ? QStringLiteral("未対応") : resolution.birthDateTimeIso},
            {QStringLiteral("referenceTerm"), QStringLiteral("未対応")},
            {QStringLiteral("referenceDirection"), QStringLiteral("未対応")},
            {QStringLiteral("referenceDateTime"), QStringLiteral("未対応")},
            {QStringLiteral("differenceMinutes"), 0},
            {QStringLiteral("absoluteDifferenceMinutes"), 0},
            {QStringLiteral("differenceDays"), QStringLiteral("未対応")},
            {QStringLiteral("calculatedStartAge"), -1},
            {QStringLiteral("conversionRule"), QStringLiteral("未対応")},
            {QStringLiteral("note"), QStringLiteral("正節との差分を使う起運計算を完了できません。%1").arg(selectionNote)}
        };
    }

    const double differenceDays = static_cast<double>(resolution.absoluteDifferenceMinutes) / (60.0 * 24.0);
    const int calculatedStartAge =
        calculateFortuneStartAgeFromDifferenceMinutes(resolution.absoluteDifferenceMinutes);

    return {
        {QStringLiteral("canDetermineDifference"), true},
        {QStringLiteral("birthDateTime"), resolution.birthDateTimeIso},
        {QStringLiteral("referenceTerm"), resolution.referenceTermName},
        {QStringLiteral("referenceDirection"), resolution.referenceDirection},
        {QStringLiteral("referenceDateTime"), resolution.referenceDateTimeIso},
        {QStringLiteral("differenceMinutes"), resolution.differenceMinutes},
        {QStringLiteral("absoluteDifferenceMinutes"), resolution.absoluteDifferenceMinutes},
        {QStringLiteral("differenceDays"), QString::number(differenceDays, 'f', 2)},
        {QStringLiteral("calculatedStartAge"), calculatedStartAge},
        {QStringLiteral("conversionRule"), QStringLiteral("差分 3 日 = 1 年、端数切り上げ")},
        {QStringLiteral("note"), QStringLiteral(
            "本アプリ採用仕様として、出生日時と参照正節日時との差分を差分 3 日 = 1 年・端数切り上げで %1 歳起運に換算しています。%2"
        ).arg(calculatedStartAge).arg(selectionNote)}
    };
}

int ChartCalculator::calculateFortuneStartAgeFromDifferenceMinutes(qint64 absoluteDifferenceMinutes) const
{
    if (absoluteDifferenceMinutes <= 0) {
        return 0;
    }

    const double differenceDays = static_cast<double>(absoluteDifferenceMinutes) / (60.0 * 24.0);
    return static_cast<int>(std::ceil(differenceDays / 3.0));
}

int ChartCalculator::calculateTentativeFortuneStartAge(const BirthInfo &birthInfo) const
{
    const QDate birthDate = QDate::fromString(birthInfo.birthDate, QStringLiteral("yyyy-MM-dd"));
    if (!birthDate.isValid()) {
        return -1;
    }

    // 厳密な節入り差計算の前段として、出生月日から 3〜10 歳へ丸めた参考値を返す。
    return 3 + positiveModulo(birthDate.month() + (birthDate.day() >= 15 ? 1 : 0), 8);
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

    lines << QStringLiteral("一般四柱推命の共通計算基盤として、このソフトで採用する暦法と起運計算を反映しています。")
          << QStringLiteral("年柱は立春の節入り日時基準で切り替えます。")
          << QStringLiteral("月柱は正節の節入り日時基準で切り替えます。")
          << monthResolution.statusMessage
          << QStringLiteral("日柱は前日 23:00 開始の日界で切り替えます。地方時補正は未実装です。")
          << QStringLiteral("時柱は日干と出生時刻から最小実装で計算しています。")
          << QStringLiteral("通変星は日干を基準に、年干・月干・時干の天干どうしのみ最小実装しています。")
          << QStringLiteral("蔵干は各支に対応する一般的な一覧のみ最小実装しています。")
          << QStringLiteral("五行分布は四柱天干・地支・蔵干を単純件数で最小集計しています。")
          << QStringLiteral("季節評価は月支ベースで春夏秋冬と日干適性を最小判定しています。")
          << QStringLiteral("強弱評価は五行分布と季節評価を使った暫定ラベルです。正式な身強身弱判定ではありません。")
          << QStringLiteral("寒暖・乾湿評価は月支ベースの調候前提情報を最小実装しています。")
          << QStringLiteral("用神候補は不足傾向などを使った断定しない暫定表示です。")
          << QStringLiteral("格局候補は月干通変星と月令参照を使った断定しない暫定表示です。")
          << QStringLiteral("起運年齢は正節との差分を 3 日 = 1 年、端数切り上げで換算します。")
          << QStringLiteral("大運一覧は月柱起点で、順行なら先へ、逆行なら前へ進めた干支列として順逆反映済みです。原命式との同干・同支・冲・害・破・刑・干合・三合・方合候補も参考表示します。")
          << QStringLiteral("大運の順逆は年干陰陽と性別の組み合わせによる一般ルールで実判定しています。")
          << QStringLiteral("節入り差情報には、出生日時と参照正節日時との差分、および起運年齢換算結果を保持します。")
          << QStringLiteral("流年一覧は出生年から並べた最小表示骨格で、原命式との同干・同支・冲・害・破・刑・干合・三合・方合候補を参考表示します。")
          << QStringLiteral("流年解釈の吉凶断定や流派固有判断は未実装です。");

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
