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
            id: birthDateField
            placeholderText: "生年月日 例: 1990-01-01"
        }

        TextField {
            id: birthTimeField
            placeholderText: "出生時刻 例: 13:30"
        }

        ComboBox {
            id: genderComboBox
            model: ["男性", "女性", "未指定"]
        }

        Button {
            text: "計算"
            onClicked: {
                appController.setBirthInfo(
                    birthDateField.text,
                    birthTimeField.text,
                    genderComboBox.currentText
                )
                chartResult = appController.mockChartResult()
                applicationWindow().pageStack.push(
                    Qt.resolvedUrl("ChartResultPage.qml"),
                    { chartResult: chartResult }
                )
            }
        }
    }
}
