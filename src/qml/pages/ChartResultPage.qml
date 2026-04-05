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
        usefulGodCandidatesStatusMessage: ""
    })
    property var interpretationResult: ({
        summaryText: "",
        detailText: "",
        cautionText: ""
    })
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
