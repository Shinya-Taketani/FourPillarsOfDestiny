#include "ChartCalculator.h"

ChartResult ChartCalculator::calculate(const BirthInfo &birthInfo) const
{
    QString description = QStringLiteral(
        "これは仮の命式結果です。ChartCalculator の本実装は未対応です。"
    );

    if (!birthInfo.birthDate.isEmpty() || !birthInfo.birthTime.isEmpty() || !birthInfo.gender.isEmpty()) {
        description += QStringLiteral(" 入力値: %1 / %2 / %3")
                           .arg(birthInfo.birthDate.isEmpty() ? QStringLiteral("未入力") : birthInfo.birthDate)
                           .arg(birthInfo.birthTime.isEmpty() ? QStringLiteral("未入力") : birthInfo.birthTime)
                           .arg(birthInfo.gender.isEmpty() ? QStringLiteral("未入力") : birthInfo.gender);
    }

    return {
        QStringLiteral("甲子"),
        QStringLiteral("乙丑"),
        QStringLiteral("丙寅"),
        QStringLiteral("丁卯"),
        description
    };
}
