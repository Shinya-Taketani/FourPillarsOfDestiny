#include "InterpretationEngine.h"

#include <QStringList>

namespace {
QString firstCandidate(const QVariantMap &map)
{
    return map.value(QStringLiteral("candidates")).toStringList().value(0);
}

QString firstRelationSummary(const QVariantList &fortunes)
{
    for (const QVariant &fortuneValue : fortunes) {
        const QVariantMap fortune = fortuneValue.toMap();
        const QString relationSummary = fortune.value(QStringLiteral("relationSummary")).toString();
        if (!relationSummary.isEmpty()) {
            return relationSummary;
        }
    }

    return QString();
}

QString firstFortunePillar(const QVariantList &fortunes)
{
    return fortunes.value(0).toMap().value(QStringLiteral("pillar")).toString();
}

QString strengthSummaryText(const QVariantMap &strengthEvaluation)
{
    const QString label = strengthEvaluation.value(QStringLiteral("label")).toString();
    const QString reason = strengthEvaluation.value(QStringLiteral("reason")).toString();
    if (label.isEmpty() && reason.isEmpty()) {
        return QString();
    }

    if (reason.isEmpty()) {
        return QStringLiteral("日主の強弱は %1 とみています。").arg(label);
    }

    return QStringLiteral("日主の強弱は %1 で、%2").arg(label, reason);
}

QString usefulGodSummaryText(const QVariantMap &usefulGodCandidates)
{
    const QString firstUsefulGod = firstCandidate(usefulGodCandidates);
    const QString reason = usefulGodCandidates.value(QStringLiteral("reason")).toString();
    if (firstUsefulGod.isEmpty() && reason.isEmpty()) {
        return QString();
    }

    if (reason.isEmpty()) {
        return QStringLiteral("用神候補は %1 を先頭に参考表示しています。").arg(firstUsefulGod);
    }

    return QStringLiteral("用神候補は %1 を先頭に参考表示しており、%2").arg(firstUsefulGod, reason);
}

QString patternSummaryText(const QVariantMap &patternCandidates)
{
    const QString firstPattern = firstCandidate(patternCandidates);
    const QString reason = patternCandidates.value(QStringLiteral("reason")).toString();
    if (firstPattern.isEmpty() && reason.isEmpty()) {
        return QString();
    }

    if (reason.isEmpty()) {
        return QStringLiteral("格局候補は %1 が第一候補です。").arg(firstPattern);
    }

    return QStringLiteral("格局候補は %1 が第一候補で、%2").arg(firstPattern, reason);
}

QString climateSummaryText(
    const QVariantMap &seasonalEvaluation,
    const QVariantMap &climateEvaluation
)
{
    const QString season = seasonalEvaluation.value(QStringLiteral("season")).toString();
    const QString suitability = seasonalEvaluation.value(QStringLiteral("suitability")).toString();
    const QString temperature = climateEvaluation.value(QStringLiteral("temperature")).toString();
    const QString moisture = climateEvaluation.value(QStringLiteral("moisture")).toString();

    QStringList parts;
    if (!season.isEmpty() || !suitability.isEmpty()) {
        parts << QStringLiteral("季節評価は %1 / %2 です。").arg(
            season.isEmpty() ? QStringLiteral("未対応") : season,
            suitability.isEmpty() ? QStringLiteral("未対応") : suitability
        );
    }
    if (!temperature.isEmpty() || !moisture.isEmpty()) {
        parts << QStringLiteral("寒暖乾湿は %1・%2 とみています。").arg(
            temperature.isEmpty() ? QStringLiteral("未対応") : temperature,
            moisture.isEmpty() ? QStringLiteral("未対応") : moisture
        );
    }

    return parts.join(QStringLiteral(" "));
}

QString fortuneSummaryText(const QVariantList &majorFortunes, const QVariantList &annualFortunes)
{
    const QString firstMajorFortune = firstFortunePillar(majorFortunes);
    const QString firstAnnualFortune = firstFortunePillar(annualFortunes);
    if (firstMajorFortune.isEmpty() && firstAnnualFortune.isEmpty()) {
        return QString();
    }

    QStringList parts;
    if (!firstMajorFortune.isEmpty()) {
        parts << QStringLiteral("先頭大運は %1 です。").arg(firstMajorFortune);
    }
    if (!firstAnnualFortune.isEmpty()) {
        parts << QStringLiteral("先頭流年は %1 です。").arg(firstAnnualFortune);
    }

    return parts.join(QStringLiteral(" "));
}

QString joinedNonEmpty(const QStringList &parts)
{
    QStringList filtered;
    for (const QString &part : parts) {
        if (!part.trimmed().isEmpty()) {
            filtered << part.trimmed();
        }
    }
    return filtered.join(QStringLiteral(" "));
}
}

InterpretationResult InterpretationEngine::interpret(const ChartResult &chartResult) const
{
    const QString strengthLine = strengthSummaryText(chartResult.strengthEvaluation);
    const QString usefulGodLine = usefulGodSummaryText(chartResult.usefulGodCandidates);
    const QString patternLine = patternSummaryText(chartResult.patternCandidates);
    const QString climateLine = climateSummaryText(
        chartResult.seasonalEvaluation,
        chartResult.climateEvaluation
    );
    const QString fortuneLine = fortuneSummaryText(
        chartResult.majorFortunes,
        chartResult.annualFortunes
    );

    QString summaryText = joinedNonEmpty({
        strengthLine,
        usefulGodLine,
        patternLine,
        climateLine,
        fortuneLine
    });
    if (summaryText.isEmpty()) {
        summaryText = QStringLiteral("命式情報が不足しているため、解釈要約は未対応です。");
    }

    const QString majorRelationSummary = firstRelationSummary(chartResult.majorFortunes);
    const QString annualRelationSummary = firstRelationSummary(chartResult.annualFortunes);

    QStringList detailLines;
    detailLines << QStringLiteral("強弱評価: %1").arg(
        chartResult.strengthEvaluation.value(QStringLiteral("label")).toString().isEmpty()
            ? QStringLiteral("未対応")
            : chartResult.strengthEvaluation.value(QStringLiteral("label")).toString()
    );
    if (!chartResult.strengthEvaluation.value(QStringLiteral("reason")).toString().isEmpty()) {
        detailLines << QStringLiteral("  根拠: %1").arg(
            chartResult.strengthEvaluation.value(QStringLiteral("reason")).toString()
        );
    }

    detailLines << QStringLiteral("用神候補: %1").arg(
        chartResult.usefulGodCandidates.value(QStringLiteral("candidates")).toStringList().join(QStringLiteral(" / "))
    );
    if (!chartResult.usefulGodCandidates.value(QStringLiteral("reason")).toString().isEmpty()) {
        detailLines << QStringLiteral("  補足: %1").arg(
            chartResult.usefulGodCandidates.value(QStringLiteral("reason")).toString()
        );
    }

    detailLines << QStringLiteral("格局候補: %1").arg(
        chartResult.patternCandidates.value(QStringLiteral("candidates")).toStringList().join(QStringLiteral(" / "))
    );
    if (!chartResult.patternCandidates.value(QStringLiteral("reason")).toString().isEmpty()) {
        detailLines << QStringLiteral("  補足: %1").arg(
            chartResult.patternCandidates.value(QStringLiteral("reason")).toString()
        );
    }

    detailLines << QStringLiteral("関係判定: 大運 %1 / 流年 %2").arg(
        majorRelationSummary.isEmpty() ? QStringLiteral("未対応") : majorRelationSummary,
        annualRelationSummary.isEmpty() ? QStringLiteral("未対応") : annualRelationSummary
    );

    detailLines << QStringLiteral("季節・調候: %1 / %2 / %3 / %4").arg(
        chartResult.seasonalEvaluation.value(QStringLiteral("season")).toString().isEmpty()
            ? QStringLiteral("未対応")
            : chartResult.seasonalEvaluation.value(QStringLiteral("season")).toString(),
        chartResult.seasonalEvaluation.value(QStringLiteral("suitability")).toString().isEmpty()
            ? QStringLiteral("未対応")
            : chartResult.seasonalEvaluation.value(QStringLiteral("suitability")).toString(),
        chartResult.climateEvaluation.value(QStringLiteral("temperature")).toString().isEmpty()
            ? QStringLiteral("未対応")
            : chartResult.climateEvaluation.value(QStringLiteral("temperature")).toString(),
        chartResult.climateEvaluation.value(QStringLiteral("moisture")).toString().isEmpty()
            ? QStringLiteral("未対応")
            : chartResult.climateEvaluation.value(QStringLiteral("moisture")).toString()
    );

    const QString detailText = detailLines.join(QStringLiteral("\n"));

    QStringList cautionParts;
    cautionParts << QStringLiteral("この解釈は暫定候補と参考表示に基づく要約です。");
    if (chartResult.monthPillarStatusMessage.contains(QStringLiteral("provisional"))
        || chartResult.description.contains(QStringLiteral("provisional"))) {
        cautionParts << QStringLiteral("節入りデータに provisional または暫定データが含まれる可能性があります。");
    }
    cautionParts << QStringLiteral("泰山流固有ルールや吉凶断定は未確定部分が残っています。");

    return {
        summaryText,
        detailText,
        cautionParts.join(QStringLiteral(" " ))
    };
}
