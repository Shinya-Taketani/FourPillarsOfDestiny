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
        seasonalEvaluationStatusMessage: ""
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

    Column {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        Label {
            text: "命式結果"
            font.pixelSize: 24
            font.bold: true
        }

        Label {
            text: "仮表示"
            color: Kirigami.Theme.disabledTextColor
        }

        Frame {
            width: parent.width

            Column {
                width: parent.width
                spacing: 12

                Label {
                    text: "年柱: " + chartResult.yearPillar
                }

                Label {
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
                    text: "日柱: " + chartResult.dayPillar
                }

                Label {
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
                    text: "詳細命式"
                    font.bold: true
                }

                Label {
                    text: "通変星(年柱): " + ((chartResult.tenGods && chartResult.tenGods.yearPillar) ? chartResult.tenGods.yearPillar : "未実装")
                }

                Label {
                    text: "通変星(月柱): " + ((chartResult.tenGods && chartResult.tenGods.monthPillar) ? chartResult.tenGods.monthPillar : "未実装")
                }

                Label {
                    text: "通変星(日柱): " + ((chartResult.tenGods && chartResult.tenGods.dayPillar) ? chartResult.tenGods.dayPillar : "未実装")
                }

                Label {
                    text: "通変星(時柱): " + ((chartResult.tenGods && chartResult.tenGods.hourPillar) ? chartResult.tenGods.hourPillar : "未実装")
                }

                Label {
                    text: "蔵干(年支): " + formatHiddenStems(chartResult.hiddenStems ? chartResult.hiddenStems.yearPillar : null)
                }

                Label {
                    text: "蔵干(月支): " + formatHiddenStems(chartResult.hiddenStems ? chartResult.hiddenStems.monthPillar : null)
                }

                Label {
                    text: "蔵干(日支): " + formatHiddenStems(chartResult.hiddenStems ? chartResult.hiddenStems.dayPillar : null)
                }

                Label {
                    text: "蔵干(時支): " + formatHiddenStems(chartResult.hiddenStems ? chartResult.hiddenStems.hourPillar : null)
                }

                Label {
                    text: "五行(木): " + formatFiveElementCount("wood")
                }

                Label {
                    text: "五行(火): " + formatFiveElementCount("fire")
                }

                Label {
                    text: "五行(土): " + formatFiveElementCount("earth")
                }

                Label {
                    text: "五行(金): " + formatFiveElementCount("metal")
                }

                Label {
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
                    text: "月支季節: " + ((chartResult.seasonalEvaluation && chartResult.seasonalEvaluation.season) ? chartResult.seasonalEvaluation.season : "未対応")
                }

                Label {
                    text: "季節適性: " + ((chartResult.seasonalEvaluation && chartResult.seasonalEvaluation.suitability) ? chartResult.seasonalEvaluation.suitability : "未対応")
                }

                Label {
                    width: parent.width
                    wrapMode: Text.WordWrap
                    visible: chartResult.seasonalEvaluationStatusMessage.length > 0
                    color: Kirigami.Theme.disabledTextColor
                    text: "季節評価状態: " + chartResult.seasonalEvaluationStatusMessage
                }
            }
        }

        Frame {
            width: parent.width

            Column {
                width: parent.width
                spacing: 12

                Label {
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
            text: "保存"
            onClicked: {
                appController.saveCurrentRecord()
                saveMessage = appController.lastSaveMessage()
            }
        }

        Row {
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
            text: "入力画面へ戻る"
            onClicked: applicationWindow().pageStack.pop()
        }
    }
}
