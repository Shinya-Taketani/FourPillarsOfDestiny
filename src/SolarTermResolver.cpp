#include "SolarTermResolver.h"

#include <QDate>

SolarTermResolution SolarTermResolver::resolveMonthPillar(const BirthInfo &birthInfo) const
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
            QStringLiteral("月柱未実装"),
            QStringLiteral("節入りデータは外部 JSON 方式を採用していますが、指定年データが未整備です。")
        };
    }

    // 節入りデータの保持方式は導入済みだが、節入り日時の利用ロジックはまだ未実装。
    return {
        false,
        false,
        QStringLiteral("月柱未実装"),
        QStringLiteral("節入りデータは読み込み可能ですが、月柱本実装は未対応です。")
    };
}
