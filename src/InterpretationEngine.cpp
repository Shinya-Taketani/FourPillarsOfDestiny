#include "InterpretationEngine.h"

#include <QMap>
#include <QStringList>

namespace {
struct TenGodMeaning
{
    QString basicMeaning;
    QString positiveExpression;
    QString cautionExpression;
    QString workTheme;
    QString relationshipTheme;
};

QString compactStatusLine(const QString &label, const QString &statusMessage)
{
    if (statusMessage.trimmed().isEmpty()) {
        return QString();
    }

    return QStringLiteral("%1: %2").arg(label, statusMessage.trimmed());
}

const QMap<QString, TenGodMeaning> &tenGodMeanings()
{
    static const QMap<QString, TenGodMeaning> meanings{
        {
            QStringLiteral("比肩"),
            {
                QStringLiteral("自我・自立・同類支援を示す傾向があります。"),
                QStringLiteral("主体性や継続力、自分の足で進む力として活きやすい星です。"),
                QStringLiteral("頑固さや孤立、譲らなさとして出やすい面には注意が必要です。"),
                QStringLiteral("独立業や裁量の大きい仕事で力を出しやすい傾向があります。"),
                QStringLiteral("対等性を重んじ、相手にも同じ基準を求めやすい傾向があります。")
            }
        },
        {
            QStringLiteral("劫財"),
            {
                QStringLiteral("競争・奪取・同類援助を示す傾向があります。"),
                QStringLiteral("突破力や開拓力、信念の強さとして活きやすい星です。"),
                QStringLiteral("極端さややり過ぎ、金銭面の荒さとして出やすい面があります。"),
                QStringLiteral("新規開拓や独自路線、危機突破の場面で力を出しやすい傾向があります。"),
                QStringLiteral("率直で裏表が少ない反面、押しの強さとして出やすいことがあります。")
            }
        },
        {
            QStringLiteral("食神"),
            {
                QStringLiteral("表現・生産・寛和を示す傾向があります。"),
                QStringLiteral("おおらかさや表現力、生産性として活きやすい星です。"),
                QStringLiteral("気の緩みや享楽、甘さとして出やすい面には注意が必要です。"),
                QStringLiteral("技術・制作・専門職など、自分の腕を形にする仕事と相性を取りやすい傾向があります。"),
                QStringLiteral("温和で対立を避けやすい反面、受け身に流れやすいことがあります。")
            }
        },
        {
            QStringLiteral("傷官"),
            {
                QStringLiteral("鋭い表現・批評・改革性を示す傾向があります。"),
                QStringLiteral("才気や分析力、競争力として活きやすい星です。"),
                QStringLiteral("反権威や言い過ぎ、過激さとして出やすい面には注意が必要です。"),
                QStringLiteral("技能職、分析、戦略、専門性の高い分野で鋭さを発揮しやすい傾向があります。"),
                QStringLiteral("本音を隠しにくく、批評性や完璧主義として表れやすいことがあります。")
            }
        },
        {
            QStringLiteral("偏財"),
            {
                QStringLiteral("外向きの財・機会・交際を示す傾向があります。"),
                QStringLiteral("柔軟さや社交性、現実感覚として活きやすい星です。"),
                QStringLiteral("散漫さや浪費、欲望過多として出やすい面があります。"),
                QStringLiteral("営業・流通・商取引・人脈活用の場面で働きやすい傾向があります。"),
                QStringLiteral("交流が広く軽快に動ける反面、落ち着かなさとして出ることがあります。")
            }
        },
        {
            QStringLiteral("正財"),
            {
                QStringLiteral("堅実な財・管理・安定を示す傾向があります。"),
                QStringLiteral("丁寧さや安定運用、誠実さとして活きやすい星です。"),
                QStringLiteral("形式化や視野の狭さ、過度な節約として出やすい面があります。"),
                QStringLiteral("会計・管理・継続業務のような積み重ね型の仕事と相性を取りやすい傾向があります。"),
                QStringLiteral("安定志向で信頼を築きやすい反面、融通が利きにくくなることがあります。")
            }
        },
        {
            QStringLiteral("偏官"),
            {
                QStringLiteral("圧力・統率・制御・危機対応を示す傾向があります。"),
                QStringLiteral("決断力や実行力、統率力として活きやすい星です。"),
                QStringLiteral("強圧さや過緊張、攻撃性として出やすい面には注意が必要です。"),
                QStringLiteral("統率、現場管理、危機対応のように責任を引き受ける場面で働きやすい傾向があります。"),
                QStringLiteral("面倒見の良さとして出る一方、支配的に見えやすいことがあります。")
            }
        },
        {
            QStringLiteral("正官"),
            {
                QStringLiteral("秩序・規範・管理・信用を示す傾向があります。"),
                QStringLiteral("規範意識や信用、品格として活きやすい星です。"),
                QStringLiteral("硬直や保身、萎縮として出やすい面には注意が必要です。"),
                QStringLiteral("組織運営、法務、管理のような秩序を扱う仕事と相性を取りやすい傾向があります。"),
                QStringLiteral("礼節や責任感が出やすい反面、形式を守りすぎて窮屈になることがあります。")
            }
        },
        {
            QStringLiteral("偏印"),
            {
                QStringLiteral("独創・要領・特殊技能を示す傾向があります。"),
                QStringLiteral("発想力や要領、独創性として活きやすい星です。"),
                QStringLiteral("不安定さや散りやすさ、継続難として出やすい面があります。"),
                QStringLiteral("企画、芸術、特殊技能、変則的な働き方で持ち味が出やすい傾向があります。"),
                QStringLiteral("自分流を大切にしやすい反面、周囲との歩調がずれやすいことがあります。")
            }
        },
        {
            QStringLiteral("印綬"),
            {
                QStringLiteral("学習・保護・文化・支援を示す傾向があります。"),
                QStringLiteral("学習力や支援受容、文化性として活きやすい星です。"),
                QStringLiteral("依存や安逸、受け身として出やすい面には注意が必要です。"),
                QStringLiteral("教育、研究、助言、文化芸術のような蓄積型の仕事と相性を取りやすい傾向があります。"),
                QStringLiteral("援助縁や受容性として出やすい反面、守られ過ぎると停滞しやすいことがあります。")
            }
        }
    };

    return meanings;
}

QString representativeTenGod(const QVariantMap &tenGods)
{
    const QString monthTenGod = tenGods.value(QStringLiteral("monthPillar")).toString();
    if (!monthTenGod.isEmpty() && monthTenGod != QStringLiteral("未対応")
        && monthTenGod != QStringLiteral("未実装") && monthTenGod != QStringLiteral("日主")) {
        return monthTenGod;
    }

    const QString yearTenGod = tenGods.value(QStringLiteral("yearPillar")).toString();
    if (!yearTenGod.isEmpty() && yearTenGod != QStringLiteral("未対応")
        && yearTenGod != QStringLiteral("未実装") && yearTenGod != QStringLiteral("日主")) {
        return yearTenGod;
    }

    const QString hourTenGod = tenGods.value(QStringLiteral("hourPillar")).toString();
    if (!hourTenGod.isEmpty() && hourTenGod != QStringLiteral("未対応")
        && hourTenGod != QStringLiteral("未実装") && hourTenGod != QStringLiteral("日主")) {
        return hourTenGod;
    }

    for (auto it = tenGods.constBegin(); it != tenGods.constEnd(); ++it) {
        const QString value = it.value().toString();
        if (!value.isEmpty() && value != QStringLiteral("未対応")
            && value != QStringLiteral("未実装") && value != QStringLiteral("日主")) {
            return value;
        }
    }

    return QString();
}

const TenGodMeaning *meaningForTenGod(const QString &tenGod)
{
    const auto &meanings = tenGodMeanings();
    const auto it = meanings.constFind(tenGod);
    if (it == meanings.constEnd()) {
        return nullptr;
    }

    return &it.value();
}

bool hasProvisionalHint(const QStringList &messages)
{
    for (const QString &message : messages) {
        if (message.contains(QStringLiteral("provisional"))
            || message.contains(QStringLiteral("暫定"))
            || message.contains(QStringLiteral("参考表示"))) {
            return true;
        }
    }

    return false;
}

bool hasAdoptedSpecHint(const QStringList &messages)
{
    for (const QString &message : messages) {
        if (message.contains(QStringLiteral("採用仕様"))
            || message.contains(QStringLiteral("正節"))
            || message.contains(QStringLiteral("立春"))) {
            return true;
        }
    }

    return false;
}

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

QString tenGodSummaryText(const QVariantMap &tenGods)
{
    const QString representative = representativeTenGod(tenGods);
    const TenGodMeaning *meaning = meaningForTenGod(representative);
    if (representative.isEmpty() || meaning == nullptr) {
        return QString();
    }

    return QStringLiteral("%1は、%2").arg(representative, meaning->basicMeaning);
}

QStringList tenGodDetailLines(const QVariantMap &tenGods)
{
    const QString representative = representativeTenGod(tenGods);
    const TenGodMeaning *meaning = meaningForTenGod(representative);
    if (representative.isEmpty() || meaning == nullptr) {
        return {};
    }

    return {
        QStringLiteral("通変星解釈: %1").arg(representative),
        QStringLiteral("  基本意味: %1").arg(meaning->basicMeaning),
        QStringLiteral("  長所: %1").arg(meaning->positiveExpression),
        QStringLiteral("  注意点: %1").arg(meaning->cautionExpression),
        QStringLiteral("  仕事面: %1").arg(meaning->workTheme),
        QStringLiteral("  対人面: %1").arg(meaning->relationshipTheme)
    };
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
    const QStringList statusMessages{
        chartResult.monthPillarStatusMessage,
        chartResult.fiveElementDistributionStatusMessage,
        chartResult.seasonalEvaluationStatusMessage,
        chartResult.strengthEvaluationStatusMessage,
        chartResult.climateEvaluationStatusMessage,
        chartResult.usefulGodCandidatesStatusMessage,
        chartResult.patternCandidatesStatusMessage,
        chartResult.majorFortunesStatusMessage,
        chartResult.annualFortunesStatusMessage,
        chartResult.majorFortuneDirectionStatusMessage,
        chartResult.solarTermDifferencePreparationStatusMessage
    };
    const bool includesProvisionalData = hasProvisionalHint(statusMessages)
        || chartResult.description.contains(QStringLiteral("provisional"));
    const bool includesAdoptedSpec = hasAdoptedSpecHint(statusMessages)
        || chartResult.description.contains(QStringLiteral("採用仕様"));

    const QString tenGodLine = tenGodSummaryText(chartResult.tenGods);
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
        tenGodLine,
        strengthLine,
        usefulGodLine,
        patternLine,
        climateLine,
        fortuneLine,
        includesAdoptedSpec
            ? QStringLiteral("月柱や行運は採用仕様に基づく判定を含みます。")
            : QString(),
        includesProvisionalData
            ? QStringLiteral("一部に provisional / 暫定データや参考表示を含みます。")
            : QString()
    });
    if (summaryText.isEmpty()) {
        summaryText = QStringLiteral("命式情報が不足しているため、解釈要約は未対応です。");
    }

    const QString majorRelationSummary = firstRelationSummary(chartResult.majorFortunes);
    const QString annualRelationSummary = firstRelationSummary(chartResult.annualFortunes);

    QStringList detailLines;
    detailLines.append(tenGodDetailLines(chartResult.tenGods));
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

    detailLines << QStringLiteral("命式計算の採用仕様:");
    const QStringList calculationStatusLines{
        compactStatusLine(QStringLiteral("月柱判定根拠"), chartResult.monthPillarStatusMessage),
        compactStatusLine(QStringLiteral("五行集計の補足"), chartResult.fiveElementDistributionStatusMessage),
        compactStatusLine(QStringLiteral("季節評価の補足"), chartResult.seasonalEvaluationStatusMessage),
        compactStatusLine(QStringLiteral("順逆の補足"), chartResult.majorFortuneDirectionStatusMessage),
        compactStatusLine(QStringLiteral("節入り差準備"), chartResult.solarTermDifferencePreparationStatusMessage)
    };
    for (const QString &line : calculationStatusLines) {
        if (!line.isEmpty()) {
            detailLines << QStringLiteral("  %1").arg(line);
        }
    }

    detailLines << QStringLiteral("暫定性と候補の補足:");
    const QStringList candidateStatusLines{
        compactStatusLine(QStringLiteral("強弱評価の補足"), chartResult.strengthEvaluationStatusMessage),
        compactStatusLine(QStringLiteral("寒暖・乾湿評価の補足"), chartResult.climateEvaluationStatusMessage),
        compactStatusLine(QStringLiteral("用神候補の補足"), chartResult.usefulGodCandidatesStatusMessage),
        compactStatusLine(QStringLiteral("格局候補の補足"), chartResult.patternCandidatesStatusMessage)
    };
    for (const QString &line : candidateStatusLines) {
        if (!line.isEmpty()) {
            detailLines << QStringLiteral("  %1").arg(line);
        }
    }

    detailLines << QStringLiteral("大運・流年の補足:");
    const QStringList fortuneStatusLines{
        compactStatusLine(QStringLiteral("大運一覧"), chartResult.majorFortunesStatusMessage),
        compactStatusLine(QStringLiteral("流年一覧"), chartResult.annualFortunesStatusMessage)
    };
    for (const QString &line : fortuneStatusLines) {
        if (!line.isEmpty()) {
            detailLines << QStringLiteral("  %1").arg(line);
        }
    }

    const QString detailText = detailLines.join(QStringLiteral("\n"));

    QStringList cautionParts;
    cautionParts << QStringLiteral("この解釈は暫定候補と参考表示に基づく要約です。");
    if (includesAdoptedSpec) {
        cautionParts << QStringLiteral("月柱や行運には採用仕様に基づく判定が含まれます。");
    }
    if (includesProvisionalData) {
        cautionParts << QStringLiteral("節入りデータや候補表示に provisional / 暫定 / 参考表示が含まれる可能性があります。");
    }
    cautionParts << QStringLiteral("大運・流年は参考表示段階の要素を含みます。");
    cautionParts << QStringLiteral("泰山流固有ルールや吉凶断定は未確定部分が残っています。");

    return {
        summaryText,
        detailText,
        cautionParts.join(QStringLiteral(" " ))
    };
}
