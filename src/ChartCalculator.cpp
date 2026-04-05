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
        hiddenStems
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
          << QStringLiteral("蔵干は各支に対応する一般的な一覧のみ最小実装しています。");

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
