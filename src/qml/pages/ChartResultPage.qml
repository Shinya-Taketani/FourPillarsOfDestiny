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

        Button {
            text: "入力画面へ戻る"
            onClicked: applicationWindow().pageStack.pop()
        }
    }
}
