import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami

Kirigami.Page {
    title: "命式入力"

    property var chartResult: ({})

    Column {
        anchors.centerIn: parent
        spacing: 12
        width: 320

        TextField {
            placeholderText: "生年月日 例: 1990-01-01"
        }

        TextField {
            placeholderText: "出生時刻 例: 13:30"
        }

        ComboBox {
            model: ["男性", "女性", "未指定"]
        }

        Button {
            text: "計算"
            onClicked: {
                chartResult = appController.mockChartResult()
                applicationWindow().pageStack.push(
                    Qt.resolvedUrl("ChartResultPage.qml"),
                    { chartResult: chartResult }
                )
            }
        }
    }
}
