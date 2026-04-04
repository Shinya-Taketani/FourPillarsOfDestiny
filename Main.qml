import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: root
    width: 1000
    height: 700
    visible: true
    title: appController.appTitle

    pageStack.initialPage: Qt.resolvedUrl("pages/HomePage.qml")
}
