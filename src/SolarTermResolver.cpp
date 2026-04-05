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

    // 節入り日時テーブルや天文計算は未実装。
    // 現段階では、月柱が節入り依存であることを明示するための受け口だけを提供する。
    return {
        false,
        false,
        QStringLiteral("月柱未実装"),
        QStringLiteral("節入り判定は未実装です。月柱本実装は今後ここへ差し込みます。")
    };
}
