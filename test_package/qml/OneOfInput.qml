import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15
// import Bgs 1.0
import Protogeneratorqt 1.0

Frame {
    id: frame

    contentItem: ColumnLayout {
        spacing: 10
        RowLayout {
            ComboBox {
                id: stateBox
                textRole: "text"
                valueRole: "value"
                model: ListModel {
                    id: possibleStates
                    ListElement { text: "State Integer"; value: TestObject.STATE_INTEGER }
                    ListElement { text: "State String"; value: TestObject.STATE_STRING }
                    ListElement { text: "State IpEndpoint"; value: TestObject.STATE_IP_ENDPOINT }
                    ListElement { text: "State Enum"; value: TestObject.STATE_ENUM }
                    ListElement { text: "State not set"; value: TestObject.STATE_NOT_SET }
                }
                Component.onCompleted: {
                    currentIndex = find(testData.stateCase);
                }
                onActivated: (index) => {
                    testData.stateCase = model.get(index).value;
                }
            }
        }


        Loader {
            Component {
                id: stateIntegerComponent
                RowLayout {
                    Label {
                        id: stateIntegerNameLabel
                        text: "One of: state Integer"
                    }
                    TextField {
                        id: integerEdit
                        text: testData.stateInteger
                        enabled: testData.stateCase === TestObject.STATE_INTEGER
                        onEditingFinished: {
                            testData.stateInteger = parseInt(text);
                        }
                    }
                }
            }

            Component {
                id: stateStringComponent
                RowLayout {
                    Label {
                        id: stateStringNameLabel
                        text: "One of: state String"
                    }
                    TextField {
                        id: sEdit
                        text: testData.stateString
                        enabled: testData.stateCase === TestObject.STATE_STRING
                        onEditingFinished: {
                            testData.stateString = text;
                        }
                    }
                }
            }

            Component {
                id: stateIpEndpointComponent
                RowLayout {
                    Label {
                        id: stateIpEndpointNameLabel
                        text: "One of: state IpEndpoint"
                    }
                    RowLayout {
                        enabled: testData.stateCase === TestObject.STATE_IP_ENDPOINT
                        TextField {
                            id: ipEndpointAddressEdit
                            text: testData.stateIpEndpoint.address
                            onEditingFinished: {
                                testData.stateIpEndpoint.address = text;
                            }
                        }
                        TextField {
                            id: ipEndpointPortEdit
                            text: testData.stateIpEndpoint.port
                            Layout.preferredWidth: 50
                            onEditingFinished: {
                                testData.stateIpEndpoint.port = parseInt(text);
                            }
                        }
                    }
                }
            }

            Component {
                id: stateEnumComponent
                ComboBox {
                    id: stateEnumEdit
                    // labelText: "State Enum"
                    Layout.fillWidth: true
                    textRole: "text"
                    valueRole: "value"

                    model: ListModel {
                        id: possibleStates
                        ListElement { text: "Option 1"; value: TestEnumEnum.OPTION_1 }
                        ListElement { text: "Option 2"; value: TestEnumEnum.OPTION_2 }
                    }
                    // Устанавливаем индекс при загрузке
                    Component.onCompleted: {
                        currentIndex = find(testData.stateEnum);
                    }

                    // Изменяем значение testData.optionalEnum при выборе элемента
                    onActivated: (index) => {
                        testData.stateEnum = model.get(index).value;
                    }
                }
            }

            Component {
                id: stateNotSetComponent
                Label {
                    text: "State not set"
                }
            }

            sourceComponent: {
                switch (testData.stateCase) {
                    case TestObject.STATE_INTEGER:
                        return stateIntegerComponent
                    case TestObject.STATE_STRING:
                        return stateStringComponent
                    case TestObject.STATE_IP_ENDPOINT:
                        return stateIpEndpointComponent
                    case TestObject.STATE_ENUM:
                        return stateEnumComponent
                    case TestObject.STATE_NOT_SET:
                        return stateNotSetComponent
                    default:
                        return undefined
                }
            }
        }
    }
}