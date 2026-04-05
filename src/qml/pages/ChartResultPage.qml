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
        description: ""
    })
    property var interpretationResult: ({
        summaryText: "",
        detailText: "",
        cautionText: ""
    })
    property string saveMessage: ""
    property string exportMessage: ""

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
                    text: "月柱: " + chartResult.monthPillar
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
