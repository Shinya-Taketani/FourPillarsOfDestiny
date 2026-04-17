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

struct ElementRoleMeaning
{
    QString basicMeaning;
    QString positiveExpression;
    QString cautionExpression;
    QString actionTheme;
    QString relationshipTheme;
    QString fortuneCycleTheme;
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

const QMap<QString, ElementRoleMeaning> &joyElementMeanings()
{
    static const QMap<QString, ElementRoleMeaning> meanings{
        {
            QStringLiteral("木"),
            {
                QStringLiteral("成長・学習・発展の力が命式を助ける傾向があります。"),
                QStringLiteral("学びや改善を積み上げるほど流れを整えやすい要素です。"),
                QStringLiteral("理想を広げ過ぎると、方針固定や頑なさとして出やすい面があります。"),
                QStringLiteral("企画、改善、開拓を前向きに進めやすくします。"),
                QStringLiteral("育成や長期的な信頼形成を支えやすい傾向があります。"),
                QStringLiteral("木の巡りは、拡大や学び直しを進めやすい追い風として働きやすいです。")
            }
        },
        {
            QStringLiteral("火"),
            {
                QStringLiteral("熱意・可視化・表現の力が命式を助ける傾向があります。"),
                QStringLiteral("前向きな推進力や明るい発信力を支えやすい要素です。"),
                QStringLiteral("熱が過ぎると、焦りや感情先行として表れやすい面があります。"),
                QStringLiteral("発信、決断、着手の勢いを出しやすくします。"),
                QStringLiteral("場を温め、率直なやり取りを進めやすい傾向があります。"),
                QStringLiteral("火の巡りは、新規始動や目立つ役割に追い風になりやすいです。")
            }
        },
        {
            QStringLiteral("土"),
            {
                QStringLiteral("安定・管理・蓄積の力が命式を助ける傾向があります。"),
                QStringLiteral("基盤を整え、生活や仕事を安定させやすい要素です。"),
                QStringLiteral("守りに入り過ぎると、停滞や抱え込みとして出やすい面があります。"),
                QStringLiteral("維持、管理、継続を丁寧に進めやすくします。"),
                QStringLiteral("包容や仲介、受け止める力を支えやすい傾向があります。"),
                QStringLiteral("土の巡りは、生活基盤や運営体制の安定化に向きやすいです。")
            }
        },
        {
            QStringLiteral("金"),
            {
                QStringLiteral("規律・精査・決断の力が命式を助ける傾向があります。"),
                QStringLiteral("磨き込みや取捨選択によって成果を締めやすい要素です。"),
                QStringLiteral("厳しさが過ぎると、批判過多や硬直として出やすい面があります。"),
                QStringLiteral("整理、選別、品質改善を進めやすくします。"),
                QStringLiteral("礼節や境界感覚を整えやすい傾向があります。"),
                QStringLiteral("金の巡りは、評価、契約、品質管理の整理に追い風になりやすいです。")
            }
        },
        {
            QStringLiteral("水"),
            {
                QStringLiteral("知性・柔軟性・情報処理の力が命式を助ける傾向があります。"),
                QStringLiteral("流れを読み、企画や調整をしなやかに進めやすい要素です。"),
                QStringLiteral("慎重さが過ぎると、考え過ぎや受け身として出やすい面があります。"),
                QStringLiteral("調査、企画、交渉、変化対応を進めやすくします。"),
                QStringLiteral("傾聴や共感、距離感の調整を支えやすい傾向があります。"),
                QStringLiteral("水の巡りは、学び直しや情報収集、人脈形成に追い風になりやすいです。")
            }
        }
    };

    return meanings;
}

const QMap<QString, ElementRoleMeaning> &harmfulElementMeanings()
{
    static const QMap<QString, ElementRoleMeaning> meanings{
        {
            QStringLiteral("木"),
            {
                QStringLiteral("成長欲や拡張性が過剰化し、理想先行に傾きやすい面があります。"),
                QStringLiteral("問題点を可視化し、見直しのきっかけにはなります。"),
                QStringLiteral("広げ過ぎ、目標過大、方針固定として出やすい点に注意が必要です。"),
                QStringLiteral("新規を増やし過ぎたり、課題を広げ過ぎたりしやすくなります。"),
                QStringLiteral("親切心が干渉に変わりやすいことがあります。"),
                QStringLiteral("木の巡りが忌神側に回ると、増やすほど乱れやすい時期になりがちです。")
            }
        },
        {
            QStringLiteral("火"),
            {
                QStringLiteral("熱意や自己表現が過熱し、焦りや衝動に出やすい面があります。"),
                QStringLiteral("停滞箇所を一気に照らして気づきを与えることがあります。"),
                QStringLiteral("感情の先走り、見栄、短気として出やすい点に注意が必要です。"),
                QStringLiteral("勢いで動き過ぎ、収拾より先に着手してしまいやすくなります。"),
                QStringLiteral("押しの強さや対立の熱さが増えやすいことがあります。"),
                QStringLiteral("火の巡りが忌神側だと、勢いはあっても収まりにくい年回りになりやすいです。")
            }
        },
        {
            QStringLiteral("土"),
            {
                QStringLiteral("安定欲や保全欲が重くなり、停滞や抱え込みに傾きやすい面があります。"),
                QStringLiteral("無理を止めるブレーキとして働くことがあります。"),
                QStringLiteral("心配し過ぎ、変化回避、溜め込みとして出やすい点に注意が必要です。"),
                QStringLiteral("維持に固執して判断が遅れやすくなります。"),
                QStringLiteral("世話焼きが束縛や過保護に変わりやすいことがあります。"),
                QStringLiteral("土の巡りが忌神側だと、生活や仕事の重さが増しやすい時期になりがちです。")
            }
        },
        {
            QStringLiteral("金"),
            {
                QStringLiteral("規律や選別の力がきつく出て、冷たさや過度な批評に傾きやすい面があります。"),
                QStringLiteral("不要なものを切る契機としては働くことがあります。"),
                QStringLiteral("批判過多、潔癖、関係断絶として出やすい点に注意が必要です。"),
                QStringLiteral("減点思考や厳格化が強く出やすくなります。"),
                QStringLiteral("距離を置き過ぎ、疑い深さが増えやすいことがあります。"),
                QStringLiteral("金の巡りが忌神側だと、評価や契約、金銭面の緊張が高まりやすいです。")
            }
        },
        {
            QStringLiteral("水"),
            {
                QStringLiteral("知性や柔軟性が過剰化し、迷いや拡散に出やすい面があります。"),
                QStringLiteral("状況を見直すためのブレーキとして働くことがあります。"),
                QStringLiteral("考え過ぎ、優柔不断、逃避として出やすい点に注意が必要です。"),
                QStringLiteral("情報収集が増える一方で、決断が遅れやすくなります。"),
                QStringLiteral("相手に合わせ過ぎて自分の軸を失いやすいことがあります。"),
                QStringLiteral("水の巡りが忌神側だと、情報過多と迷いが増えて進みにくくなりやすいです。")
            }
        }
    };

    return meanings;
}

const QMap<QString, ElementRoleMeaning> &climateElementMeanings()
{
    static const QMap<QString, ElementRoleMeaning> meanings{
        {
            QStringLiteral("木"),
            {
                QStringLiteral("停滞をほどき、生発や準備を助ける補助的な調候要素です。"),
                QStringLiteral("風通しをつくり、動き始める条件を整えやすいです。"),
                QStringLiteral("調候の主役として断定せず、補助要素として扱うのが安全です。"),
                QStringLiteral("硬くなった流れに芽を出しやすくします。"),
                QStringLiteral("関係のこわばりをほどきやすい傾向があります。"),
                QStringLiteral("木の調候は、伸びる前の下ごしらえとして読むのが無理のない扱いです。")
            }
        },
        {
            QStringLiteral("火"),
            {
                QStringLiteral("冷えた命式を暖め、活動性や社交性を通しやすくする調候の中心です。"),
                QStringLiteral("冷えを解き、行動や対人の動線を通しやすくします。"),
                QStringLiteral("調候の火は即吉断定ではなく、温め過ぎにも注意が必要です。"),
                QStringLiteral("凍った判断や着手の重さをほどきやすくします。"),
                QStringLiteral("温かみや開放感を出しやすい傾向があります。"),
                QStringLiteral("火の調候は、寒冷や停滞の強い命式で環境を動かす助けになりやすいです。")
            }
        },
        {
            QStringLiteral("土"),
            {
                QStringLiteral("緩衝や受け皿、保温の役割を持つ補助的な調候要素です。"),
                QStringLiteral("土台を整え、温度差の揺れをやわらげやすくします。"),
                QStringLiteral("直接の調候神として固定せず、補助的に扱うのが安全です。"),
                QStringLiteral("場づくりや定着、支えを整えやすくします。"),
                QStringLiteral("安心感や受容の土台をつくりやすい傾向があります。"),
                QStringLiteral("土の調候は、環境の受け皿を整える時期として読むのが無理のない扱いです。")
            }
        },
        {
            QStringLiteral("金"),
            {
                QStringLiteral("収斂や整理、剪定を助ける補助的な調候要素です。"),
                QStringLiteral("余分を削ぎ、機能を通しやすくします。"),
                QStringLiteral("調候の主役として一般化せず、整理役として控えめに扱うのが安全です。"),
                QStringLiteral("基準づくりや整理整頓を進めやすくします。"),
                QStringLiteral("距離感や境界線を整えやすい傾向があります。"),
                QStringLiteral("金の調候は、整理や標準化が進みやすい時期として読むのが無理のない扱いです。")
            }
        },
        {
            QStringLiteral("水"),
            {
                QStringLiteral("熱燥を鎮め、潤いと冷静さを回復しやすくする調候の中心です。"),
                QStringLiteral("熱を冷まし、乾きを潤して判断を落ち着かせやすくします。"),
                QStringLiteral("調候の水は即吉断定ではなく、冷やし過ぎにも注意が必要です。"),
                QStringLiteral("過熱した判断や感情を鎮めやすくします。"),
                QStringLiteral("聞く力や鎮静、感情のクーリングに出やすい傾向があります。"),
                QStringLiteral("水の調候は、熱燥過多の命式で立て直しを助けやすいです。")
            }
        }
    };

    return meanings;
}

const ElementRoleMeaning *meaningForElementRole(const QString &element, const QString &role)
{
    const QMap<QString, ElementRoleMeaning> *meanings = nullptr;
    if (role == QStringLiteral("joy")) {
        meanings = &joyElementMeanings();
    } else if (role == QStringLiteral("harmful")) {
        meanings = &harmfulElementMeanings();
    } else if (role == QStringLiteral("climate")) {
        meanings = &climateElementMeanings();
    }

    if (meanings == nullptr) {
        return nullptr;
    }

    const auto it = meanings->constFind(element);
    if (it == meanings->constEnd()) {
        return nullptr;
    }

    return &it.value();
}

QString firstUsefulElement(const QVariantMap &usefulGodCandidates)
{
    return usefulGodCandidates.value(QStringLiteral("candidates")).toStringList().value(0);
}

QString harmfulElementCandidate(const QVariantMap &usefulGodCandidates)
{
    const QVariantList rankedElements = usefulGodCandidates.value(QStringLiteral("rankedElements")).toList();
    if (!rankedElements.isEmpty()) {
        const QString lastElement = rankedElements.constLast().toString();
        if (!lastElement.isEmpty()) {
            return lastElement;
        }
    }

    const QStringList candidates = usefulGodCandidates.value(QStringLiteral("candidates")).toStringList();
    if (candidates.size() >= 2) {
        return candidates.constLast();
    }

    return QString();
}

QString climateElementCandidate(
    const QVariantMap &seasonalEvaluation,
    const QVariantMap &climateEvaluation
)
{
    const QString temperature = climateEvaluation.value(QStringLiteral("temperature")).toString();
    const QString moisture = climateEvaluation.value(QStringLiteral("moisture")).toString();
    const QString suitability = seasonalEvaluation.value(QStringLiteral("suitability")).toString();

    if (temperature.contains(QStringLiteral("寒"))) {
        return QStringLiteral("火");
    }
    if (temperature.contains(QStringLiteral("暖")) || temperature.contains(QStringLiteral("熱"))) {
        return QStringLiteral("水");
    }
    if (moisture.contains(QStringLiteral("乾"))) {
        return QStringLiteral("水");
    }
    if (moisture.contains(QStringLiteral("湿"))) {
        return QStringLiteral("火");
    }
    if (suitability.contains(QStringLiteral("不利"))) {
        return QStringLiteral("火");
    }

    return QString();
}

QString usefulElementSummaryText(
    const QVariantMap &usefulGodCandidates,
    const QVariantMap &seasonalEvaluation,
    const QVariantMap &climateEvaluation
)
{
    QStringList parts;

    const QString usefulElement = firstUsefulElement(usefulGodCandidates);
    const ElementRoleMeaning *usefulMeaning = meaningForElementRole(usefulElement, QStringLiteral("joy"));
    if (!usefulElement.isEmpty() && usefulMeaning != nullptr) {
        parts << QStringLiteral("喜神候補の%1は、%2 %3").arg(
            usefulElement,
            usefulMeaning->basicMeaning,
            usefulMeaning->positiveExpression
        );
    }

    const QString harmfulElement = harmfulElementCandidate(usefulGodCandidates);
    const ElementRoleMeaning *harmfulMeaning = meaningForElementRole(harmfulElement, QStringLiteral("harmful"));
    if (!harmfulElement.isEmpty() && harmfulMeaning != nullptr && harmfulElement != usefulElement) {
        parts << QStringLiteral("忌神側の%1は、%2").arg(harmfulElement, harmfulMeaning->cautionExpression);
    }

    const QString climateElement = climateElementCandidate(seasonalEvaluation, climateEvaluation);
    const ElementRoleMeaning *climateMeaning = meaningForElementRole(climateElement, QStringLiteral("climate"));
    if (!climateElement.isEmpty() && climateMeaning != nullptr) {
        parts << QStringLiteral("調候用神候補の%1は、%2").arg(climateElement, climateMeaning->basicMeaning);
    }

    QStringList filtered;
    for (const QString &part : parts) {
        if (!part.trimmed().isEmpty()) {
            filtered << part.trimmed();
        }
    }
    return filtered.join(QStringLiteral(" "));
}

QStringList usefulElementDetailLines(
    const QVariantMap &usefulGodCandidates,
    const QVariantMap &seasonalEvaluation,
    const QVariantMap &climateEvaluation
)
{
    QStringList lines;

    const QString usefulElement = firstUsefulElement(usefulGodCandidates);
    const ElementRoleMeaning *usefulMeaning = meaningForElementRole(usefulElement, QStringLiteral("joy"));
    if (!usefulElement.isEmpty() && usefulMeaning != nullptr) {
        lines << QStringLiteral("喜神の意味: %1").arg(usefulElement);
        lines << QStringLiteral("  基本意味: %1").arg(usefulMeaning->basicMeaning);
        lines << QStringLiteral("  長所: %1").arg(usefulMeaning->positiveExpression);
        lines << QStringLiteral("  注意点: %1").arg(usefulMeaning->cautionExpression);
        lines << QStringLiteral("  行動面: %1").arg(usefulMeaning->actionTheme);
        lines << QStringLiteral("  対人面: %1").arg(usefulMeaning->relationshipTheme);
        lines << QStringLiteral("  行運面: %1").arg(usefulMeaning->fortuneCycleTheme);
    }

    const QString harmfulElement = harmfulElementCandidate(usefulGodCandidates);
    const ElementRoleMeaning *harmfulMeaning = meaningForElementRole(harmfulElement, QStringLiteral("harmful"));
    if (!harmfulElement.isEmpty() && harmfulMeaning != nullptr && harmfulElement != usefulElement) {
        lines << QStringLiteral("忌神の意味: %1").arg(harmfulElement);
        lines << QStringLiteral("  基本意味: %1").arg(harmfulMeaning->basicMeaning);
        lines << QStringLiteral("  長所寄りの見方: %1").arg(harmfulMeaning->positiveExpression);
        lines << QStringLiteral("  注意点: %1").arg(harmfulMeaning->cautionExpression);
        lines << QStringLiteral("  行動面: %1").arg(harmfulMeaning->actionTheme);
        lines << QStringLiteral("  対人面: %1").arg(harmfulMeaning->relationshipTheme);
        lines << QStringLiteral("  行運面: %1").arg(harmfulMeaning->fortuneCycleTheme);
    }

    const QString climateElement = climateElementCandidate(seasonalEvaluation, climateEvaluation);
    const ElementRoleMeaning *climateMeaning = meaningForElementRole(climateElement, QStringLiteral("climate"));
    if (!climateElement.isEmpty() && climateMeaning != nullptr) {
        lines << QStringLiteral("調候用神の補足: %1").arg(climateElement);
        lines << QStringLiteral("  基本意味: %1").arg(climateMeaning->basicMeaning);
        lines << QStringLiteral("  長所: %1").arg(climateMeaning->positiveExpression);
        lines << QStringLiteral("  注意点: %1").arg(climateMeaning->cautionExpression);
        lines << QStringLiteral("  行動面: %1").arg(climateMeaning->actionTheme);
        lines << QStringLiteral("  対人面: %1").arg(climateMeaning->relationshipTheme);
        lines << QStringLiteral("  行運面: %1").arg(climateMeaning->fortuneCycleTheme);
    }

    return lines;
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
    const QString usefulElementLine = usefulElementSummaryText(
        chartResult.usefulGodCandidates,
        chartResult.seasonalEvaluation,
        chartResult.climateEvaluation
    );
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
        usefulElementLine,
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
    detailLines.append(usefulElementDetailLines(
        chartResult.usefulGodCandidates,
        chartResult.seasonalEvaluation,
        chartResult.climateEvaluation
    ));

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
