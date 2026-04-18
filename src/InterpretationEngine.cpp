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

struct DayMasterMeaning
{
    QString basicNature;
    QString positiveExpression;
    QString cautionExpression;
    QString workTheme;
    QString relationshipTheme;
};

struct BranchSectionMeaning
{
    QString coreMeaning;
    QString positiveExpression;
    QString cautionExpression;
};

struct KinshipMeaning
{
    QString basicMeaning;
    QString positiveExpression;
    QString cautionExpression;
    QString relationshipTheme;
    QString distanceExpression;
};

QString compactStatusLine(const QString &label, const QString &statusMessage)
{
    if (statusMessage.trimmed().isEmpty()) {
        return QString();
    }

    return QStringLiteral("%1: %2").arg(label, statusMessage.trimmed());
}

QString heavenlyStemOfPillar(const QString &pillar)
{
    return pillar.isEmpty() ? QString() : pillar.left(1);
}

QString earthlyBranchOfPillar(const QString &pillar)
{
    return pillar.size() < 2 ? QString() : pillar.mid(1, 1);
}

const QMap<QString, DayMasterMeaning> &dayMasterMeanings()
{
    static const QMap<QString, DayMasterMeaning> meanings{
        {QStringLiteral("甲"), {QStringLiteral("大樹のように骨格をつくり、大きな方向性を見て進めやすい日主です。"), QStringLiteral("筋を通し、道を切り開く力として表れやすいです。"), QStringLiteral("硬直すると押し切りや独断に傾きやすい面があります。"), QStringLiteral("方針設計、経営、開拓、企画統率に向きやすいです。"), QStringLiteral("まっすぐ支えようとする一方、相手にも筋を求めやすい傾向があります。")}},
        {QStringLiteral("乙"), {QStringLiteral("草花やつるのように、しなやかに広がりながら環境へ馴染みやすい日主です。"), QStringLiteral("柔らかな調整力や感受性として表れやすいです。"), QStringLiteral("迷いや遠慮が強まると決め切れなさに傾きやすい面があります。"), QStringLiteral("調整、補佐、デザイン、対人支援に向きやすいです。"), QStringLiteral("相手に合わせるのが上手い一方、本音を後ろへ置きやすい傾向があります。")}},
        {QStringLiteral("丙"), {QStringLiteral("太陽のように明るく、外へ熱と意志を放ちやすい日主です。"), QStringLiteral("率直さや牽引力、場を照らす力として表れやすいです。"), QStringLiteral("勢いが過ぎると大づかみや感情先行に傾きやすい面があります。"), QStringLiteral("発信、営業、先導、表現活動に向きやすいです。"), QStringLiteral("明快で温かい一方、強く出過ぎると圧になりやすい傾向があります。")}},
        {QStringLiteral("丁"), {QStringLiteral("灯火のように、細やかな観察で必要な場所を照らしやすい日主です。"), QStringLiteral("気配りや集中力、静かな情熱として表れやすいです。"), QStringLiteral("神経を使い過ぎると傷つきやすさやこだわりに傾きやすい面があります。"), QStringLiteral("研究、編集、調整、専門職に向きやすいです。"), QStringLiteral("繊細に寄り添える一方、気疲れしやすい傾向があります。")}},
        {QStringLiteral("戊"), {QStringLiteral("山岳のように重厚で、土台を保ち続けやすい日主です。"), QStringLiteral("安定感、包容力、構えの大きさとして表れやすいです。"), QStringLiteral("動きが遅くなると固執や鈍重さに傾きやすい面があります。"), QStringLiteral("管理、保全、基盤構築、長期運営に向きやすいです。"), QStringLiteral("受け止める力が強い一方、抱え込みやすい傾向があります。")}},
        {QStringLiteral("己"), {QStringLiteral("田園の湿土のように、細やかに育てて蓄えやすい日主です。"), QStringLiteral("面倒見や実務感覚、育成力として表れやすいです。"), QStringLiteral("内に溜め込み過ぎると過干渉や悩み込みに傾きやすい面があります。"), QStringLiteral("実務、育成、補佐、現場調整に向きやすいです。"), QStringLiteral("親身で支え役になりやすい一方、境界が曖昧になりやすい傾向があります。")}},
        {QStringLiteral("庚"), {QStringLiteral("鋼鉄のように剛断で、是非をはっきり分けやすい日主です。"), QStringLiteral("決断力や突破力、改革性として表れやすいです。"), QStringLiteral("強さが過ぎると攻撃性や対立化に傾きやすい面があります。"), QStringLiteral("執行、交渉、改革、難所突破に向きやすいです。"), QStringLiteral("頼もしさが出る一方、白黒を急ぎ過ぎる傾向があります。")}},
        {QStringLiteral("辛"), {QStringLiteral("宝石のように精製され、品位や精度を重んじやすい日主です。"), QStringLiteral("審美眼や精密さ、洗練として表れやすいです。"), QStringLiteral("神経を使い過ぎると傷つきやすさや批判性に傾きやすい面があります。"), QStringLiteral("品質管理、審査、美容工芸、調整に向きやすいです。"), QStringLiteral("品よく距離を取れる一方、選別が強く出やすい傾向があります。")}},
        {QStringLiteral("壬"), {QStringLiteral("大洋のように流動し、広く包み込みながら動きやすい日主です。"), QStringLiteral("スケール感や応用力、融通として表れやすいです。"), QStringLiteral("広がり過ぎると拡散や落ち着かなさに傾きやすい面があります。"), QStringLiteral("流通、企画、越境的な仕事、変化対応に向きやすいです。"), QStringLiteral("広くつながれる一方、散りやすさが出やすい傾向があります。")}},
        {QStringLiteral("癸"), {QStringLiteral("雨露のように静かに浸透し、滋養を届けやすい日主です。"), QStringLiteral("観察力や静かな知性、染み込む理解として表れやすいです。"), QStringLiteral("迷いが強まると受け身や湿っぽさに傾きやすい面があります。"), QStringLiteral("研究、記録、支援、ケア、補助に向きやすいです。"), QStringLiteral("心情理解が深い一方、言語化に時間がかかりやすい傾向があります。")}}
    };

    return meanings;
}

const QMap<QString, BranchSectionMeaning> &monthBranchMeanings()
{
    static const QMap<QString, BranchSectionMeaning> meanings{
        {QStringLiteral("子"), {QStringLiteral("情報の流れを素早く察し、周囲の変化に感応しやすい月支です。"), QStringLiteral("察知力や機転として社会面に出やすいです。"), QStringLiteral("落ち着きにくさや散りやすさとして見えやすい面があります。")}},
        {QStringLiteral("丑"), {QStringLiteral("慎重に積み上げ、実務を安定させやすい月支です。"), QStringLiteral("粘り強さや堅実さとして社会面に出やすいです。"), QStringLiteral("変化への反応が遅く、重さとして見えやすい面があります。")}},
        {QStringLiteral("寅"), {QStringLiteral("先に動いて道を開き、発動力を社会面へ出しやすい月支です。"), QStringLiteral("開拓力や行動力として見られやすいです。"), QStringLiteral("焦りや衝動が先に出やすい面があります。")}},
        {QStringLiteral("卯"), {QStringLiteral("関係調整や洗練を大切にし、協調を社会面へ出しやすい月支です。"), QStringLiteral("美意識や調整力として見られやすいです。"), QStringLiteral("気疲れや優柔不断として出やすい面があります。")}},
        {QStringLiteral("辰"), {QStringLiteral("調整しながら展開し、複数の要素を抱えて進めやすい月支です。"), QStringLiteral("段取り力や包含力として見られやすいです。"), QStringLiteral("曖昧さや迷いが残りやすい面があります。")}},
        {QStringLiteral("巳"), {QStringLiteral("内側で熱を持ちながら見抜き、戦略的に絞り込みやすい月支です。"), QStringLiteral("洞察力や戦略性として社会面へ出やすいです。"), QStringLiteral("牽制や疑い深さとして見えやすい面があります。")}},
        {QStringLiteral("午"), {QStringLiteral("率直に押し出し、熱量をそのまま社会面へ出しやすい月支です。"), QStringLiteral("明るさや推進力として見られやすいです。"), QStringLiteral("熱しやすさや波の大きさとして見えやすい面があります。")}},
        {QStringLiteral("未"), {QStringLiteral("守り育てる姿勢が社会面に出やすく、穏当に支えやすい月支です。"), QStringLiteral("面倒見や穏当さとして見られやすいです。"), QStringLiteral("抱え込みやため込みとして出やすい面があります。")}},
        {QStringLiteral("申"), {QStringLiteral("目的へ合理的に動き、機転を社会面で使いやすい月支です。"), QStringLiteral("要領や判断の速さとして見られやすいです。"), QStringLiteral("打算的、落ち着き不足として見えやすい面があります。")}},
        {QStringLiteral("酉"), {QStringLiteral("精査して整える姿勢が社会面へ出やすく、完成度を求めやすい月支です。"), QStringLiteral("精度や審美眼として見られやすいです。"), QStringLiteral("批判的、神経質として出やすい面があります。")}},
        {QStringLiteral("戌"), {QStringLiteral("原則や責任を守り抜く姿勢が社会面へ出やすい月支です。"), QStringLiteral("義理堅さや責任感として見られやすいです。"), QStringLiteral("頑なさや内圧として出やすい面があります。")}},
        {QStringLiteral("亥"), {QStringLiteral("受け入れながら広げる姿勢が社会面に出やすく、包摂力を持ちやすい月支です。"), QStringLiteral("共感や理想性として見られやすいです。"), QStringLiteral("境界が曖昧になりやすい面があります。")}}
    };

    return meanings;
}

const QMap<QString, BranchSectionMeaning> &dayBranchPartnerMeanings()
{
    static const QMap<QString, BranchSectionMeaning> meanings{
        {QStringLiteral("子"), {QStringLiteral("知的で会話重視の関係をつくりやすい配偶者宮の傾向があります。"), QStringLiteral("友達のように言葉を交わしながら関係を育てやすいです。"), QStringLiteral("情の温度が下がると、理屈先行に見えやすい面があります。")}},
        {QStringLiteral("丑"), {QStringLiteral("堅実で実務的な安心感を求めやすい配偶者宮の傾向があります。"), QStringLiteral("長続き志向で生活を整えやすいです。"), QStringLiteral("変化が少なく、停滞感として出やすい面があります。")}},
        {QStringLiteral("寅"), {QStringLiteral("活動的で前進型の関係を求めやすい配偶者宮の傾向があります。"), QStringLiteral("刺激と成長をともに求めやすいです。"), QStringLiteral("衝突時には熱くなりやすい面があります。")}},
        {QStringLiteral("卯"), {QStringLiteral("柔和で調和的な関係を求めやすい配偶者宮の傾向があります。"), QStringLiteral("穏やかで気づかいのある関係をつくりやすいです。"), QStringLiteral("遠慮が重なると本音が遅れやすい面があります。")}},
        {QStringLiteral("辰"), {QStringLiteral("しっかり支え合いながら共同成長を求めやすい配偶者宮の傾向があります。"), QStringLiteral("頼り合いながら現実を整えやすいです。"), QStringLiteral("主導権争いになると重くなりやすい面があります。")}},
        {QStringLiteral("巳"), {QStringLiteral("冷静で洞察的な距離感を大切にしやすい配偶者宮の傾向があります。"), QStringLiteral("知的で慎重な相手を評価しやすいです。"), QStringLiteral("疑い深さや牽制として出やすい面があります。")}},
        {QStringLiteral("午"), {QStringLiteral("情熱や自由さを共有できる関係を求めやすい配偶者宮の傾向があります。"), QStringLiteral("明るさや表現の楽しさを関係へ持ち込みやすいです。"), QStringLiteral("束縛への反発が強く出やすい面があります。")}},
        {QStringLiteral("未"), {QStringLiteral("思いやりや癒やしを重視し、安心感のある関係を求めやすい配偶者宮の傾向があります。"), QStringLiteral("優しく支え合う関係をつくりやすいです。"), QStringLiteral("依存や抱え込みとして出やすい面があります。")}},
        {QStringLiteral("申"), {QStringLiteral("機転や戦略性を共有できる関係を求めやすい配偶者宮の傾向があります。"), QStringLiteral("実利やテンポの良さを関係へ持ち込みやすいです。"), QStringLiteral("計算高く見えやすい面があります。")}},
        {QStringLiteral("酉"), {QStringLiteral("美意識や精査を重視し、洗練された関係を求めやすい配偶者宮の傾向があります。"), QStringLiteral("品位や選択眼を関係へ活かしやすいです。"), QStringLiteral("批評モードが強いと緊張感を生みやすい面があります。")}},
        {QStringLiteral("戌"), {QStringLiteral("誠実さや責任感を軸に、約束を守る関係を求めやすい配偶者宮の傾向があります。"), QStringLiteral("義理堅く支え合う関係をつくりやすいです。"), QStringLiteral("頑固さや譲れなさとして出やすい面があります。")}},
        {QStringLiteral("亥"), {QStringLiteral("包摂や理想を共有し、共感重視の関係を求めやすい配偶者宮の傾向があります。"), QStringLiteral("優しく受け止め合う関係をつくりやすいです。"), QStringLiteral("境界が甘くなりやすい面があります。")}}
    };

    return meanings;
}

const QMap<QString, KinshipMeaning> &kinshipMeanings()
{
    static const QMap<QString, KinshipMeaning> meanings{
        {
            QStringLiteral("父"),
            {
                QStringLiteral("現実面や社会的な基準を示す存在として意識しやすい六親です。"),
                QStringLiteral("現実感覚や行動基準を受け取りやすい関係として出やすいです。"),
                QStringLiteral("距離が出ると評価や結果だけが先に立ちやすい面があります。"),
                QStringLiteral("役割や責任を通して関係を認識しやすい傾向があります。"),
                QStringLiteral("近いと指針になりやすく、離れると厳しさとして感じやすいです。")
            }
        },
        {
            QStringLiteral("母"),
            {
                QStringLiteral("保護や受容、支えを示す存在として意識しやすい六親です。"),
                QStringLiteral("安心感や受け止められる感覚につながりやすい関係として出やすいです。"),
                QStringLiteral("近すぎると依存や甘えとして出やすい面があります。"),
                QStringLiteral("支援や育成を通して関係を深めやすい傾向があります。"),
                QStringLiteral("近いと心の支えになりやすく、離れると不足感として残りやすいです。")
            }
        },
        {
            QStringLiteral("兄弟"),
            {
                QStringLiteral("競争や並走、自立の鏡として意識しやすい六親です。"),
                QStringLiteral("励まし合いながら力を出す関係として働きやすいです。"),
                QStringLiteral("張り合いが強すぎると対立や比較に傾きやすい面があります。"),
                QStringLiteral("対等性や同じ目線を通して関係が動きやすい傾向があります。"),
                QStringLiteral("近いと刺激になりやすく、離れると比較意識だけが残りやすいです。")
            }
        },
        {
            QStringLiteral("姉妹"),
            {
                QStringLiteral("共感や日常的な支え合いとして意識しやすい六親です。"),
                QStringLiteral("気づかいや協力を通して関係を整えやすいです。"),
                QStringLiteral("遠慮が重なると本音が見えにくくなりやすい面があります。"),
                QStringLiteral("生活感覚や感情共有を通して関係が深まりやすい傾向があります。"),
                QStringLiteral("近いと安心感になりやすく、離れると気持ちのすれ違いが残りやすいです。")
            }
        },
        {
            QStringLiteral("妻"),
            {
                QStringLiteral("生活の安定や現実の共有を担う関係として意識しやすい六親です。"),
                QStringLiteral("共に暮らしや実務を整える力として働きやすいです。"),
                QStringLiteral("役割負担が偏ると実利だけの関係に見えやすい面があります。"),
                QStringLiteral("安心感、日常運営、支え合いを通して関係が育ちやすい傾向があります。"),
                QStringLiteral("近いと実務の相棒になりやすく、離れるとすれ違いが生活面へ出やすいです。")
            }
        },
        {
            QStringLiteral("夫"),
            {
                QStringLiteral("責任や秩序、外向きの支えを担う関係として意識しやすい六親です。"),
                QStringLiteral("守りや役割分担を通して安心感をつくりやすいです。"),
                QStringLiteral("規範が強すぎると窮屈さや上下感として出やすい面があります。"),
                QStringLiteral("責任感や約束を通して関係が安定しやすい傾向があります。"),
                QStringLiteral("近いと支柱になりやすく、離れると評価や形式だけが残りやすいです。")
            }
        },
        {
            QStringLiteral("子ども"),
            {
                QStringLiteral("未来への広がりや表現の延長として意識しやすい六親です。"),
                QStringLiteral("育成や創造の喜びとして働きやすいです。"),
                QStringLiteral("期待が強すぎると管理や心配の形で出やすい面があります。"),
                QStringLiteral("成長の見守りや表現の受け渡しを通して関係が深まりやすい傾向があります。"),
                QStringLiteral("近いと育てる喜びになりやすく、離れると期待や不安として残りやすいです。")
            }
        }
    };

    return meanings;
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

QString dayMasterSummaryText(const QString &dayPillar)
{
    const QString stem = heavenlyStemOfPillar(dayPillar);
    const auto it = dayMasterMeanings().constFind(stem);
    if (it == dayMasterMeanings().constEnd()) {
        return QString();
    }

    return QStringLiteral("日主の%1は、%2").arg(stem, it.value().basicNature);
}

QStringList dayMasterDetailLines(const QString &dayPillar)
{
    const QString stem = heavenlyStemOfPillar(dayPillar);
    const auto it = dayMasterMeanings().constFind(stem);
    if (it == dayMasterMeanings().constEnd()) {
        return {};
    }

    const DayMasterMeaning &meaning = it.value();
    return {
        QStringLiteral("日主の本質: %1").arg(stem),
        QStringLiteral("  基本性質: %1").arg(meaning.basicNature),
        QStringLiteral("  長所寄り: %1").arg(meaning.positiveExpression),
        QStringLiteral("  注意点: %1").arg(meaning.cautionExpression),
        QStringLiteral("  仕事面: %1").arg(meaning.workTheme),
        QStringLiteral("  対人面: %1").arg(meaning.relationshipTheme)
    };
}

QStringList monthBranchDetailLines(const QString &monthPillar)
{
    const QString branch = earthlyBranchOfPillar(monthPillar);
    const auto it = monthBranchMeanings().constFind(branch);
    if (it == monthBranchMeanings().constEnd()) {
        return {};
    }

    const BranchSectionMeaning &meaning = it.value();
    return {
        QStringLiteral("社会的な性質: %1").arg(branch),
        QStringLiteral("  核心: %1").arg(meaning.coreMeaning),
        QStringLiteral("  長所寄り: %1").arg(meaning.positiveExpression),
        QStringLiteral("  注意点: %1").arg(meaning.cautionExpression)
    };
}

QStringList dayBranchPartnerDetailLines(const QString &dayPillar)
{
    const QString branch = earthlyBranchOfPillar(dayPillar);
    const auto it = dayBranchPartnerMeanings().constFind(branch);
    if (it == dayBranchPartnerMeanings().constEnd()) {
        return {};
    }

    const BranchSectionMeaning &meaning = it.value();
    return {
        QStringLiteral("パートナー傾向: %1").arg(branch),
        QStringLiteral("  核心: %1").arg(meaning.coreMeaning),
        QStringLiteral("  長所寄り: %1").arg(meaning.positiveExpression),
        QStringLiteral("  注意点: %1").arg(meaning.cautionExpression)
    };
}

QString spouseKinshipKey()
{
    return QStringLiteral("配偶者");
}

QString spouseKinshipMeaningText()
{
    const KinshipMeaning *wifeMeaning = nullptr;
    const KinshipMeaning *husbandMeaning = nullptr;
    const auto &meanings = kinshipMeanings();
    const auto wifeIt = meanings.constFind(QStringLiteral("妻"));
    if (wifeIt != meanings.constEnd()) {
        wifeMeaning = &wifeIt.value();
    }
    const auto husbandIt = meanings.constFind(QStringLiteral("夫"));
    if (husbandIt != meanings.constEnd()) {
        husbandMeaning = &husbandIt.value();
    }

    if (wifeMeaning == nullptr && husbandMeaning == nullptr) {
        return QString();
    }

    if (wifeMeaning != nullptr && husbandMeaning != nullptr) {
        return QStringLiteral("妻は%1 夫は%2").arg(
            wifeMeaning->basicMeaning,
            husbandMeaning->basicMeaning
        );
    }

    return wifeMeaning != nullptr ? wifeMeaning->basicMeaning : husbandMeaning->basicMeaning;
}

QStringList kinshipDetailLines(const QString &dayPillar, const QString &monthPillar)
{
    const QString dayStem = heavenlyStemOfPillar(dayPillar);
    const QString dayBranch = earthlyBranchOfPillar(dayPillar);
    const QString monthBranch = earthlyBranchOfPillar(monthPillar);
    const auto &meanings = kinshipMeanings();

    auto buildLines = [&](const QString &label) -> QStringList {
        const auto it = meanings.constFind(label);
        if (it == meanings.constEnd()) {
            return {};
        }

        const KinshipMeaning &meaning = it.value();
        return {
            QStringLiteral("  %1: %2").arg(label, meaning.basicMeaning),
            QStringLiteral("    長所寄り: %1").arg(meaning.positiveExpression),
            QStringLiteral("    注意点: %1").arg(meaning.cautionExpression),
            QStringLiteral("    関係の出方: %1").arg(meaning.relationshipTheme),
            QStringLiteral("    距離感の補足: %1").arg(meaning.distanceExpression)
        };
    };

    QStringList lines{
        QStringLiteral("六親: 日主 %1 / 月支 %2 / 日支 %3 を踏まえた参考説明です。").arg(
            dayStem.isEmpty() ? QStringLiteral("未対応") : dayStem,
            monthBranch.isEmpty() ? QStringLiteral("未対応") : monthBranch,
            dayBranch.isEmpty() ? QStringLiteral("未対応") : dayBranch
        )
    };
    lines.append(buildLines(QStringLiteral("父")));
    lines.append(buildLines(QStringLiteral("母")));
    lines.append(buildLines(QStringLiteral("兄弟")));
    lines.append(buildLines(QStringLiteral("姉妹")));

    lines << QStringLiteral("  %1: %2").arg(spouseKinshipKey(), spouseKinshipMeaningText());
    if (!dayBranch.isEmpty()) {
        lines << QStringLiteral("    配偶者宮補足: 日支 %1 の傾向も参考にします。").arg(dayBranch);
    }

    lines.append(buildLines(QStringLiteral("子ども")));
    return lines;
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

QString kinshipSummaryText(const QString &dayPillar)
{
    const QString dayBranch = earthlyBranchOfPillar(dayPillar);
    if (dayBranch.isEmpty()) {
        return QString();
    }

    return QStringLiteral("家族や配偶者の読みは、日支 %1 を含む六親の参考説明を併記しています。").arg(dayBranch);
}

QString elementPolarityText(const QString &element, const QVariantMap &usefulGodCandidates)
{
    if (element.isEmpty()) {
        return QString();
    }

    if (firstUsefulElement(usefulGodCandidates) == element) {
        return QStringLiteral("%1は喜神候補").arg(element);
    }
    if (harmfulElementCandidate(usefulGodCandidates) == element) {
        return QStringLiteral("%1は忌神側").arg(element);
    }

    return QString();
}

QStringList candidateElementsFromFortune(const QVariantMap &fortune)
{
    const QString pillar = fortune.value(QStringLiteral("pillar")).toString();
    QStringList elements;
    const QString stem = heavenlyStemOfPillar(pillar);
    const QString branch = earthlyBranchOfPillar(pillar);

    auto appendIfMapped = [&elements](const QString &symbol) {
        static const QMap<QString, QString> map{
            {QStringLiteral("甲"), QStringLiteral("木")},
            {QStringLiteral("乙"), QStringLiteral("木")},
            {QStringLiteral("丙"), QStringLiteral("火")},
            {QStringLiteral("丁"), QStringLiteral("火")},
            {QStringLiteral("戊"), QStringLiteral("土")},
            {QStringLiteral("己"), QStringLiteral("土")},
            {QStringLiteral("庚"), QStringLiteral("金")},
            {QStringLiteral("辛"), QStringLiteral("金")},
            {QStringLiteral("壬"), QStringLiteral("水")},
            {QStringLiteral("癸"), QStringLiteral("水")},
            {QStringLiteral("寅"), QStringLiteral("木")},
            {QStringLiteral("卯"), QStringLiteral("木")},
            {QStringLiteral("巳"), QStringLiteral("火")},
            {QStringLiteral("午"), QStringLiteral("火")},
            {QStringLiteral("辰"), QStringLiteral("土")},
            {QStringLiteral("丑"), QStringLiteral("土")},
            {QStringLiteral("未"), QStringLiteral("土")},
            {QStringLiteral("戌"), QStringLiteral("土")},
            {QStringLiteral("申"), QStringLiteral("金")},
            {QStringLiteral("酉"), QStringLiteral("金")},
            {QStringLiteral("亥"), QStringLiteral("水")},
            {QStringLiteral("子"), QStringLiteral("水")}
        };
        const QString element = map.value(symbol);
        if (!element.isEmpty() && !elements.contains(element)) {
            elements << element;
        }
    };

    appendIfMapped(stem);
    appendIfMapped(branch);
    return elements;
}

QString fortuneRelationClause(const QVariantMap &fortune)
{
    const QString relationSummary = fortune.value(QStringLiteral("relationSummary")).toString();
    if (!relationSummary.isEmpty()) {
        return QStringLiteral("%1 が参考的に重なっています。").arg(relationSummary);
    }

    QStringList parts;
    const QStringList sameStemMatches = fortune.value(QStringLiteral("sameStemMatches")).toStringList();
    const QStringList sameBranchMatches = fortune.value(QStringLiteral("sameBranchMatches")).toStringList();
    const QStringList clashBranches = fortune.value(QStringLiteral("clashBranches")).toStringList();
    const QStringList stemCombinationCandidates = fortune.value(QStringLiteral("stemCombinationCandidates")).toStringList();

    if (!sameStemMatches.isEmpty()) {
        parts << QStringLiteral("同干 %1").arg(sameStemMatches.join(QStringLiteral(" / ")));
    }
    if (!sameBranchMatches.isEmpty()) {
        parts << QStringLiteral("同支 %1").arg(sameBranchMatches.join(QStringLiteral(" / ")));
    }
    if (!clashBranches.isEmpty()) {
        parts << QStringLiteral("冲候補 %1").arg(clashBranches.join(QStringLiteral(" / ")));
    }
    if (!stemCombinationCandidates.isEmpty()) {
        parts << QStringLiteral("干合候補 %1").arg(stemCombinationCandidates.join(QStringLiteral(" / ")));
    }

    if (parts.isEmpty()) {
        return QString();
    }

    return QStringLiteral("%1 が関係補足として見えます。").arg(parts.join(QStringLiteral("、")));
}

QString majorFortuneExplanationText(
    const QVariantList &majorFortunes,
    const QVariantMap &usefulGodCandidates
)
{
    const QVariantMap fortune = majorFortunes.value(0).toMap();
    const QString pillar = fortune.value(QStringLiteral("pillar")).toString();
    if (pillar.isEmpty()) {
        return QString();
    }

    const QString tenGod = fortune.value(QStringLiteral("tenGod")).toString();
    const QString twelvePhase = fortune.value(QStringLiteral("twelvePhase")).toString();
    const QString note = fortune.value(QStringLiteral("note")).toString();
    const QString relationClause = fortuneRelationClause(fortune);

    QString polarityClause;
    for (const QString &element : candidateElementsFromFortune(fortune)) {
        const QString polarity = elementPolarityText(element, usefulGodCandidates);
        if (!polarity.isEmpty()) {
            polarityClause = QStringLiteral("%1として働く場面があります。").arg(polarity);
            break;
        }
    }

    QStringList parts{
        QStringLiteral("%1の時期は、十年単位で流れを形づくる参考運です。").arg(pillar)
    };
    if (!tenGod.isEmpty()) {
        parts << QStringLiteral("表面には%1のテーマが出やすいです。").arg(tenGod);
    }
    if (!twelvePhase.isEmpty()) {
        parts << QStringLiteral("十二運では%1にあたり、勢いの出方をみる補足になります。").arg(twelvePhase);
    }
    if (!polarityClause.isEmpty()) {
        parts << polarityClause;
    }
    if (!relationClause.isEmpty()) {
        parts << relationClause;
    }
    if (!note.isEmpty()) {
        parts << QStringLiteral("補足メモとして %1").arg(note);
    }

    QStringList filtered;
    for (const QString &part : parts) {
        if (!part.trimmed().isEmpty()) {
            filtered << part.trimmed();
        }
    }
    return filtered.join(QStringLiteral(" "));
}

QString annualFortuneExplanationText(
    const QVariantList &annualFortunes,
    const QVariantList &majorFortunes,
    const QVariantMap &usefulGodCandidates
)
{
    const QVariantMap fortune = annualFortunes.value(0).toMap();
    const QString pillar = fortune.value(QStringLiteral("pillar")).toString();
    if (pillar.isEmpty()) {
        return QString();
    }

    const QString year = fortune.value(QStringLiteral("year")).toString();
    const QString tenGod = fortune.value(QStringLiteral("tenGod")).toString();
    const QString twelvePhase = fortune.value(QStringLiteral("twelvePhase")).toString();
    const QString relationClause = fortuneRelationClause(fortune);
    const QString currentMajorPillar = firstFortunePillar(majorFortunes);

    QString polarityClause;
    for (const QString &element : candidateElementsFromFortune(fortune)) {
        const QString polarity = elementPolarityText(element, usefulGodCandidates);
        if (!polarity.isEmpty()) {
            polarityClause = QStringLiteral("%1として短期的に表面化しやすいです。").arg(polarity);
            break;
        }
    }

    QStringList parts{
        QStringLiteral("%1年の%2は、その年の変化として表に出やすい参考運です。").arg(
            year.isEmpty() ? QStringLiteral("当年") : year,
            pillar
        )
    };
    if (!tenGod.isEmpty()) {
        parts << QStringLiteral("まず%1のテーマが前面に出やすいです。").arg(tenGod);
    }
    if (!currentMajorPillar.isEmpty()) {
        parts << QStringLiteral("現行の%1運と重なることで、その年の傾向が増幅または調整されやすくなります。").arg(currentMajorPillar);
    }
    if (!twelvePhase.isEmpty()) {
        parts << QStringLiteral("十二運では%1にあたり、その年の勢いの強弱をみる補足になります。").arg(twelvePhase);
    }
    if (!polarityClause.isEmpty()) {
        parts << polarityClause;
    }
    if (!relationClause.isEmpty()) {
        parts << relationClause;
    }

    QStringList filtered;
    for (const QString &part : parts) {
        if (!part.trimmed().isEmpty()) {
            filtered << part.trimmed();
        }
    }
    return filtered.join(QStringLiteral(" "));
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
    const QString dayMasterLine = dayMasterSummaryText(chartResult.dayPillar);
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
    const QString kinshipLine = kinshipSummaryText(chartResult.dayPillar);
    const QString majorFortuneDetail = majorFortuneExplanationText(
        chartResult.majorFortunes,
        chartResult.usefulGodCandidates
    );
    const QString annualFortuneDetail = annualFortuneExplanationText(
        chartResult.annualFortunes,
        chartResult.majorFortunes,
        chartResult.usefulGodCandidates
    );
    const QString fortuneCycleLine = (!majorFortuneDetail.isEmpty() || !annualFortuneDetail.isEmpty())
        ? QStringLiteral("先頭大運・先頭流年の参考説明を併記しています。")
        : QString();

    QString summaryText = joinedNonEmpty({
        tenGodLine,
        dayMasterLine,
        strengthLine,
        usefulGodLine,
        usefulElementLine,
        patternLine,
        climateLine,
        fortuneLine,
        kinshipLine,
        fortuneCycleLine,
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
    detailLines.append(dayMasterDetailLines(chartResult.dayPillar));
    detailLines.append(monthBranchDetailLines(chartResult.monthPillar));
    detailLines.append(dayBranchPartnerDetailLines(chartResult.dayPillar));
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
    detailLines.append(kinshipDetailLines(chartResult.dayPillar, chartResult.monthPillar));
    if (!majorFortuneDetail.isEmpty()) {
        detailLines << QStringLiteral("大運補足: %1").arg(majorFortuneDetail);
    }
    if (!annualFortuneDetail.isEmpty()) {
        detailLines << QStringLiteral("歳運補足: %1").arg(annualFortuneDetail);
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
