import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
// import SortFilterProxyModel 1.0
// import ObjectModel 1.0
import Om 1.0


Frame {
    id: frame

    contentItem: ColumnLayout {
        TableAddItem {
            Layout.fillWidth: true
        }



        Control {
            id: listViewControl
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentItem: RowLayout {
                id: rowContent
                Layout.fillWidth: true
                // Layout.preferredHeight: 400

                ListView {
                    id: repeatedIpEndpointTable
                    clip: true
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ScrollBar.vertical: ScrollBar {}

                    model: testData.repeatedIpEndpoint

                    delegate: Frame {
                        Layout.topMargin: -1
                        width: repeatedIpEndpointTable.width
                        contentItem: RowLayout {
                            Label {
                                text: modelData.address
                            }
                            Label {
                                text: modelData.port
                            }

                            Button {
                                Layout.alignment: Qt.AlignRight
                                Layout.margins: 5
                                //icon.name: "close"
                                text: "close"

                                ToolTip.text: qsTr("Remove item")
                                ToolTip.visible: hovered

                                onClicked: testData.repeatedIpEndpoint.Remove(index);
                            }
                        }
                    }
                }

                ListView {
                    id: repeatedStringTable
	                clip: true
                    Layout.fillWidth: true
                    Layout.fillHeight: true

	                ScrollBar.vertical: ScrollBar {}

                    model: testData.repeatedString

                    delegate: Frame {
                        Layout.topMargin: -1
                        width: repeatedStringTable.width
                        contentItem: RowLayout {                            
                            Label {
                                text: modelData
                            }

                            Button {
                                Layout.alignment: Qt.AlignRight
                                Layout.margins: 5
                                //icon.name: "close"
                                text: "close"

                                ToolTip.text: qsTr("Remove item")
                                ToolTip.visible: hovered

                                onClicked: testData.repeatedString.Remove(index);
                            }
                        }
                    }
                }
                ListView {
                    id: repeatedEnumTable
	                clip: true
                    Layout.fillWidth: true
                    Layout.fillHeight: true

	                ScrollBar.vertical: ScrollBar {}

                    model: testData.repeatedEnum

                    delegate: Frame {
                        Layout.topMargin: -1
                        width: repeatedStringTable.width
                        contentItem: RowLayout {
                            Label {
                                text: modelData
                            }
                            Button {
                                Layout.alignment: Qt.AlignRight
                                Layout.margins: 5
                                text: "close"

                                ToolTip.text: qsTr("Remove item")
                                ToolTip.visible: hovered

                                onClicked: testData.repeatedEnum.Remove(index);
                            }
                        }
                    }
                }
            }
        }
    }
}

