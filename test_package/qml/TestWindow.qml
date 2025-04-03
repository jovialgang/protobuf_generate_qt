import QtQuick 2.11
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.3
import QtQuick.Layouts 1.3
import Qt.labs.settings 1.0
import FSysQmlUtils 1.0

ApplicationWindow {
    id: mainWindow
    objectName: "mainWindow"
    width: 1000
    height: 600
    minimumWidth: 200
    minimumHeight: 100
    visible: true

    onClosing: {
        Qt.quit();
    }
    Component.onCompleted: {
        print("qrc:/qml/TestWindow.qml Loaded")
    }

    // Диалог сохранения файла
    FileDialog {
        id: saveDialog
        title: "Save File"
        selectExisting: false  // Разрешает вводить новое имя файла
        nameFilters: ["Text Files (*.txt)", "All Files (*)"]
        onAccepted: {
            if (fileUrl !== "") {
                qmlFileSystem.Save(testData.Serialize(), fileUrl.toString())
                print("Saved to:", fileUrl)
            }
        }
    }

    // Диалог загрузки файла
    FileDialog {
        id: loadDialog
        title: "Load File"
        selectExisting: true  // Разрешает выбирать только существующие файлы
        nameFilters: ["Text Files (*.txt)", "All Files (*)"]
        onAccepted: {
            if (fileUrl !== "") {
                testData.Parse(qmlFileSystem.Load(fileUrl.toString()))
                print("Loaded from:", fileUrl)
            }
        }
    }

    menuBar: MenuBar {
        id: menuBar
        Menu {
            title: "Settings"

            MenuItem {
                text: "Save"
                onTriggered: saveDialog.open()
            }

            MenuItem {
                text: "Load"
                onTriggered: loadDialog.open()
            }
        }
    }


    ColumnLayout {
        anchors.fill: parent

        Item {
            implicitHeight: 250
            Layout.fillWidth: true
            Layout.margins: 10

            RowLayout {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right

                SingleInput {
                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }

                OptionalInput {
                    Layout.alignment: Qt.AlignTop
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }

                OneOfInput {
                    Layout.alignment: Qt.AlignTop | Qt.AlignRight
                    //Layout.preferredWidth: 350
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }
            }
        }

        RepeatedTable {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
