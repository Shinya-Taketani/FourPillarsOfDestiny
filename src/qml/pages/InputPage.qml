import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami

Kirigami.Page {
    title: "命式入力"

    property var chartResult: ({})
    property string validationMessage: ""

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

        Label {
            width: parent.width
            visible: validationMessage.length > 0
            wrapMode: Text.WordWrap
            color: Kirigami.Theme.negativeTextColor
            text: validationMessage
        }

        Button {
            text: "計算"
            onClicked: {
                appController.setBirthInfo(
                    birthDateField.text,
                    birthTimeField.text,
                    genderComboBox.currentText
                )

                if (!appController.isBirthInfoValid()) {
                    validationMessage = appController.birthInfoValidationErrors().join("\n")
                    return
                }

                validationMessage = ""
                chartResult = appController.calculateChartResult()
                applicationWindow().pageStack.push(
                    Qt.resolvedUrl("ChartResultPage.qml"),
                    { chartResult: chartResult }
                )
            }
        }
    }
}
