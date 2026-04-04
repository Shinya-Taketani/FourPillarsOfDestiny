import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami

Kirigami.Page {
    title: "ホーム"

    Column {
        anchors.centerIn: parent
        spacing: 16

        Label {
            text: appController.helloMessage()
        }

        Button {
            text: "命式入力へ"
            onClicked: applicationWindow().pageStack.push(Qt.resolvedUrl("InputPage.qml"))
        }
    }
}
