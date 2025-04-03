import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
// import Bgs 1.0
import Protogeneratorqt 1.0

Frame {
    id: frame

    contentItem: ColumnLayout {
        ColumnLayout {
            RowLayout {
                Label {
                    Layout.fillWidth: true
                    text: "Integer"
                }
                TextField {
                    id: iEdit
                    text: testData.integerField
                    onEditingFinished: {
                        testData.integerField = parseInt(text);
                    }
                }
            }
        }
        ColumnLayout {
            RowLayout {
                Label {
                    Layout.fillWidth: true
                    text: "String"
                }
                TextField {
                    id: sEdit
                    text: testData.stringField
                    onEditingFinished: {
                        testData.stringField = text;
                    }
                }
            }
        }

        ColumnLayout {
            RowLayout {
                Label {
                    Layout.fillWidth: true
                    text: "IpEndpoint"
                }
                RowLayout {
                    TextField {
                        id: ipEndpointAddressEdit
                        text: testData.ipEndpointField.address
                        onEditingFinished: {
                            testData.ipEndpointField.address = text;
                        }
                    }
                    TextField {
                        id: ipEndpointPortEdit
                        text: testData.ipEndpointField.port
                        Layout.preferredWidth: 50
                        onEditingFinished: {
                            testData.ipEndpointField.port = parseInt(text);
                        }
                    }
                    // BgsLabeledTextField {
                    //     id: ipEndpointAddressEdit
                    //     text: testData.ipEndpointField.address
                    //     onEditingFinished: {
                    //         testData.ipEndpointField.address = text;
                    //     }
                    // }
                    // BgsLabeledTextField {
                    //     id: ipEndpointPortEdit
                    //     text: testData.ipEndpointField.port
                    //     Layout.preferredWidth: 50
                    //     onEditingFinished: {
                    //         testData.ipEndpointField.port = parseInt(text);
                    //     }
                    // }
                }
            }
        }
        ColumnLayout {
            RowLayout {
                // ComboBox {
                //     // labelText: "Enum"
                //     Layout.fillWidth: true
                //     textRole: "text"
                //     valueRole: "value"
                //
                //     model: ListModel {
                //         id: possibleStates
                //         ListElement { text: "Option 1"; value: TestEnumEnum.OPTION_1 }
                //         ListElement { text: "Option 2"; value: TestEnumEnum.OPTION_2 }
                //     }
                //     currentIndex: indexOf(testData.enumField)
                //     onActivated: testData.enumField = selectedValue
                // }
                ComboBox {
                    Layout.fillWidth: true
                    textRole: "text"
                    valueRole: "value"

                    model: ListModel {
                        id: possibleStates
                        ListElement { text: "Option 1"; value: TestEnumEnum.OPTION_1 }
                        ListElement { text: "Option 2"; value: TestEnumEnum.OPTION_2 }
                    }

                    // Устанавливаем currentIndex, если testData.enumField уже содержит значение
                    Component.onCompleted: {
                        currentIndex = find(testData.enumField);
                    }

                    onActivated: (index) => {
                        testData.enumField = model.get(index).value;
                    }
                }

            }
        }
    }
}