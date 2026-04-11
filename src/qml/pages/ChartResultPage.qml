import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami

Kirigami.Page {
    title: "命式結果"

    property var chartResult: ({
        yearPillar: "",
        monthPillar: "",
        dayPillar: "",
        hourPillar: "",
        description: "",
        monthPillarStatusMessage: "",
        tenGods: {},
        hiddenStems: {},
        fiveElements: {},
        fiveElementDistributionStatusMessage: "",
        seasonalEvaluation: {},
        seasonalEvaluationStatusMessage: "",
        strengthEvaluation: {},
        strengthEvaluationStatusMessage: "",
        climateEvaluation: {},
        climateEvaluationStatusMessage: "",
        usefulGodCandidates: {},
        usefulGodCandidatesStatusMessage: "",
        patternCandidates: {},
        patternCandidatesStatusMessage: "",
        majorFortunes: [],
        majorFortunesStatusMessage: "",
        annualFortunes: [],
        annualFortunesStatusMessage: "",
        majorFortuneDirection: {},
        majorFortuneDirectionStatusMessage: "",
        solarTermDifferencePreparation: {},
        solarTermDifferencePreparationStatusMessage: ""
    })
    property var interpretationResult: ({
        summaryText: "",
        detailText: "",
        cautionText: ""
    })
    property int currentYear: (new Date()).getFullYear()
    property int birthYearFromAnnualFortunes: {
        if (!chartResult.annualFortunes || chartResult.annualFortunes.length === undefined || chartResult.annualFortunes.length === 0) {
            return -1
        }

        const firstYear = chartResult.annualFortunes[0].year
        return firstYear === undefined || firstYear === null ? -1 : firstYear
    }
    property int currentAgeForFortunes: birthYearFromAnnualFortunes >= 0 ? currentYear - birthYearFromAnnualFortunes : -1
    property string saveMessage: ""
    property string exportMessage: ""

    function formatHiddenStems(value) {
        if (value === undefined || value === null) {
            return "未対応"
        }

        if (typeof value === "string") {
            return value.length > 0 ? value : "未対応"
        }

        if (value.length !== undefined) {
            return value.length > 0 ? value.join(" ") : "未対応"
        }

        return "未対応"
    }

    function formatFiveElementCount(key) {
        if (!chartResult.fiveElements || chartResult.fiveElements[key] === undefined || chartResult.fiveElements[key] === null) {
            return "0"
        }

        return chartResult.fiveElements[key]
    }

    ScrollView {
        id: scrollView
        anchors.fill: parent
        clip: true

        Column {
            width: scrollView.availableWidth
            spacing: 16

            Item {
                width: parent.width
                height: 24
            }

            Label {
                width: parent.width
                text: "命式結果"
                font.pixelSize: 24
                font.bold: true
            }

            Label {
                width: parent.width
                text: "仮表示"
                color: Kirigami.Theme.disabledTextColor
            }

            Frame {
                width: parent.width

                Column {
                    width: parent.width
                    spacing: 12

                    Label {
                        width: parent.width
                        text: "年柱: " + chartResult.yearPillar
                    }

                    Label {
                        width: parent.width
                        text: "月柱: " + (chartResult.monthPillar.length > 0 ? chartResult.monthPillar : "未対応")
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        visible: chartResult.monthPillarStatusMessage.length > 0
                        color: Kirigami.Theme.disabledTextColor
                        text: "月柱状態: " + chartResult.monthPillarStatusMessage
                    }

                    Label {
                        width: parent.width
                        text: "日柱: " + chartResult.dayPillar
                    }

                    Label {
                        width: parent.width
                        text: "時柱: " + chartResult.hourPillar
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: chartResult.description
                    }
                }
            }

            Frame {
                width: parent.width

                Column {
                    width: parent.width
                    spacing: 12

                    Label {
                        width: parent.width
                        text: "詳細命式"
                        font.bold: true
                    }

                    Label {
                        width: parent.width
                        text: "通変星(年柱): " + ((chartResult.tenGods && chartResult.tenGods.yearPillar) ? chartResult.tenGods.yearPillar : "未実装")
                    }

                    Label {
                        width: parent.width
                        text: "通変星(月柱): " + ((chartResult.tenGods && chartResult.tenGods.monthPillar) ? chartResult.tenGods.monthPillar : "未実装")
                    }

                    Label {
                        width: parent.width
                        text: "通変星(日柱): " + ((chartResult.tenGods && chartResult.tenGods.dayPillar) ? chartResult.tenGods.dayPillar : "未実装")
                    }

                    Label {
                        width: parent.width
                        text: "通変星(時柱): " + ((chartResult.tenGods && chartResult.tenGods.hourPillar) ? chartResult.tenGods.hourPillar : "未実装")
                    }

                    Label {
                        width: parent.width
                        text: "蔵干(年支): " + formatHiddenStems(chartResult.hiddenStems ? chartResult.hiddenStems.yearPillar : null)
                    }

                    Label {
                        width: parent.width
                        text: "蔵干(月支): " + formatHiddenStems(chartResult.hiddenStems ? chartResult.hiddenStems.monthPillar : null)
                    }

                    Label {
                        width: parent.width
                        text: "蔵干(日支): " + formatHiddenStems(chartResult.hiddenStems ? chartResult.hiddenStems.dayPillar : null)
                    }

                    Label {
                        width: parent.width
                        text: "蔵干(時支): " + formatHiddenStems(chartResult.hiddenStems ? chartResult.hiddenStems.hourPillar : null)
                    }

                    Label {
                        width: parent.width
                        text: "五行(木): " + formatFiveElementCount("wood")
                    }

                    Label {
                        width: parent.width
                        text: "五行(火): " + formatFiveElementCount("fire")
                    }

                    Label {
                        width: parent.width
                        text: "五行(土): " + formatFiveElementCount("earth")
                    }

                    Label {
                        width: parent.width
                        text: "五行(金): " + formatFiveElementCount("metal")
                    }

                    Label {
                        width: parent.width
                        text: "五行(水): " + formatFiveElementCount("water")
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        visible: chartResult.fiveElementDistributionStatusMessage.length > 0
                        color: Kirigami.Theme.disabledTextColor
                        text: "五行集計状態: " + chartResult.fiveElementDistributionStatusMessage
                    }

                    Label {
                        width: parent.width
                        text: "月支季節: " + ((chartResult.seasonalEvaluation && chartResult.seasonalEvaluation.season) ? chartResult.seasonalEvaluation.season : "未対応")
                    }

                    Label {
                        width: parent.width
                        text: "季節適性: " + ((chartResult.seasonalEvaluation && chartResult.seasonalEvaluation.suitability) ? chartResult.seasonalEvaluation.suitability : "未対応")
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        visible: chartResult.seasonalEvaluationStatusMessage.length > 0
                        color: Kirigami.Theme.disabledTextColor
                        text: "季節評価状態: " + chartResult.seasonalEvaluationStatusMessage
                    }

                    Label {
                        width: parent.width
                        text: "暫定強弱評価: " + ((chartResult.strengthEvaluation && chartResult.strengthEvaluation.label) ? chartResult.strengthEvaluation.label : "未対応")
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "暫定強弱理由: " + ((chartResult.strengthEvaluation && chartResult.strengthEvaluation.reason) ? chartResult.strengthEvaluation.reason : "未対応")
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        visible: chartResult.strengthEvaluationStatusMessage.length > 0
                        color: Kirigami.Theme.disabledTextColor
                        text: "暫定強弱状態: " + chartResult.strengthEvaluationStatusMessage
                    }

                    Label {
                        width: parent.width
                        text: "寒暖傾向: " + ((chartResult.climateEvaluation && chartResult.climateEvaluation.temperature) ? chartResult.climateEvaluation.temperature : "未対応")
                    }

                    Label {
                        width: parent.width
                        text: "乾湿傾向: " + ((chartResult.climateEvaluation && chartResult.climateEvaluation.moisture) ? chartResult.climateEvaluation.moisture : "未対応")
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "寒暖・乾湿メモ: " + ((chartResult.climateEvaluation && chartResult.climateEvaluation.note) ? chartResult.climateEvaluation.note : "未対応")
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        visible: chartResult.climateEvaluationStatusMessage.length > 0
                        color: Kirigami.Theme.disabledTextColor
                        text: "寒暖・乾湿状態: " + chartResult.climateEvaluationStatusMessage
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "用神候補: " + formatHiddenStems(chartResult.usefulGodCandidates ? chartResult.usefulGodCandidates.candidates : null)
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "用神候補理由: " + ((chartResult.usefulGodCandidates && chartResult.usefulGodCandidates.reason) ? chartResult.usefulGodCandidates.reason : "未対応")
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "用神候補注記: " + ((chartResult.usefulGodCandidates && chartResult.usefulGodCandidates.note) ? chartResult.usefulGodCandidates.note : "未対応")
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        visible: chartResult.usefulGodCandidatesStatusMessage.length > 0
                        color: Kirigami.Theme.disabledTextColor
                        text: "用神候補状態: " + chartResult.usefulGodCandidatesStatusMessage
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "格局候補: " + formatHiddenStems(chartResult.patternCandidates ? chartResult.patternCandidates.candidates : null)
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "格局候補理由: " + ((chartResult.patternCandidates && chartResult.patternCandidates.reason) ? chartResult.patternCandidates.reason : "未対応")
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "格局候補注記: " + ((chartResult.patternCandidates && chartResult.patternCandidates.note) ? chartResult.patternCandidates.note : "未対応")
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        visible: chartResult.patternCandidatesStatusMessage.length > 0
                        color: Kirigami.Theme.disabledTextColor
                        text: "格局候補状態: " + chartResult.patternCandidatesStatusMessage
                    }

                    Label {
                        width: parent.width
                        text: "大運一覧"
                        font.bold: true
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "順逆: " + ((chartResult.majorFortuneDirection && chartResult.majorFortuneDirection.direction)
                                           ? chartResult.majorFortuneDirection.direction : "未対応")
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "順逆理由: " + ((chartResult.majorFortuneDirection && chartResult.majorFortuneDirection.reason)
                                             ? chartResult.majorFortuneDirection.reason : "未対応")
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "順逆注記: " + ((chartResult.majorFortuneDirection && chartResult.majorFortuneDirection.note)
                                             ? chartResult.majorFortuneDirection.note : "未対応")
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        visible: chartResult.majorFortuneDirectionStatusMessage.length > 0
                        color: Kirigami.Theme.disabledTextColor
                        text: "順逆状態: " + chartResult.majorFortuneDirectionStatusMessage
                    }

                    Label {
                        width: parent.width
                        text: "起運年齢統合計算"
                        font.bold: true
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "出生日時: " + ((chartResult.solarTermDifferencePreparation && chartResult.solarTermDifferencePreparation.birthDateTime)
                                           ? chartResult.solarTermDifferencePreparation.birthDateTime : "未対応")
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "参照節入り: "
                              + ((chartResult.solarTermDifferencePreparation && chartResult.solarTermDifferencePreparation.referenceTerm)
                                 ? chartResult.solarTermDifferencePreparation.referenceTerm : "未対応")
                              + " / "
                              + ((chartResult.solarTermDifferencePreparation && chartResult.solarTermDifferencePreparation.referenceDirection)
                                 ? chartResult.solarTermDifferencePreparation.referenceDirection : "未対応")
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "節入り日時差(日): " + ((chartResult.solarTermDifferencePreparation && chartResult.solarTermDifferencePreparation.differenceDays)
                                                ? chartResult.solarTermDifferencePreparation.differenceDays : "未対応")
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "起運年齢: " + ((chartResult.solarTermDifferencePreparation && chartResult.solarTermDifferencePreparation.calculatedStartAge !== undefined
                                                && chartResult.solarTermDifferencePreparation.calculatedStartAge !== null
                                                && chartResult.solarTermDifferencePreparation.calculatedStartAge >= 0)
                                                ? chartResult.solarTermDifferencePreparation.calculatedStartAge + "歳"
                                                : "未対応")
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: "節入り差注記: " + ((chartResult.solarTermDifferencePreparation && chartResult.solarTermDifferencePreparation.note)
                                              ? chartResult.solarTermDifferencePreparation.note : "未対応")
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        visible: chartResult.solarTermDifferencePreparationStatusMessage.length > 0
                        color: Kirigami.Theme.disabledTextColor
                        text: "統合計算状態: " + chartResult.solarTermDifferencePreparationStatusMessage
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        color: Kirigami.Theme.disabledTextColor
                        text: "開始年齢帯は節入り差と順逆実判定を使った採用仕様の計算結果です。大運干支は月柱起点の順逆反映済みです。"
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        visible: currentAgeForFortunes >= 0
                        color: Kirigami.Theme.disabledTextColor
                        text: "現在の目安: " + currentYear + "年 / " + currentAgeForFortunes + "歳前後"
                    }

                    Repeater {
                        model: chartResult.majorFortunes ? chartResult.majorFortunes : []

                        delegate: Rectangle {
                            required property var modelData
                            property bool isCurrentMajorFortune: currentAgeForFortunes >= 0
                                && modelData.startAge !== undefined
                                && modelData.startAge !== null
                                && modelData.endAge !== undefined
                                && modelData.endAge !== null
                                && currentAgeForFortunes >= modelData.startAge
                                && currentAgeForFortunes <= modelData.endAge
                            width: parent.width
                            radius: 6
                            color: isCurrentMajorFortune ? Kirigami.Theme.highlightColor : Kirigami.Theme.alternateBackgroundColor
                            border.width: isCurrentMajorFortune ? 2 : 0
                            border.color: Kirigami.Theme.highlightColor
                            implicitHeight: majorFortuneColumn.implicitHeight + 16

                            Column {
                                id: majorFortuneColumn
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 4

                                Row {
                                    width: parent.width
                                    spacing: 8

                                    Label {
                                        text: (modelData.label ? modelData.label : "未対応")
                                        font.bold: true
                                        color: isCurrentMajorFortune
                                            ? Kirigami.Theme.highlightedTextColor
                                            : Kirigami.Theme.textColor
                                    }

                                    Label {
                                        visible: isCurrentMajorFortune
                                        text: "現在"
                                        font.bold: true
                                        color: isCurrentMajorFortune
                                            ? Kirigami.Theme.highlightedTextColor
                                            : Kirigami.Theme.highlightColor
                                    }
                                }

                                Label {
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                    text: "大運: " + (modelData.pillar ? modelData.pillar : "未対応")
                                    color: isCurrentMajorFortune
                                        ? Kirigami.Theme.highlightedTextColor
                                        : Kirigami.Theme.textColor
                                }

                                Label {
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                    text: "通変星: "
                                          + (modelData.tenGod ? modelData.tenGod : "未対応")
                                          + " / 十二運: "
                                          + (modelData.twelvePhase ? modelData.twelvePhase : "未対応")
                                    color: isCurrentMajorFortune
                                        ? Kirigami.Theme.highlightedTextColor
                                        : Kirigami.Theme.textColor
                                }

                                Label {
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                    text: modelData.note ? modelData.note : ""
                                    color: isCurrentMajorFortune
                                        ? Kirigami.Theme.highlightedTextColor
                                        : Kirigami.Theme.disabledTextColor
                                }
                            }
                        }
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        visible: chartResult.majorFortunesStatusMessage.length > 0
                        color: Kirigami.Theme.disabledTextColor
                        text: "大運状態: " + chartResult.majorFortunesStatusMessage
                    }

                    Label {
                        width: parent.width
                        text: "流年一覧"
                        font.bold: true
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        color: Kirigami.Theme.disabledTextColor
                        text: "現在年 " + currentYear + " に一致する流年を強調表示しています。"
                    }

                    Repeater {
                        model: chartResult.annualFortunes ? chartResult.annualFortunes : []

                        delegate: Rectangle {
                            required property var modelData
                            property bool isCurrentAnnualFortune: modelData.year === currentYear
                            width: parent.width
                            radius: 6
                            color: isCurrentAnnualFortune ? Kirigami.Theme.highlightColor : Kirigami.Theme.alternateBackgroundColor
                            border.width: isCurrentAnnualFortune ? 2 : 0
                            border.color: Kirigami.Theme.highlightColor
                            implicitHeight: annualFortuneColumn.implicitHeight + 16

                            Column {
                                id: annualFortuneColumn
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 4

                                Row {
                                    width: parent.width
                                    spacing: 8

                                    Label {
                                        text: modelData.year ? modelData.year + "年" : "未対応"
                                        font.bold: true
                                        color: isCurrentAnnualFortune
                                            ? Kirigami.Theme.highlightedTextColor
                                            : Kirigami.Theme.textColor
                                    }

                                    Label {
                                        visible: isCurrentAnnualFortune
                                        text: "現在"
                                        font.bold: true
                                        color: isCurrentAnnualFortune
                                            ? Kirigami.Theme.highlightedTextColor
                                            : Kirigami.Theme.highlightColor
                                    }
                                }

                                Label {
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                    text: "流年: " + (modelData.pillar ? modelData.pillar : "未対応")
                                    color: isCurrentAnnualFortune
                                        ? Kirigami.Theme.highlightedTextColor
                                        : Kirigami.Theme.textColor
                                }

                                Label {
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                    text: "通変星: "
                                          + (modelData.tenGod ? modelData.tenGod : "未対応")
                                          + " / 十二運: "
                                          + (modelData.twelvePhase ? modelData.twelvePhase : "未対応")
                                    color: isCurrentAnnualFortune
                                        ? Kirigami.Theme.highlightedTextColor
                                        : Kirigami.Theme.textColor
                                }

                                Label {
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                    text: "関係: " + (modelData.relationSummary ? modelData.relationSummary : "未対応")
                                    color: isCurrentAnnualFortune
                                        ? Kirigami.Theme.highlightedTextColor
                                        : Kirigami.Theme.textColor
                                }

                                Label {
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                    text: modelData.relationNote ? modelData.relationNote : ""
                                    color: isCurrentAnnualFortune
                                        ? Kirigami.Theme.highlightedTextColor
                                        : Kirigami.Theme.disabledTextColor
                                }

                                Label {
                                    width: parent.width
                                    wrapMode: Text.WordWrap
                                    text: modelData.note ? modelData.note : ""
                                    color: isCurrentAnnualFortune
                                        ? Kirigami.Theme.highlightedTextColor
                                        : Kirigami.Theme.disabledTextColor
                                }
                            }
                        }
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        visible: chartResult.annualFortunesStatusMessage.length > 0
                        color: Kirigami.Theme.disabledTextColor
                        text: "流年状態: " + chartResult.annualFortunesStatusMessage
                    }
                }
            }

            Frame {
                width: parent.width

                Column {
                    width: parent.width
                    spacing: 12

                    Label {
                        width: parent.width
                        text: "解釈メモ"
                        font.bold: true
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: interpretationResult.summaryText
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        text: interpretationResult.detailText
                    }

                    Label {
                        width: parent.width
                        wrapMode: Text.WordWrap
                        color: Kirigami.Theme.disabledTextColor
                        text: interpretationResult.cautionText
                    }
                }
            }

            Button {
                width: parent.width
                text: "保存"
                onClicked: {
                    appController.saveCurrentRecord()
                    saveMessage = appController.lastSaveMessage()
                }
            }

            Row {
                width: parent.width
                spacing: 12

                Button {
                    text: "テキスト出力"
                    onClicked: {
                        appController.exportCurrentRecordAsText()
                        exportMessage = appController.lastExportMessage()
                    }
                }

                Button {
                    text: "JSON出力"
                    onClicked: {
                        appController.exportCurrentRecordAsJson()
                        exportMessage = appController.lastExportMessage()
                    }
                }
            }

            Label {
                width: parent.width
                visible: saveMessage.length > 0
                wrapMode: Text.WordWrap
                text: saveMessage
            }

            Label {
                width: parent.width
                visible: exportMessage.length > 0
                wrapMode: Text.WordWrap
                text: exportMessage
            }

            Button {
                width: parent.width
                text: "入力画面へ戻る"
                onClicked: applicationWindow().pageStack.pop()
            }

            Item {
                width: parent.width
                height: 24
            }
        }
    }
}
