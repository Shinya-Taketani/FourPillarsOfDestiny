#include "InterpretationEngine.h"

InterpretationResult InterpretationEngine::interpret(const ChartResult &chartResult) const
{
    QString detailText = QStringLiteral(
        "ChartResult に基づく本実装の解釈ロジックは未対応です。"
    );

    if (!chartResult.yearPillar.isEmpty() || !chartResult.monthPillar.isEmpty()
        || !chartResult.dayPillar.isEmpty() || !chartResult.hourPillar.isEmpty()) {
        detailText += QStringLiteral(" 現在の仮表示対象: %1 / %2 / %3 / %4")
                          .arg(chartResult.yearPillar.isEmpty() ? QStringLiteral("-") : chartResult.yearPillar)
                          .arg(chartResult.monthPillar.isEmpty() ? QStringLiteral("-") : chartResult.monthPillar)
                          .arg(chartResult.dayPillar.isEmpty() ? QStringLiteral("-") : chartResult.dayPillar)
                          .arg(chartResult.hourPillar.isEmpty() ? QStringLiteral("-") : chartResult.hourPillar);
    }

    return {
        QStringLiteral("これは仮の解釈結果です。"),
        detailText,
        QStringLiteral("泰山流固有ルールは未実装であり、要確認事項があります。")
    };
}
