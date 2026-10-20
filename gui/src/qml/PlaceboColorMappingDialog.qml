import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import org.streetpea.chiaking
import "controls" as C

DialogView {
    enum GamutMappingFunction {
        Clip,
        Perceptual,
        SoftClip,
        Relative,
        Saturation,
        Absolute,
        Desaturate,
        Darken,
        Highlight,
        Linear
    }
    enum ToneMappingFunction {
        Clip,
        Spline,
        St209440,
        St209410,
        Bt2390,
        Bt2446a,
        Reinhard,
        Mobius,
        Hable,
        Gamma,
        Linear,
        LinearLight
    }
    id: dialog
    objectName: "placeboColorMappingDialog"
    property var backend: Chiaki
    property var navigation: root
    property int currentSection: 0
    title: qsTr("Color Mapping settings")
    header: ""
    buttonVisible: false
    Material.theme: Material.Dark
    Material.primary: "#11151c"
    Material.background: "#11151c"
    function close() { navigation.closeDialog(); }
    function ensureFocusVisible() {
        const item = Window.window ? Window.window.activeFocusItem : null;
        if (!item) return;
        let ancestor = item;
        while (ancestor && ancestor !== colorMappingFlick.contentItem) ancestor = ancestor.parent;
        if (!ancestor) return;
        // Keep the slider's value label visible with its control.
        const target = item.parent && item.parent !== colorMappingFlick.contentItem
                && item.parent.height <= colorMappingFlick.height ? item.parent : item;
        const point = target.mapToItem(colorMappingFlick.contentItem, 0, 0);
        if (point.y < colorMappingFlick.contentY) colorMappingFlick.contentY = point.y;
        else if (point.y + target.height > colorMappingFlick.contentY + colorMappingFlick.height)
            colorMappingFlick.contentY = point.y + target.height - colorMappingFlick.height;
    }
    Keys.onPressed: event => {
        if (event.modifiers || (event.key !== Qt.Key_PageUp && event.key !== Qt.Key_PageDown)) return;
        currentSection = Math.max(0, Math.min(4, currentSection + (event.key === Qt.Key_PageDown ? 1 : -1)));
        sections.itemAt(currentSection).forceActiveFocus(Qt.TabFocusReason);
        event.accepted = true;
    }
    onCurrentSectionChanged: colorMappingFlick.contentY = 0
    Item {
        Rectangle { parent: dialog; anchors.fill: parent; color: "#11151c"; z: -1 }
        RowLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 20
            ColumnLayout {
                Layout.preferredWidth: 180
                Layout.minimumWidth: 180
                Layout.maximumWidth: 180
                Layout.fillHeight: true
                spacing: 4
                Repeater {
                    id: sections
                    model: [qsTr("Gamut Mapping 1"), qsTr("Gamut Mapping 2"), qsTr("Tone-mapping 1"), qsTr("Tone-mapping 2"), qsTr("Tone-mapping 3")]
                    delegate: C.Button {
                        required property int index
                        required property string modelData
                        objectName: "colorMappingSection" + index
                        Layout.fillWidth: true
                        text: modelData
                        Accessible.name: modelData
                        checked: dialog.currentSection === index
                        font.pixelSize: 14
                        onClicked: dialog.currentSection = index
                        background: Rectangle {
                            radius: 6
                            color: parent.checked ? "#233b57" : parent.hovered ? "#1a202b" : "transparent"
                            border.color: parent.activeFocus ? "#78b8ff" : "transparent"
                        }
                    }
                }
                Item { Layout.fillHeight: true }
            }
            Flickable {
                id: colorMappingFlick
                objectName: "colorMappingFlick"
                Layout.minimumWidth: 0
                Layout.preferredWidth: 800
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: width
                contentHeight: colorMappingGroups.implicitHeight + 16
                flickableDirection: Flickable.VerticalFlick
                clip: true
                ScrollBar.vertical: ScrollBar {}
                ColumnLayout {
                    id: colorMappingGroups
                    width: Math.max(0, colorMappingFlick.width - 16)
                    spacing: 14
                    ColumnLayout {
                        objectName: "colorMappingGroup0"
                        visible: dialog.currentSection === 0
                        Layout.fillWidth: true
                        spacing: 12
                        Label { text: qsTr("Gamut Mapping 1"); font.pixelSize: 16; font.bold: true }
                        Label { text: qsTr("Defaults are shown in parentheses."); font.pixelSize: 14; wrapMode: Text.Wrap; Layout.fillWidth: true }
                        Frame {
                            Layout.fillWidth: true
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            GridLayout {
                                anchors.fill: parent
                                columns: 2
                                columnSpacing: 20
                                rowSpacing: 16
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Gamut Mapping Function:") + "\n" + qsTr("(Perceptual)")
                                    visible: true
                                }
                                C.ComboBox {
                                    id: placeboGamutMappingFunction
                                    objectName: "placeboGamutMappingFunction"
                                    Accessible.name: qsTr("Gamut Mapping Function:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredWidth: 400
                                    popup.width: width
                                    font.pixelSize: 14
                                    model: [qsTr("Clip"), qsTr("Perceptual"), qsTr("Soft Clip"), qsTr("Relative"), qsTr("Saturation"), qsTr("Absolute"), qsTr("Desaturate"), qsTr("Darken"), qsTr("Highlight"), qsTr("Linear")]
                                    currentIndex: dialog.backend.settings.placeboGamutMappingFunction
                                    onActivated: index => dialog.backend.settings.placeboGamutMappingFunction = index
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Perceptual Deadzone:") + "\n" + qsTr("(0.30)")
                                    visible: dialog.backend.settings.placeboGamutMappingFunction == PlaceboColorMappingDialog.GamutMappingFunction.Perceptual
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboGamutMappingFunction == PlaceboColorMappingDialog.GamutMappingFunction.Perceptual
                                    C.Slider {
                                        id: placeboGamutMappingPerceptualDeadzone
                                        objectName: "placeboGamutMappingPerceptualDeadzone"
                                        Accessible.name: qsTr("Perceptual Deadzone:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.00
                                        to: 1.00
                                        stepSize: 0.01
                                        value: dialog.backend.settings.placeboGamutMappingPerceptualDeadzone
                                        onMoved: dialog.backend.settings.placeboGamutMappingPerceptualDeadzone = value
                                    }
                                    Label { objectName: "placeboGamutMappingPerceptualDeadzoneValue"; text: qsTr(placeboGamutMappingPerceptualDeadzone.value.toFixed(2)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Perceptual Strength:") + "\n" + qsTr("(0.80)")
                                    visible: dialog.backend.settings.placeboGamutMappingFunction == PlaceboColorMappingDialog.GamutMappingFunction.Perceptual
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboGamutMappingFunction == PlaceboColorMappingDialog.GamutMappingFunction.Perceptual
                                    C.Slider {
                                        id: placeboGamutMappingPerceptualStrength
                                        objectName: "placeboGamutMappingPerceptualStrength"
                                        Accessible.name: qsTr("Perceptual Strength:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.00
                                        to: 1.00
                                        stepSize: 0.01
                                        value: dialog.backend.settings.placeboGamutMappingPerceptualStrength
                                        onMoved: dialog.backend.settings.placeboGamutMappingPerceptualStrength = value
                                    }
                                    Label { objectName: "placeboGamutMappingPerceptualStrengthValue"; text: qsTr(placeboGamutMappingPerceptualStrength.value.toFixed(2)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Colorimetric Gamma:") + "\n" + qsTr("(1.80)")
                                    visible: dialog.backend.settings.placeboGamutMappingFunction == PlaceboColorMappingDialog.GamutMappingFunction.Relative || dialog.backend.settings.placeboGamutMappingFunction == PlaceboColorMappingDialog.GamutMappingFunction.Absolute || dialog.backend.settings.placeboGamutMappingFunction == PlaceboColorMappingDialog.GamutMappingFunction.Darken
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboGamutMappingFunction == PlaceboColorMappingDialog.GamutMappingFunction.Relative || dialog.backend.settings.placeboGamutMappingFunction == PlaceboColorMappingDialog.GamutMappingFunction.Absolute || dialog.backend.settings.placeboGamutMappingFunction == PlaceboColorMappingDialog.GamutMappingFunction.Darken
                                    C.Slider {
                                        id: placeboGamutMappingColorimetricGamma
                                        objectName: "placeboGamutMappingColorimetricGamma"
                                        Accessible.name: qsTr("Colorimetric Gamma:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.00
                                        to: 10.00
                                        stepSize: 0.01
                                        value: dialog.backend.settings.placeboGamutMappingColorimetricGamma
                                        onMoved: dialog.backend.settings.placeboGamutMappingColorimetricGamma = value
                                    }
                                    Label { objectName: "placeboGamutMappingColorimetricGammaValue"; text: qsTr(placeboGamutMappingColorimetricGamma.value.toFixed(2)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Soft Clip Knee:") + "\n" + qsTr("(0.70)")
                                    visible: dialog.backend.settings.placeboGamutMappingFunction == PlaceboColorMappingDialog.GamutMappingFunction.Perceptual || dialog.backend.settings.placeboGamutMappingFunction == PlaceboColorMappingDialog.GamutMappingFunction.SoftClip
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboGamutMappingFunction == PlaceboColorMappingDialog.GamutMappingFunction.Perceptual || dialog.backend.settings.placeboGamutMappingFunction == PlaceboColorMappingDialog.GamutMappingFunction.SoftClip
                                    C.Slider {
                                        id: placeboGamutMappingSoftClipKnee
                                        objectName: "placeboGamutMappingSoftClipKnee"
                                        Accessible.name: qsTr("Soft Clip Knee:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.00
                                        to: 1.00
                                        stepSize: 0.01
                                        value: dialog.backend.settings.placeboGamutMappingSoftClipKnee
                                        onMoved: dialog.backend.settings.placeboGamutMappingSoftClipKnee = value
                                    }
                                    Label { objectName: "placeboGamutMappingSoftClipKneeValue"; text: qsTr(placeboGamutMappingSoftClipKnee.value.toFixed(2)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Soft Clip Desaturation:") + "\n" + qsTr("(0.35)")
                                    visible: dialog.backend.settings.placeboGamutMappingFunction == PlaceboColorMappingDialog.GamutMappingFunction.SoftClip
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboGamutMappingFunction == PlaceboColorMappingDialog.GamutMappingFunction.SoftClip
                                    C.Slider {
                                        id: placeboGamutMappingSoftClipDesat
                                        objectName: "placeboGamutMappingSoftClipDesat"
                                        Accessible.name: qsTr("Soft Clip Desaturation:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.00
                                        to: 1.00
                                        stepSize: 0.01
                                        value: dialog.backend.settings.placeboGamutMappingSoftClipDesat
                                        onMoved: dialog.backend.settings.placeboGamutMappingSoftClipDesat = value
                                    }
                                    Label { objectName: "placeboGamutMappingSoftClipDesatValue"; text: qsTr(placeboGamutMappingSoftClipDesat.value.toFixed(2)); font.pixelSize: 14 }
                                }
                            }
                        }
                    }
                    ColumnLayout {
                        objectName: "colorMappingGroup1"
                        visible: dialog.currentSection === 1
                        Layout.fillWidth: true
                        spacing: 12
                        Label { text: qsTr("Gamut Mapping 2"); font.pixelSize: 16; font.bold: true }
                        Label { text: qsTr("Defaults are shown in parentheses."); font.pixelSize: 14; wrapMode: Text.Wrap; Layout.fillWidth: true }
                        Frame {
                            Layout.fillWidth: true
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            GridLayout {
                                anchors.fill: parent
                                columns: 2
                                columnSpacing: 20
                                rowSpacing: 16
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("LUT 3D Size I:") + "\n" + qsTr("(48)")
                                    visible: true
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: true
                                    C.Slider {
                                        id: placeboGamutMappingLut3dSizeI
                                        objectName: "placeboGamutMappingLut3dSizeI"
                                        Accessible.name: qsTr("LUT 3D Size I:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0
                                        to: 1024
                                        stepSize: 1
                                        value: dialog.backend.settings.placeboGamutMappingLut3dSizeI
                                        onMoved: dialog.backend.settings.placeboGamutMappingLut3dSizeI = value
                                    }
                                    Label { objectName: "placeboGamutMappingLut3dSizeIValue"; text: qsTr("%1").arg(placeboGamutMappingLut3dSizeI.value); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("LUT 3D Size C:") + "\n" + qsTr("(32)")
                                    visible: true
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: true
                                    C.Slider {
                                        id: placeboGamutMappingLut3dSizeC
                                        objectName: "placeboGamutMappingLut3dSizeC"
                                        Accessible.name: qsTr("LUT 3D Size C:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0
                                        to: 1024
                                        stepSize: 1
                                        value: dialog.backend.settings.placeboGamutMappingLut3dSizeC
                                        onMoved: dialog.backend.settings.placeboGamutMappingLut3dSizeC = value
                                    }
                                    Label { objectName: "placeboGamutMappingLut3dSizeCValue"; text: qsTr("%1").arg(placeboGamutMappingLut3dSizeC.value); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("LUT 3D Size h:") + "\n" + qsTr("(256)")
                                    visible: true
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: true
                                    C.Slider {
                                        id: placeboGamutMappingLut3dSizeH
                                        objectName: "placeboGamutMappingLut3dSizeH"
                                        Accessible.name: qsTr("LUT 3D Size h:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0
                                        to: 1024
                                        stepSize: 1
                                        value: dialog.backend.settings.placeboGamutMappingLut3dSizeH
                                        onMoved: dialog.backend.settings.placeboGamutMappingLut3dSizeH = value
                                    }
                                    Label { objectName: "placeboGamutMappingLut3dSizeHValue"; text: qsTr("%1").arg(placeboGamutMappingLut3dSizeH.value); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("LUT 3D Tricubic Enabled:") + "\n" + qsTr("(Unchecked)")
                                    visible: true
                                }
                                C.CheckBox {
                                    id: placeboGamutMappingLut3dTricubicEnabled
                                    objectName: "placeboGamutMappingLut3dTricubicEnabled"
                                    Accessible.name: qsTr("LUT 3D Tricubic Enabled:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    font.pixelSize: 14
                                    text: qsTr("Enable Tricubic Interpolation for 3DLUTs")
                                    checked: dialog.backend.settings.placeboGamutMappingLut3dTricubicEnabled
                                    onToggled: dialog.backend.settings.placeboGamutMappingLut3dTricubicEnabled = checked
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Gamut Expansion Enabled:") + "\n" + qsTr("(Unchecked)")
                                    visible: true
                                }
                                C.CheckBox {
                                    id: placeboGamutMappingGamutExpansionEnabled
                                    objectName: "placeboGamutMappingGamutExpansionEnabled"
                                    Accessible.name: qsTr("Gamut Expansion Enabled:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    font.pixelSize: 14
                                    text: qsTr("Enable Gamut Expansion")
                                    checked: dialog.backend.settings.placeboGamutMappingGamutExpansionEnabled
                                    onToggled: dialog.backend.settings.placeboGamutMappingGamutExpansionEnabled = checked
                                }
                            }
                        }
                    }
                    ColumnLayout {
                        objectName: "colorMappingGroup2"
                        visible: dialog.currentSection === 2
                        Layout.fillWidth: true
                        spacing: 12
                        Label { text: qsTr("Tone-mapping 1"); font.pixelSize: 16; font.bold: true }
                        Label { text: qsTr("Defaults are shown in parentheses."); font.pixelSize: 14; wrapMode: Text.Wrap; Layout.fillWidth: true }
                        Frame {
                            Layout.fillWidth: true
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            GridLayout {
                                anchors.fill: parent
                                columns: 2
                                columnSpacing: 20
                                rowSpacing: 16
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Tone-mapping Function:") + "\n" + qsTr("(Spline)")
                                    visible: true
                                }
                                C.ComboBox {
                                    id: placeboToneMappingFunction
                                    objectName: "placeboToneMappingFunction"
                                    Accessible.name: qsTr("Tone-mapping Function:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredWidth: 400
                                    popup.width: width
                                    font.pixelSize: 14
                                    model: [qsTr("Clip"), qsTr("Spline"), qsTr("St2094-40"), qsTr("St-2094-10"), qsTr("Bt2390"), qsTr("Bt2446a"), qsTr("Reinhard"), qsTr("Mobius"), qsTr("Hable"), qsTr("Gamma"), qsTr("Linear"), qsTr("Linear Light")]
                                    currentIndex: dialog.backend.settings.placeboToneMappingFunction
                                    onActivated: index => dialog.backend.settings.placeboToneMappingFunction = index
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Knee Adaptation:") + "\n" + qsTr("(0.40)")
                                    visible: dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.Spline || dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.St209440 || dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.St209410
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.Spline || dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.St209440 || dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.St209410
                                    C.Slider {
                                        id: placeboToneMappingKneeAdaptation
                                        objectName: "placeboToneMappingKneeAdaptation"
                                        Accessible.name: qsTr("Knee Adaptation:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.00
                                        to: 1.00
                                        stepSize: 0.01
                                        value: dialog.backend.settings.placeboToneMappingKneeAdaptation
                                        onMoved: dialog.backend.settings.placeboToneMappingKneeAdaptation = value
                                    }
                                    Label { objectName: "placeboToneMappingKneeAdaptationValue"; text: qsTr(placeboToneMappingKneeAdaptation.value.toFixed(2)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Knee Minimum:") + "\n" + qsTr("(0.10)")
                                    visible: placeboToneMappingKneeAdaptation.visible
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: placeboToneMappingKneeAdaptation.visible
                                    C.Slider {
                                        id: placeboToneMappingKneeMinimum
                                        objectName: "placeboToneMappingKneeMinimum"
                                        Accessible.name: qsTr("Knee Minimum:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.00
                                        to: 0.50
                                        stepSize: 0.01
                                        value: dialog.backend.settings.placeboToneMappingKneeMinimum
                                        onMoved: dialog.backend.settings.placeboToneMappingKneeMinimum = value
                                    }
                                    Label { objectName: "placeboToneMappingKneeMinimumValue"; text: qsTr(placeboToneMappingKneeMinimum.value.toFixed(2)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Knee Maximum:") + "\n" + qsTr("(0.80)")
                                    visible: placeboToneMappingKneeAdaptation.visible
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: placeboToneMappingKneeAdaptation.visible
                                    C.Slider {
                                        id: placeboToneMappingKneeMaximum
                                        objectName: "placeboToneMappingKneeMaximum"
                                        Accessible.name: qsTr("Knee Maximum:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.50
                                        to: 1.00
                                        stepSize: 0.01
                                        value: dialog.backend.settings.placeboToneMappingKneeMaximum
                                        onMoved: dialog.backend.settings.placeboToneMappingKneeMaximum = value
                                    }
                                    Label { objectName: "placeboToneMappingKneeMaximumValue"; text: qsTr(placeboToneMappingKneeMaximum.value.toFixed(2)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Knee Default:") + "\n" + qsTr("(0.40)")
                                    visible: true
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: true
                                    C.Slider {
                                        id: placeboToneMappingKneeDefault
                                        objectName: "placeboToneMappingKneeDefault"
                                        Accessible.name: qsTr("Knee Default:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.00
                                        to: 1.00
                                        stepSize: 0.01
                                        value: dialog.backend.settings.placeboToneMappingKneeDefault
                                        onMoved: dialog.backend.settings.placeboToneMappingKneeDefault = value
                                    }
                                    Label { objectName: "placeboToneMappingKneeDefaultValue"; text: qsTr(placeboToneMappingKneeDefault.value.toFixed(2)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Knee Offset:") + "\n" + qsTr("(1.00)")
                                    visible: dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.Bt2390
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.Bt2390
                                    C.Slider {
                                        id: placeboToneMappingKneeOffset
                                        objectName: "placeboToneMappingKneeOffset"
                                        Accessible.name: qsTr("Knee Offset:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.50
                                        to: 2.00
                                        stepSize: 0.01
                                        value: dialog.backend.settings.placeboToneMappingKneeOffset
                                        onMoved: dialog.backend.settings.placeboToneMappingKneeOffset = value
                                    }
                                    Label { objectName: "placeboToneMappingKneeOffsetValue"; text: qsTr(placeboToneMappingKneeOffset.value.toFixed(2)); font.pixelSize: 14 }
                                }
                            }
                        }
                    }
                    ColumnLayout {
                        objectName: "colorMappingGroup3"
                        visible: dialog.currentSection === 3
                        Layout.fillWidth: true
                        spacing: 12
                        Label { text: qsTr("Tone-mapping 2"); font.pixelSize: 16; font.bold: true }
                        Label { text: qsTr("Defaults are shown in parentheses."); font.pixelSize: 14; wrapMode: Text.Wrap; Layout.fillWidth: true }
                        Frame {
                            Layout.fillWidth: true
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            GridLayout {
                                anchors.fill: parent
                                columns: 2
                                columnSpacing: 20
                                rowSpacing: 16
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Slope Tuning:") + "\n" + qsTr("(1.5)")
                                    visible: dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.Spline
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.Spline
                                    C.Slider {
                                        id: placeboToneMappingSlopeTuning
                                        objectName: "placeboToneMappingSlopeTuning"
                                        Accessible.name: qsTr("Slope Tuning:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.0
                                        to: 10.0
                                        stepSize: 0.1
                                        value: dialog.backend.settings.placeboToneMappingSlopeTuning
                                        onMoved: dialog.backend.settings.placeboToneMappingSlopeTuning = value
                                    }
                                    Label { objectName: "placeboToneMappingSlopeTuningValue"; text: qsTr(placeboToneMappingSlopeTuning.value.toFixed(1)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Slope Offset:") + "\n" + qsTr("(0.20)")
                                    visible: dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.Spline
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.Spline
                                    C.Slider {
                                        id: placeboToneMappingSlopeOffset
                                        objectName: "placeboToneMappingSlopeOffset"
                                        Accessible.name: qsTr("Slope Offset:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.00
                                        to: 1.00
                                        stepSize: 0.01
                                        value: dialog.backend.settings.placeboToneMappingSlopeOffset
                                        onMoved: dialog.backend.settings.placeboToneMappingSlopeOffset = value
                                    }
                                    Label { objectName: "placeboToneMappingSlopeOffsetValue"; text: qsTr(placeboToneMappingSlopeOffset.value.toFixed(2)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Spline Contrast:") + "\n" + qsTr("(0.50)")
                                    visible: dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.Spline
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.Spline
                                    C.Slider {
                                        id: placeboToneMappingSplineContrast
                                        objectName: "placeboToneMappingSplineContrast"
                                        Accessible.name: qsTr("Spline Contrast:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.00
                                        to: 1.50
                                        stepSize: 0.01
                                        value: dialog.backend.settings.placeboToneMappingSplineContrast
                                        onMoved: dialog.backend.settings.placeboToneMappingSplineContrast = value
                                    }
                                    Label { objectName: "placeboToneMappingSplineContrastValue"; text: qsTr(placeboToneMappingSplineContrast.value.toFixed(2)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Reinhard Contrast:") + "\n" + qsTr("(0.50)")
                                    visible: dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.Reinhard
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.Reinhard
                                    C.Slider {
                                        id: placeboToneMappingReinhardContrast
                                        objectName: "placeboToneMappingReinhardContrast"
                                        Accessible.name: qsTr("Reinhard Contrast:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.00
                                        to: 1.00
                                        stepSize: 0.01
                                        value: dialog.backend.settings.placeboToneMappingReinhardContrast
                                        onMoved: dialog.backend.settings.placeboToneMappingReinhardContrast = value
                                    }
                                    Label { objectName: "placeboToneMappingReinhardContrastValue"; text: qsTr(placeboToneMappingReinhardContrast.value.toFixed(2)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Linear Knee:") + "\n" + qsTr("(0.3)")
                                    visible: dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.Mobius || dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.Gamma
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.Mobius || dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.Gamma
                                    C.Slider {
                                        id: placeboToneMappingLinearKnee
                                        objectName: "placeboToneMappingLinearKnee"
                                        Accessible.name: qsTr("Linear Knee:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.00
                                        to: 1.00
                                        stepSize: 0.01
                                        value: dialog.backend.settings.placeboToneMappingLinearKnee
                                        onMoved: dialog.backend.settings.placeboToneMappingLinearKnee = value
                                    }
                                    Label { objectName: "placeboToneMappingLinearKneeValue"; text: qsTr(placeboToneMappingLinearKnee.value.toFixed(2)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Exposure:") + "\n" + qsTr("(1.0)")
                                    visible: dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.Linear || dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.LinearLight
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.Linear || dialog.backend.settings.placeboToneMappingFunction == PlaceboColorMappingDialog.ToneMappingFunction.LinearLight
                                    C.Slider {
                                        id: placeboToneMappingExposure
                                        objectName: "placeboToneMappingExposure"
                                        Accessible.name: qsTr("Exposure:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.0
                                        to: 10.0
                                        stepSize: 0.1
                                        value: dialog.backend.settings.placeboToneMappingExposure
                                        onMoved: dialog.backend.settings.placeboToneMappingExposure = value
                                    }
                                    Label { objectName: "placeboToneMappingExposureValue"; text: qsTr(placeboToneMappingExposure.value.toFixed(1)); font.pixelSize: 14 }
                                }
                            }
                        }
                    }
                    ColumnLayout {
                        objectName: "colorMappingGroup4"
                        visible: dialog.currentSection === 4
                        Layout.fillWidth: true
                        spacing: 12
                        Label { text: qsTr("Tone-mapping 3"); font.pixelSize: 16; font.bold: true }
                        Label { text: qsTr("Defaults are shown in parentheses."); font.pixelSize: 14; wrapMode: Text.Wrap; Layout.fillWidth: true }
                        Frame {
                            Layout.fillWidth: true
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            GridLayout {
                                anchors.fill: parent
                                columns: 2
                                columnSpacing: 20
                                rowSpacing: 16
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Inverse Tone-mapping Enabled:") + "\n" + qsTr("(Unchecked)")
                                    visible: true
                                }
                                C.CheckBox {
                                    id: placeboToneMappingInverseToneMappingEnabled
                                    objectName: "placeboToneMappingInverseToneMappingEnabled"
                                    Accessible.name: qsTr("Inverse Tone-mapping Enabled:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    font.pixelSize: 14
                                    text: qsTr("Enable Inverse Tone-mapping")
                                    checked: dialog.backend.settings.placeboToneMappingInverseToneMappingEnabled
                                    onToggled: dialog.backend.settings.placeboToneMappingInverseToneMappingEnabled = checked
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Tone-mapping Metadata:") + "\n" + qsTr("(Any)")
                                    visible: true
                                }
                                C.ComboBox {
                                    id: placeboToneMappingMetadata
                                    objectName: "placeboToneMappingMetadata"
                                    Accessible.name: qsTr("Tone-mapping Metadata:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredWidth: 400
                                    popup.width: width
                                    font.pixelSize: 14
                                    model: [qsTr("Any"), qsTr("None"), qsTr("HDR10"), qsTr("HDR10 Plus"), qsTr("Cie_y")]
                                    currentIndex: dialog.backend.settings.placeboToneMappingMetadata
                                    onActivated: index => dialog.backend.settings.placeboToneMappingMetadata = index
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Tone LUT Size:") + "\n" + qsTr("(256)")
                                    visible: true
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: true
                                    C.Slider {
                                        id: placeboToneMappingToneLutSize
                                        objectName: "placeboToneMappingToneLutSize"
                                        Accessible.name: qsTr("Tone LUT Size:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0
                                        to: 4096
                                        stepSize: 1
                                        value: dialog.backend.settings.placeboToneMappingToneLutSize
                                        onMoved: dialog.backend.settings.placeboToneMappingToneLutSize = value
                                    }
                                    Label { objectName: "placeboToneMappingToneLutSizeValue"; text: qsTr("%1").arg(placeboToneMappingToneLutSize.value); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Contrast Recovery:") + "\n" + qsTr("(0.30)")
                                    visible: true
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: true
                                    C.Slider {
                                        id: placeboToneMappingContrastRecovery
                                        objectName: "placeboToneMappingContrastRecovery"
                                        Accessible.name: qsTr("Contrast Recovery:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.00
                                        to: 2.00
                                        stepSize: 0.01
                                        value: dialog.backend.settings.placeboToneMappingContrastRecovery
                                        onMoved: dialog.backend.settings.placeboToneMappingContrastRecovery = value
                                    }
                                    Label { objectName: "placeboToneMappingContrastRecoveryValue"; text: qsTr(placeboToneMappingContrastRecovery.value.toFixed(2)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Contrast Smoothness:") + "\n" + qsTr("(3.5)")
                                    visible: true
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: true
                                    C.Slider {
                                        id: placeboToneMappingContrastSmoothness
                                        objectName: "placeboToneMappingContrastSmoothness"
                                        Accessible.name: qsTr("Contrast Smoothness:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 1.0
                                        to: 32.0
                                        stepSize: 0.1
                                        value: dialog.backend.settings.placeboToneMappingContrastSmoothness
                                        onMoved: dialog.backend.settings.placeboToneMappingContrastSmoothness = value
                                    }
                                    Label { objectName: "placeboToneMappingContrastSmoothnessValue"; text: qsTr(placeboToneMappingContrastSmoothness.value.toFixed(1)); font.pixelSize: 14 }
                                }
                            }
                        }
                    }
                }
                Connections {
                    target: dialog.Window.window
                    function onActiveFocusItemChanged() { dialog.ensureFocusVisible(); }
                }
            }
        }
    }
}
