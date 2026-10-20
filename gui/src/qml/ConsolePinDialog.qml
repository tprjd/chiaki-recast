import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import org.streetpea.chiaking

import "controls" as C

DialogView {
    required property var submitPin
    required property var cancelPin
    property bool completed: false
    title: qsTr("Set console pin")
    buttonText: qsTr("Set")
    buttonEnabled: pin.acceptableInput && !completed
    onAccepted: {
        if (completed || !pin.acceptableInput) return;
        completed = true;
        submitPin(pin.text.trim());
        stack.pop();
    }
    StackView.onRemoved: cancelPin()
    Item {
        GridLayout {
            anchors {
                top: parent.top
                horizontalCenter: parent.horizontalCenter
                topMargin: 20
            }
            columns: 2
            rowSpacing: 10
            columnSpacing: 20
                        Label {
                            Layout.alignment: Qt.AlignRight
                            text: qsTr("Remote Play PIN (4 digits):")
                        }

                        TextField {
                            id: pin
                            validator: RegularExpressionValidator { regularExpression: /[0-9]{4}/ }
                            echoMode: Chiaki.settings.streamerMode ? TextInput.Password : TextInput.Normal
                            Layout.preferredWidth: 400
                        }
        }

    }
}
