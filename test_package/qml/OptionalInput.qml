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
                CheckBox {
                    id: optionalIntegerBox
                    text: "Optional Integer"
                    checked: testData.hasOptionalInteger
                }
                TextField {
                    id: integerEdit
                    enabled: optionalIntegerBox.checked
                    text: testData.hasOptionalInteger ? testData.optionalInteger : ""
                    onEditingFinished: {
                        testData.optionalInteger = parseInt(text);
                    }
                }
            }
        }

        ColumnLayout {
            RowLayout {
                CheckBox {
                    id: optionalStringBox
                    text: "Optional String"
                    checked: testData.hasOptionalString
                    onCheckedChanged: {
                        testData.hasOptionalString = checked;
                    }
                }
                TextField {
                    id: stringEdit
                    enabled: optionalStringBox.checked
                    text: testData.optionalString
                    onEditingFinished: {
                        testData.optionalString = text;
                    }
                }
            }
        }

        ColumnLayout {
            RowLayout {
                CheckBox {
                    id: optionalIpEndpointBox
                    text: "Optional IpEndpoint"
                    checked: testData.hasOptionalIpEndpoint
                    onCheckedChanged: {
                        testData.hasOptionalIpEndpoint = checked;
                    }
                }
                RowLayout {
                    enabled: optionalIpEndpointBox.checked
                    TextField {
                        id: ipEndpointAddressEdit
                        text: testData.hasOptionalIpEndpoint ? testData.optionalIpEndpoint.address : ""
                        onEditingFinished: {
                            if (!testData.optionalIpEndpoint.hasOptionalIpEndpoint) {
                                testData.optionalIpEndpoint.hasOptionalIpEndpoint = true;
                            }
                            testData.optionalIpEndpoint.address = text;
                        }
                    }

                    TextField {
                        id: ipEndpointPortEdit
                        text: testData.hasOptionalIpEndpoint ? testData.optionalIpEndpoint.port: ""
                        Layout.preferredWidth: 50
                        onEditingFinished: {
                            if (!testData.optionalIpEndpoint.hasOptionalIpEndpoint) {
                                testData.optionalIpEndpoint.hasOptionalIpEndpoint = true;
                            }
                            testData.optionalIpEndpoint.port = parseInt(text);
                        }
                    }
                }
            }
        }

        ColumnLayout {
            RowLayout {
                CheckBox {
                    id: optionalEnumBox
                    text: "Optional Enum"
                    checked: testData.hasOptionalEnum
                    onCheckedChanged: {
                        testData.hasOptionalEnum = checked;
                    }
                }
                ComboBox {
                    id: enumEdit
                    Layout.fillWidth: true
                    textRole: "text"
                    valueRole: "value"
                    enabled: optionalEnumBox.checked

                    model: ListModel {
                        id: possibleStates
                        ListElement { text: "Option 1"; value: TestEnumEnum.OPTION_1 }
                        ListElement { text: "Option 2"; value: TestEnumEnum.OPTION_2 }
                    }
                    Component.onCompleted: {
                        currentIndex = find(testData.optionalEnum);
                    }

                    onActivated: (index) => {
                        testData.optionalEnum = model.get(index).value;
                    }
                }
            }
        }
    }
}