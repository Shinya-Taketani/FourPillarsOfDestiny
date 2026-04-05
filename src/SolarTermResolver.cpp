#include "SolarTermResolver.h"

#include <QDate>
#include <QDateTime>
#include <QJsonObject>
#include <QTimeZone>

namespace {
int monthOffsetFromTerm(const QString &termName)
{
    if (termName == QStringLiteral("立春")) {
        return 0;
    }

    if (termName == QStringLiteral("啓蟄")) {
        return 1;
    }

    return -1;
}

QString earthlyBranchForMonthOffset(int monthOffset)
{
    static const QStringList branches{
        QStringLiteral("寅"),
        QStringLiteral("卯"),
        QStringLiteral("辰"),
        QStringLiteral("巳"),
        QStringLiteral("午"),
        QStringLiteral("未"),
        QStringLiteral("申"),
        QStringLiteral("酉"),
        QStringLiteral("戌"),
        QStringLiteral("亥"),
        QStringLiteral("子"),
        QStringLiteral("丑")
    };

    if (monthOffset < 0 || monthOffset >= branches.size()) {
        return QString();
    }

    return branches.at(monthOffset);
}

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

    if (index < 0 || index >= stems.size()) {
        return QString();
    }

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

    if (index < 0 || index >= branches.size()) {
        return QString();
    }

    return branches.at(index);
}

int positiveModulo(int value, int divisor)
{
    const int remainder = value % divisor;
    return remainder < 0 ? remainder + divisor : remainder;
}

int firstMonthStemIndexForYearStem(const QString &yearStem)
{
    if (yearStem == QStringLiteral("甲") || yearStem == QStringLiteral("己")) {
        return 2;
    }

    if (yearStem == QStringLiteral("乙") || yearStem == QStringLiteral("庚")) {
        return 4;
    }

    if (yearStem == QStringLiteral("丙") || yearStem == QStringLiteral("辛")) {
        return 6;
    }

    if (yearStem == QStringLiteral("丁") || yearStem == QStringLiteral("壬")) {
        return 8;
    }

    if (yearStem == QStringLiteral("戊") || yearStem == QStringLiteral("癸")) {
        return 0;
    }

    return -1;
}

QString monthPillarFromYearStem(const QString &yearPillar, int monthOffset)
{
    if (yearPillar.isEmpty()) {
        return QString();
    }

    const QString yearStem = yearPillar.left(1);
    const int firstMonthStemIndex = firstMonthStemIndexForYearStem(yearStem);
    if (firstMonthStemIndex < 0) {
        return QString();
    }

    const QString monthBranch = earthlyBranchForMonthOffset(monthOffset);
    if (monthBranch.isEmpty()) {
        return QString();
    }

    return heavenlyStemAt((firstMonthStemIndex + monthOffset) % 10) + monthBranch;
}

QString yearPillarForGregorianYear(int year)
{
    const int offset = positiveModulo(year - 1984, 60);
    return heavenlyStemAt(offset % 10) + earthlyBranchAt(offset % 12);
}
}

SolarTermResolution SolarTermResolver::resolveMonthPillar(const BirthInfo &birthInfo, const QString &yearPillar) const
{
    const QDate birthDate = QDate::fromString(birthInfo.birthDate, QStringLiteral("yyyy-MM-dd"));
    if (!birthDate.isValid()) {
        return {
            false,
            false,
            QStringLiteral("月柱未計算"),
            QStringLiteral("生年月日が不正のため、節入り判定の前提が不足しています。")
        };
    }

    const SolarTermYearData yearData = m_dataSource.loadYearData(birthDate.year());
    if (!yearData.dataSourceAvailable) {
        return {
            false,
            false,
            QStringLiteral("月柱未実装"),
            QStringLiteral("節入りデータを参照できません。月柱本実装は保留しています。")
        };
    }

    if (!yearData.hasYearData) {
        return {
            false,
            false,
            QStringLiteral("月柱未対応"),
            QStringLiteral("節入りデータは外部 JSON 方式を採用していますが、指定年データが未整備です。")
        };
    }

    int matchedMonthOffset = -1;
    QString matchedTermName;
    const QDateTime birthDateTime(birthDate, QTime(0, 0), QTimeZone::systemTimeZone());

    for (const QJsonValue &entryValue : yearData.entries) {
        if (!entryValue.isObject()) {
            continue;
        }

        const QJsonObject entryObject = entryValue.toObject();
        const QString termName = entryObject.value(QStringLiteral("term")).toString();
        const QDateTime termDateTime = QDateTime::fromString(
            entryObject.value(QStringLiteral("at")).toString(),
            Qt::ISODate
        );

        if (!termDateTime.isValid()) {
            continue;
        }

        const int monthOffset = monthOffsetFromTerm(termName);
        if (monthOffset < 0) {
            continue;
        }

        if (termDateTime <= birthDateTime) {
            matchedMonthOffset = monthOffset;
            matchedTermName = termName;
        }
    }

    if (matchedMonthOffset < 0) {
        const SolarTermYearData previousYearData = m_dataSource.loadYearData(birthDate.year() - 1);
        if (!previousYearData.dataSourceAvailable || !previousYearData.hasYearData) {
            return {
                false,
                false,
                QStringLiteral("月柱未対応"),
                QStringLiteral("年初の節入り前ですが、前年データが未整備のため月柱計算に対応していません。")
            };
        }

        const QString previousYearPillar = yearPillarForGregorianYear(birthDate.year() - 1);
        const QString monthPillar = monthPillarFromYearStem(previousYearPillar, 11);
        if (monthPillar.isEmpty()) {
            return {
                false,
                false,
                QStringLiteral("月柱未対応"),
                QStringLiteral("前年基準の年初月柱計算に失敗しました。")
            };
        }

        return {
            true,
            true,
            monthPillar,
            QStringLiteral("年初の最初の節入り前のため、前年の最終月区間として暫定計算しました。")
        };
    }

    const QString monthPillar = monthPillarFromYearStem(yearPillar, matchedMonthOffset);
    if (monthPillar.isEmpty()) {
        return {
            false,
            false,
            QStringLiteral("月柱未対応"),
            QStringLiteral("年干から月干を導く限定実装に失敗しました。")
        };
    }

    return {
        true,
        true,
        monthPillar,
        QStringLiteral("節入りサンプルデータによる限定実装です。%1 以降の区間として月柱を計算しました。")
            .arg(matchedTermName)
    };
}
