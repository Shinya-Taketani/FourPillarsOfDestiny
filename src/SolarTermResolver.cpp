#include "SolarTermResolver.h"

#include <cstdlib>
#include <limits>
#include <QDate>
#include <QDateTime>
#include <QTimeZone>

namespace {
struct SolarTermMoment
{
    QString termName;
    QDateTime at;
};

QString dataQualityLabel(const SolarTermYearData &yearData)
{
    if (yearData.adoptable == QStringLiteral("verified")) {
        return QStringLiteral("verified（公的または採用済みデータ）");
    }

    if (yearData.adoptable == QStringLiteral("provisional")) {
        if (yearData.yearStatus == QStringLiteral("legacy_sample")) {
            return QStringLiteral("provisional（既存サンプルの legacy fallback）");
        }

        return QStringLiteral("provisional（暫定データ）");
    }

    return QStringLiteral("未対応");
}

enum class SolarTermReferenceMode
{
    Nearest = 0,
    Previous = 1,
    Next = 2
};

bool isPrincipalTermName(const QString &termName)
{
    static const QStringList principalTerms{
        QStringLiteral("小寒"),
        QStringLiteral("立春"),
        QStringLiteral("啓蟄"),
        QStringLiteral("清明"),
        QStringLiteral("立夏"),
        QStringLiteral("芒種"),
        QStringLiteral("小暑"),
        QStringLiteral("立秋"),
        QStringLiteral("白露"),
        QStringLiteral("寒露"),
        QStringLiteral("立冬"),
        QStringLiteral("大雪")
    };

    return principalTerms.contains(termName);
}

int monthOffsetFromTerm(const QString &termName)
{
    if (termName == QStringLiteral("小寒")) {
        return 11;
    }

    if (termName == QStringLiteral("立春")) {
        return 0;
    }

    if (termName == QStringLiteral("啓蟄")) {
        return 1;
    }

    if (termName == QStringLiteral("清明")) {
        return 2;
    }

    if (termName == QStringLiteral("立夏")) {
        return 3;
    }

    if (termName == QStringLiteral("芒種")) {
        return 4;
    }

    if (termName == QStringLiteral("小暑")) {
        return 5;
    }

    if (termName == QStringLiteral("立秋")) {
        return 6;
    }

    if (termName == QStringLiteral("白露")) {
        return 7;
    }

    if (termName == QStringLiteral("寒露")) {
        return 8;
    }

    if (termName == QStringLiteral("立冬")) {
        return 9;
    }

    if (termName == QStringLiteral("大雪")) {
        return 10;
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

QList<SolarTermMoment> solarTermMomentsFromYearData(const SolarTermYearData &yearData)
{
    QList<SolarTermMoment> moments;
    for (const SolarTermEntry &entry : yearData.entries) {
        if (entry.termName.isEmpty() || !entry.atDateTime.isValid() || !isPrincipalTermName(entry.termName)) {
            continue;
        }

        moments.append({entry.termName, entry.atDateTime});
    }

    return moments;
}

QDateTime birthDateTimeForComparison(const BirthInfo &birthInfo, const QList<SolarTermMoment> &moments)
{
    const QDate birthDate = QDate::fromString(birthInfo.birthDate, QStringLiteral("yyyy-MM-dd"));
    if (!birthDate.isValid()) {
        return {};
    }

    QTime birthTime = QTime::fromString(birthInfo.birthTime, QStringLiteral("HH:mm"));
    if (!birthTime.isValid()) {
        birthTime = QTime(0, 0);
    }

    if (!moments.isEmpty()) {
        return QDateTime(birthDate, birthTime, moments.first().at.timeZone());
    }

    return QDateTime(birthDate, birthTime, QTimeZone::systemTimeZone());
}

SolarTermDifferenceResolution resolveSolarTermDifference(
    const BirthInfo &birthInfo,
    const SolarTermDataSource &dataSource,
    SolarTermReferenceMode mode
)
{
    const QDate birthDate = QDate::fromString(birthInfo.birthDate, QStringLiteral("yyyy-MM-dd"));
    if (!birthDate.isValid()) {
        return {
            false,
            false,
            QString(),
            QStringLiteral("未対応"),
            QStringLiteral("未対応"),
            QString(),
            0,
            0,
            QStringLiteral("生年月日が不正のため、節入り差の前処理に対応していません。")
        };
    }

    QList<SolarTermMoment> candidates;
    QStringList missingYears;

    for (int year = birthDate.year() - 1; year <= birthDate.year() + 1; ++year) {
        const SolarTermYearData yearData = dataSource.loadYearData(year);
        if (!yearData.dataSourceAvailable) {
            return {
                false,
                false,
                QString(),
                QStringLiteral("未対応"),
                QStringLiteral("未対応"),
                QString(),
                0,
                0,
                QStringLiteral("節入りデータを参照できないため、節入り差の前処理に対応していません。")
            };
        }

        if (!yearData.hasYearData) {
            missingYears << QString::number(year);
            continue;
        }

        candidates.append(solarTermMomentsFromYearData(yearData));
    }

    if (candidates.isEmpty()) {
        return {
            false,
            false,
            QString(),
            QStringLiteral("未対応"),
            QStringLiteral("未対応"),
            QString(),
            0,
            0,
            QStringLiteral("節入り時刻データが未整備のため、節入り差の前処理に対応していません。")
        };
    }

    const QDateTime birthDateTime = birthDateTimeForComparison(birthInfo, candidates);
    if (!birthDateTime.isValid()) {
        return {
            false,
            false,
            QString(),
            QStringLiteral("未対応"),
            QStringLiteral("未対応"),
            QString(),
            0,
            0,
            QStringLiteral("出生日時を組み立てられないため、節入り差の前処理に対応していません。")
        };
    }

    int selectedIndex = -1;
    qint64 selectedAbsoluteDifference = std::numeric_limits<qint64>::max();

    for (int index = 0; index < candidates.size(); ++index) {
        const qint64 differenceMinutes = candidates.at(index).at.secsTo(birthDateTime) / 60;
        const qint64 absoluteDifference = std::llabs(differenceMinutes);

        if (mode == SolarTermReferenceMode::Nearest) {
            if (absoluteDifference < selectedAbsoluteDifference) {
                selectedAbsoluteDifference = absoluteDifference;
                selectedIndex = index;
            }
            continue;
        }

        if (mode == SolarTermReferenceMode::Previous && differenceMinutes >= 0) {
            if (selectedIndex < 0 || candidates.at(selectedIndex).at < candidates.at(index).at) {
                selectedAbsoluteDifference = absoluteDifference;
                selectedIndex = index;
            }
            continue;
        }

        if (mode == SolarTermReferenceMode::Next && differenceMinutes < 0) {
            if (selectedIndex < 0 || candidates.at(selectedIndex).at > candidates.at(index).at) {
                selectedAbsoluteDifference = absoluteDifference;
                selectedIndex = index;
            }
        }
    }

    if (selectedIndex < 0) {
        QString statusMessage = QStringLiteral("参照対象となる節入り日時を特定できませんでした。");
        if (mode == SolarTermReferenceMode::Previous) {
            statusMessage = QStringLiteral("出生日時より前の節入り日時を特定できませんでした。");
        } else if (mode == SolarTermReferenceMode::Next) {
            statusMessage = QStringLiteral("出生日時より後の節入り日時を特定できませんでした。");
        }

        return {
            false,
            false,
            birthDateTime.toString(Qt::ISODate),
            QStringLiteral("未対応"),
            QStringLiteral("未対応"),
            QString(),
            0,
            0,
            statusMessage
        };
    }

    const SolarTermMoment selectedMoment = candidates.at(selectedIndex);
    const qint64 differenceMinutes = selectedMoment.at.secsTo(birthDateTime) / 60;
    const qint64 maximumSupportedDifferenceMinutes = 60LL * 24LL * 60LL;
    if (selectedAbsoluteDifference > maximumSupportedDifferenceMinutes) {
        return {
            true,
            false,
            birthDateTime.toString(Qt::ISODate),
            QStringLiteral("未対応"),
            QStringLiteral("未対応"),
            QString(),
            0,
            0,
            QStringLiteral("近傍の節入り日時データが不足しているため、節入り差の参考値は未対応です。")
        };
    }

    const QString direction = differenceMinutes >= 0
        ? QStringLiteral("直前節入り")
        : QStringLiteral("直後節入り");

    QString statusMessage = QStringLiteral(
        "一般四柱推命の共通基盤として、正節の節入り日時との差分を保持する採用仕様です。"
    );
    if (!missingYears.isEmpty()) {
        statusMessage += QStringLiteral(" 近傍年の一部データが未整備です: %1。")
                             .arg(missingYears.join(QStringLiteral("、")));
    }
    if (!QTime::fromString(birthInfo.birthTime, QStringLiteral("HH:mm")).isValid()) {
        statusMessage += QStringLiteral(" 出生時刻が不正または未入力のため 00:00 を仮採用しています。");
    }

    return {
        true,
        true,
        birthDateTime.toString(Qt::ISODate),
        selectedMoment.termName,
        direction,
        selectedMoment.at.toString(Qt::ISODate),
        differenceMinutes,
        selectedAbsoluteDifference,
        statusMessage
    };
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
            QStringLiteral("生年月日が不正のため、節入り判定の前提が不足しています。"),
            QStringLiteral("未対応"),
            QStringLiteral("未対応"),
            QString()
        };
    }

    const SolarTermYearData yearData = m_dataSource.loadYearData(birthDate.year());
    if (!yearData.dataSourceAvailable) {
        return {
            false,
            false,
            QStringLiteral("月柱未対応"),
            QStringLiteral("正節データを参照できないため、月柱を確定できません。"),
            QStringLiteral("未対応"),
            QStringLiteral("未対応"),
            QString()
        };
    }

    if (!yearData.hasYearData) {
        return {
            false,
            false,
            QStringLiteral("月柱未対応"),
            QStringLiteral("指定年の正節データが未整備のため、月柱を確定できません。"),
            yearData.sourceQuality,
            yearData.adoptable,
            yearData.notes
        };
    }

    int matchedMonthOffset = -1;
    QString matchedTermName;
    const QDateTime birthDateTime = birthDateTimeForComparison(birthInfo, solarTermMomentsFromYearData(yearData));
    if (!birthDateTime.isValid()) {
        return {
            false,
            false,
            QStringLiteral("月柱未計算"),
            QStringLiteral("出生日時を組み立てられないため、月柱を確定できません。"),
            yearData.sourceQuality,
            yearData.adoptable,
            yearData.notes
        };
    }

    for (const SolarTermEntry &entry : yearData.entries) {
        const int monthOffset = monthOffsetFromTerm(entry.termName);
        if (monthOffset < 0) {
            continue;
        }

        if (entry.atDateTime <= birthDateTime) {
            matchedMonthOffset = monthOffset;
            matchedTermName = entry.termName;
        }
    }

    if (matchedMonthOffset < 0) {
        const SolarTermYearData previousYearData = m_dataSource.loadYearData(birthDate.year() - 1);
        if (!previousYearData.dataSourceAvailable || !previousYearData.hasYearData) {
            return {
                false,
                false,
                QStringLiteral("月柱未対応"),
                QStringLiteral("年初の最初の正節前ですが、前年の正節データが未整備のため月柱を確定できません。"),
                previousYearData.sourceQuality,
                previousYearData.adoptable,
                previousYearData.notes
            };
        }

        const QString previousYearPillar = yearPillarForGregorianYear(birthDate.year() - 1);
        const QString monthPillar = monthPillarFromYearStem(previousYearPillar, 11);
        if (monthPillar.isEmpty()) {
            return {
                false,
                false,
                QStringLiteral("月柱未対応"),
                QStringLiteral("前年基準の丑月計算に失敗しました。"),
                previousYearData.sourceQuality,
                previousYearData.adoptable,
                previousYearData.notes
            };
        }

        return {
            true,
            true,
            monthPillar,
            QStringLiteral(
                "本アプリ採用仕様として、正節の節入り日時比較により前年の丑月区間と判定しました。 使用データ品質: %1。"
            ).arg(dataQualityLabel(previousYearData)),
            previousYearData.sourceQuality,
            previousYearData.adoptable,
            previousYearData.notes
        };
    }

    const QString monthPillar = monthPillarFromYearStem(yearPillar, matchedMonthOffset);
    if (monthPillar.isEmpty()) {
        return {
            false,
            false,
            QStringLiteral("月柱未対応"),
            QStringLiteral("年干から月干を導く一般ルール計算に失敗しました。"),
            yearData.sourceQuality,
            yearData.adoptable,
            yearData.notes
        };
    }

    return {
        true,
        true,
        monthPillar,
        QStringLiteral(
            "本アプリ採用仕様として、正節 %1 の節入り日時以後の区間から月柱を確定しました。 使用データ品質: %2。"
        ).arg(matchedTermName, dataQualityLabel(yearData)),
        yearData.sourceQuality,
        yearData.adoptable,
        yearData.notes
    };
}

SolarTermDifferenceResolution SolarTermResolver::resolveNearestSolarTermDifference(const BirthInfo &birthInfo) const
{
    return resolveSolarTermDifference(
        birthInfo,
        m_dataSource,
        SolarTermReferenceMode::Nearest
    );
}

SolarTermDifferenceResolution SolarTermResolver::resolvePreviousSolarTermDifference(const BirthInfo &birthInfo) const
{
    return resolveSolarTermDifference(
        birthInfo,
        m_dataSource,
        SolarTermReferenceMode::Previous
    );
}

SolarTermDifferenceResolution SolarTermResolver::resolveNextSolarTermDifference(const BirthInfo &birthInfo) const
{
    return resolveSolarTermDifference(
        birthInfo,
        m_dataSource,
        SolarTermReferenceMode::Next
    );
}
