import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami

Kirigami.Page {
    title: "保存済み一覧"

    property var records: []
    property string statusMessage: ""

    function refreshRecords() {
        records = appController.savedRecords()
        if (records.length === 0) {
            statusMessage = "保存済みデータはまだありません。"
        } else {
            statusMessage = ""
        }
    }

    Component.onCompleted: refreshRecords()

    Column {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 16

        Button {
            text: "一覧を更新"
            onClicked: refreshRecords()
        }

        Label {
            visible: statusMessage.length > 0
            width: parent.width
            wrapMode: Text.WordWrap
            text: statusMessage
        }

        ScrollView {
            width: parent.width
            height: parent.height - 120

            Column {
                width: parent.width
                spacing: 12

                Repeater {
                    model: records

                    delegate: Frame {
                        required property var modelData
                        width: parent.width

                        Column {
                            width: parent.width
                            spacing: 8

                            Label {
                                text: "保存日時: " + (modelData.savedAt || "")
                            }

                            Label {
                                text: "生年月日: " + (modelData.birthDate || "")
                            }

                            Label {
                                text: "性別: " + (modelData.gender || "")
                            }

                            Label {
                                width: parent.width
                                wrapMode: Text.WordWrap
                                text: "ファイル: " + (modelData.fileName || "")
                            }

                            Button {
                                text: "この内容を表示"
                                onClicked: {
                                    if (appController.loadSavedRecord(modelData.filePath)) {
                                        applicationWindow().pageStack.push(
                                            Qt.resolvedUrl("ChartResultPage.qml"),
                                            {
                                                chartResult: appController.currentChartResult(),
                                                interpretationResult: appController.currentInterpretationResult()
                                            }
                                        )
                                    } else {
                                        statusMessage = appController.lastSaveMessage()
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
