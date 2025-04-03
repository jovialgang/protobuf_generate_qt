import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
// import Bgs 1.0
// import ObjectModel 1.0
import Om 1.0
import Protogeneratorqt 1.0

Frame {
    id: frame

    Layout.fillWidth: true

    Component {
        id: tableIpEndpointItemComponent
        IpEndpointObject {
            
        }
    }

    contentItem: RowLayout {
        ComboBox {
            id: tableBox
            textRole: "text"
            valueRole: "value"
            model: ListModel {
                id: possibleStates
                ListElement { text: "IpEndpoint"; value: 0 }
                ListElement { text: "String"; value: 1 }
                ListElement { text: "Enum"; value: 2 }
            }

            Component.onCompleted: {
                currentIndex = 0;
            }
        }

        Loader {
            Layout.fillWidth: true

            Component {
                id: repeatedIpEndpointTableAdd

                RowLayout {
                    Layout.fillWidth: true
                    
                    RowLayout {
                        Layout.alignment: Qt.AlignLeft
                        Label {
                            text: "Address"
                        }
                        TextField {
                            id: repeatedIpEndpointAddress
                        }

                        Label {
                            text: "Port"
                        }
                        TextField {
                            id: repeatedIpEndpointPort
                        }
                    }

                    Button {
                        Layout.alignment: Qt.AlignRight
                        text: "Add"

                        onClicked: {
                            let obj = tableIpEndpointItemComponent.createObject(frame.contentItem, { address: repeatedIpEndpointAddress.text, port: repeatedIpEndpointPort.text });
                            testData.repeatedIpEndpoint.Append(obj);
                            repeatedIpEndpointAddress.text = ""
                            repeatedIpEndpointPort.text = ""
                        }
                    }
                }
            }

            Component {
                id: repeatedStringTableAdd

                RowLayout {
                    Layout.fillWidth: true

                    RowLayout {
                        Layout.alignment: Qt.AlignLeft
                        Label {
                            text: "String"
                        }
                        TextField {
                            id: repeatedStringText
                        }
                    }
                    
                    Button {
                        Layout.alignment: Qt.AlignRight
                        text: "Add"

                        onClicked: {
                            testData.repeatedString.Append(repeatedStringText.text);
                            repeatedStringText.text = "";
                        }
                    }
                }
            }

            Component {
                id: repeatedEnumTableAdd

                RowLayout {
                    Layout.fillWidth: true

                    // RowLayout {
                    //     Layout.alignment: Qt.AlignLeft
                    //     Label {
                    //         text: "Integer"
                    //     }
                    //     TextField {
                    //         id: repeatedEnumText
                    //     }
                    // }
                    ComboBox {
                        id: repeatedEnumComboBox
                        Layout.alignment: Qt.AlignLeft
                        //Layout.fillWidth: true
                        textRole: "text"
                        valueRole: "value"

                        model: ListModel {
                            id: possibleStates
                            ListElement { text: "Option 1"; value: TestEnumEnum.OPTION_1 }
                            ListElement { text: "Option 2"; value: TestEnumEnum.OPTION_2 }
                        }
                    }
                    
                    Button {
                        Layout.alignment: Qt.AlignRight
                        text: "Add"

                        onClicked: {
                            testData.repeatedEnum.Append(parseInt(repeatedEnumComboBox.currentIndex));
                        }
                    }
                }
            }

            sourceComponent: {
                switch(tableBox.currentIndex) {
                    case 0: {
                        return repeatedIpEndpointTableAdd;
                    }
                    case 1: {
                        return repeatedStringTableAdd;
                    }
                    case 2: {
                        return repeatedEnumTableAdd;
                    }
                    default: {
                        return undefined;
                    }
                }
            }
        }
    }
}
