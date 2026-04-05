#include "ChartCalculator.h"

#include <QDate>
#include <QTime>

namespace {
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
}

ChartResult ChartCalculator::calculate(const BirthInfo &birthInfo) const
{
    const QString yearPillar = calculateYearPillar(birthInfo);
    const QString monthPillar = calculateMonthPillar(birthInfo);
    const QString dayPillar = calculateDayPillar(birthInfo);
    const QString hourPillar = calculateHourPillar(birthInfo);
    const QString description = buildDescription(
        birthInfo,
        yearPillar,
        monthPillar,
        dayPillar,
        hourPillar
    );

    return {
        yearPillar,
        monthPillar,
        dayPillar,
        hourPillar,
        description
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

QString ChartCalculator::calculateMonthPillar(const BirthInfo &birthInfo) const
{
    const SolarTermResolution resolution = m_solarTermResolver.resolveMonthPillar(birthInfo);
    return resolution.monthPillar;
}

QString ChartCalculator::calculateDayPillar(const BirthInfo &birthInfo) const
{
    Q_UNUSED(birthInfo);

    // 日柱計算は今後の実装対象として保留する。
    return QStringLiteral("日柱未実装");
}

QString ChartCalculator::calculateHourPillar(const BirthInfo &birthInfo) const
{
    const QTime birthTime = QTime::fromString(birthInfo.birthTime, QStringLiteral("HH:mm"));
    if (!birthTime.isValid()) {
        return QStringLiteral("時柱未計算");
    }

    const int minutes = birthTime.hour() * 60 + birthTime.minute();
    const int branchIndex = positiveModulo(((minutes + 60) / 120), 12);

    // 日干未実装のため、時干はまだ確定できない。時支のみ返す。
    return QStringLiteral("?") + earthlyBranchAt(branchIndex);
}

QString ChartCalculator::buildDescription(
    const BirthInfo &birthInfo,
    const QString &yearPillar,
    const QString &monthPillar,
    const QString &dayPillar,
    const QString &hourPillar
) const
{
    QStringList lines;
    const SolarTermResolution monthResolution = m_solarTermResolver.resolveMonthPillar(birthInfo);

    lines << QStringLiteral("一般四柱推命の共通計算基盤の途中実装です。")
          << QStringLiteral("年柱は暦年ベースで計算しています。立春基準の年切り替えは未対応です。")
          << QStringLiteral("月柱は節入り判定責務を分離済みです。")
          << monthResolution.statusMessage
          << QStringLiteral("日柱は未実装です。")
          << QStringLiteral("時柱は出生時刻から時支のみ計算しています。時干は未実装です。");

    if (!birthInfo.birthDate.isEmpty() || !birthInfo.birthTime.isEmpty() || !birthInfo.gender.isEmpty()) {
        lines << QStringLiteral("入力値: %1 / %2 / %3")
                     .arg(birthInfo.birthDate.isEmpty() ? QStringLiteral("未入力") : birthInfo.birthDate)
                     .arg(birthInfo.birthTime.isEmpty() ? QStringLiteral("未入力") : birthInfo.birthTime)
                     .arg(birthInfo.gender.isEmpty() ? QStringLiteral("未入力") : birthInfo.gender);
    }

    lines << QStringLiteral("計算結果: %1 / %2 / %3 / %4")
                 .arg(yearPillar)
                 .arg(monthPillar)
                 .arg(dayPillar)
                 .arg(hourPillar);

    return lines.join(QLatin1Char('\n'));
}
