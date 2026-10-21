import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "controls" as C

ColumnLayout {
    id: pane
    property var settings
    signal saved()
    property string inputError: ""
    spacing: 14

    Repeater {
        model: [
            {name: "mouseHorizontalSensitivity", label: qsTr("Horizontal sensitivity"), unit: "×", factor: 1, minimum: pane.settings.mouseSensitivityMinimum, maximum: pane.settings.mouseSensitivityMaximum, step: 0.05, decimals: 2},
            {name: "mouseVerticalSensitivity", label: qsTr("Vertical sensitivity"), unit: "×", factor: 1, minimum: pane.settings.mouseSensitivityMinimum, maximum: pane.settings.mouseSensitivityMaximum, step: 0.05, decimals: 2},
            {name: "mouseDeadZoneCompensation", label: qsTr("Game dead-zone compensation"), unit: "%", factor: 100, minimum: 0, maximum: pane.settings.mouseCompensationMaximum * 100, step: 1, decimals: 0}
        ]
        delegate: Frame {
            id: row
            required property var modelData
            readonly property real storedValue: Number(pane.settings[modelData.name]) * modelData.factor
            Layout.fillWidth: true
            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
            ColumnLayout {
                anchors.fill: parent
                spacing: 6
                RowLayout {
                    Layout.fillWidth: true
                    Label { text: row.modelData.label; wrapMode: Text.Wrap; Layout.fillWidth: true; font.pixelSize: 14 }
                    C.TextField {
                        id: numeric
                        objectName: row.modelData.name
                        Layout.preferredWidth: 84
                        horizontalAlignment: TextInput.AlignRight
                        Accessible.name: row.modelData.label + " " + row.modelData.unit
                        selectByMouse: true
                        validator: DoubleValidator {
                            bottom: row.modelData.minimum
                            top: row.modelData.maximum
                            decimals: row.modelData.decimals
                            notation: DoubleValidator.StandardNotation
                            locale: "C"
                        }
                        Binding {
                            target: numeric; property: "text"
                            value: row.storedValue.toFixed(row.modelData.decimals)
                            when: !numeric.activeFocus || numeric.readOnly
                            restoreMode: Binding.RestoreNone
                        }
                        onEditingFinished: {
                            const value = Number(text);
                            if (text.trim() && acceptableInput && isFinite(value)
                                    && value >= row.modelData.minimum && value <= row.modelData.maximum) {
                                pane.settings[row.modelData.name] = value / row.modelData.factor;
                                pane.inputError = "";
                                pane.saved();
                            } else pane.inputError = qsTr("Enter a value from %1 to %2 %3.").arg(row.modelData.minimum).arg(row.modelData.maximum).arg(row.modelData.unit);
                            text = row.storedValue.toFixed(row.modelData.decimals);
                        }
                    }
                    Label { text: row.modelData.unit; font.pixelSize: 14 }
                }
                C.Slider {
                    objectName: row.modelData.name + "Slider"
                    Layout.fillWidth: true
                    Accessible.name: row.modelData.label
                    from: row.modelData.minimum
                    to: row.modelData.maximum
                    stepSize: row.modelData.step
                    value: row.storedValue
                    onMoved: {
                        pane.settings[row.modelData.name] = value / row.modelData.factor;
                        pane.inputError = "";
                        pane.saved();
                    }
                }
            }
        }
    }
    Label {
        text: qsTr("Increase compensation only if small mouse movements do not move the camera.")
        wrapMode: Text.Wrap
        Layout.fillWidth: true
        font.pixelSize: 13
    }
    C.CheckBox {
        objectName: "mouseInvertVertical"
        text: qsTr("Invert vertical camera movement")
        Accessible.name: text
        checked: pane.settings.mouseInvertVertical
        onToggled: { pane.settings.mouseInvertVertical = checked; pane.saved(); }
    }
    Label {
        text: pane.inputError
        visible: text.length > 0
        color: "#ffb4ab"
        wrapMode: Text.Wrap
        Layout.fillWidth: true
    }
    Label {
        text: qsTr("Sensitivity is a multiplier. The game still controls its response curve and maximum turn speed.")
        wrapMode: Text.Wrap
        Layout.fillWidth: true
        font.pixelSize: 13
    }
}
