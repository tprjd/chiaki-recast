import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import org.streetpea.chiaking
import "controls" as C

DialogView {
    id: dialog
    objectName: "placeboSettingsDialog"
    property var backend: Chiaki
    property var navigation: root
    property string options_url: ""
    property int currentSection: 0
    title: qsTr("Renderer settings")
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
        while (ancestor && ancestor !== rendererFlick.contentItem) ancestor = ancestor.parent;
        if (!ancestor) return;
        // Keep the slider's value label visible with its control.
        const target = item.parent && item.parent !== rendererFlick.contentItem
                && item.parent.height <= rendererFlick.height ? item.parent : item;
        const point = target.mapToItem(rendererFlick.contentItem, 0, 0);
        if (point.y < rendererFlick.contentY) rendererFlick.contentY = point.y;
        else if (point.y + target.height > rendererFlick.contentY + rendererFlick.height)
            rendererFlick.contentY = point.y + target.height - rendererFlick.height;
    }
    Keys.onPressed: event => {
        if (event.modifiers || (event.key !== Qt.Key_PageUp && event.key !== Qt.Key_PageDown)) return;
        currentSection = Math.max(0, Math.min(6, currentSection + (event.key === Qt.Key_PageDown ? 1 : -1)));
        sections.itemAt(currentSection).forceActiveFocus(Qt.TabFocusReason);
        event.accepted = true;
    }
    onCurrentSectionChanged: rendererFlick.contentY = 0
    Item {
        Rectangle { parent: dialog; anchors.fill: parent; color: "#11151c"; z: -1 }
        RowLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 20
            Flickable {
                id: sectionSidebar
                Layout.preferredWidth: 180
                Layout.minimumWidth: 180
                Layout.maximumWidth: 180
                Layout.fillHeight: true
                contentHeight: sectionColumn.implicitHeight
                clip: true
                ScrollBar.vertical: ScrollBar {}
                ColumnLayout {
                    id: sectionColumn
                    width: parent.width
                    spacing: 4
                    Repeater {
                        id: sections
                        model: [qsTr("Config"), qsTr("Scaling"), qsTr("Debanding"), qsTr("Sigmoidization"), qsTr("Color"), qsTr("Peak Detection"), qsTr("Color Mapping")]
                        delegate: C.Button {
                            required property int index
                            required property string modelData
                            objectName: "rendererSection" + index
                            Layout.fillWidth: true
                            text: modelData
                            contentItem: Label {
                                text: parent.text
                                font: parent.font
                                wrapMode: Text.Wrap
                                verticalAlignment: Text.AlignVCenter
                            }
                            Accessible.name: modelData
                            checked: dialog.currentSection === index
                            font.pixelSize: 14
                            onActiveFocusChanged: if (activeFocus)
                                sectionSidebar.contentY = Math.max(0, Math.min(y, sectionSidebar.contentHeight - sectionSidebar.height))
                            onClicked: dialog.currentSection = index
                            background: Rectangle {
                                radius: 6
                                color: parent.checked ? "#233b57" : parent.hovered ? "#1a202b" : "transparent"
                                border.color: parent.activeFocus ? "#78b8ff" : "transparent"
                            }
                        }
                    }
                }
            }
            Flickable {
                id: rendererFlick
                objectName: "rendererFlick"
                Layout.minimumWidth: 0
                Layout.preferredWidth: 800
                Layout.fillWidth: true
                Layout.fillHeight: true
                contentWidth: width
                contentHeight: rendererGroups.implicitHeight + 16
                flickableDirection: Flickable.VerticalFlick
                clip: true
                ScrollBar.vertical: ScrollBar {}
                ColumnLayout {
                    id: rendererGroups
                    width: Math.max(0, rendererFlick.width - 16)
                    spacing: 14
                    ColumnLayout {
                        objectName: "rendererGroup0"
                        visible: dialog.currentSection === 0
                        Layout.fillWidth: true
                        spacing: 12
                        Label { text: qsTr("Config"); font.pixelSize: 16; font.bold: true }
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
                                    text: qsTr("Libplacebo Renderer Options Details")
                                    visible: !options_url
                                }
                                C.Button {
                                    Layout.fillWidth: true
                                    font.pixelSize: 14
                                    Accessible.name: text
                                    id: openwebpage
                                    objectName: "openRendererDocumentation"
                                    text: qsTr("Open Documentation")
                                    onClicked: {
                                        options_url = dialog.backend.openPlaceboOptionsLink()
                                    }
                                    visible: !options_url
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Open Documentation in Web Browser with copied URL")
                                    visible: options_url
                                }
                                TextField {
                                    id: openurl
                                    objectName: "rendererDocumentationUrl"
                                    echoMode: dialog.backend.settings.streamerMode ? TextInput.Password : TextInput.Normal
                                    text: options_url
                                    visible: false
                                    Layout.fillWidth: true
                                }
                                C.Button {
                                    Layout.fillWidth: true
                                    font.pixelSize: 14
                                    Accessible.name: text
                                    id: copyUrl
                                    objectName: "copyRendererDocumentationUrl"
                                    text: qsTr("Click to Re-Copy URL")
                                    onClicked: {
                                        openurl.selectAll()
                                        openurl.copy()
                                    }
                                    visible: options_url
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Export Render Settings to File")
                                }
                                C.Button {
                                    Layout.fillWidth: true
                                    font.pixelSize: 14
                                    Accessible.name: text
                                    id: exportButton
                                    objectName: "exportPlaceboSettings"
                                    text: qsTr("Export")
                                    onClicked: {
                                        dialog.backend.settings.exportPlaceboSettings();
                                    }
                                    Material.roundedScale: Material.SmallScale
                                    KeyNavigation.down: importButton
                                    KeyNavigation.right: importButton
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Import Render Settings from File")
                                }
                                C.Button {
                                    Layout.fillWidth: true
                                    font.pixelSize: 14
                                    Accessible.name: text
                                    id: importButton
                                    objectName: "importPlaceboSettings"
                                    lastInFocusChain: true
                                    text: qsTr("Import")
                                    onClicked: {
                                        dialog.backend.settings.importPlaceboSettings();
                                    }
                                    Material.roundedScale: Material.SmallScale
                                    KeyNavigation.up: exportButton
                                    KeyNavigation.left: exportButton
                                }
                            }
                        }
                    }
                    ColumnLayout {
                        objectName: "rendererGroup1"
                        visible: dialog.currentSection === 1
                        Layout.fillWidth: true
                        spacing: 12
                        Label { text: qsTr("Scaling"); font.pixelSize: 16; font.bold: true }
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
                                    text: qsTr("Upscaler:") + "\n" + qsTr("(EwaLanczosSharp, FSR on modest upscale, or FSRCNNX with FSR fallback below 1.2x)")
                                    visible: true
                                }
                                C.ComboBox {
                                    id: placeboUpscaler
                                    objectName: "placeboUpscaler"
                                    Accessible.name: qsTr("Upscaler:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredWidth: 400
                                    popup.width: width
                                    font.pixelSize: 14
                                    model: [qsTr("Custom"), qsTr("Nearest"), qsTr("Bilinear"), qsTr("Oversample"), qsTr("Bicubic"), qsTr("Gaussian"), qsTr("Catmull Rom"), qsTr("Lanczos"), qsTr("EwaLanczos"), qsTr("EwaLanczosSharp"), qsTr("EwaLanczosSharpest"), qsTr("FSR"), qsTr("FSRCNNX x2 8-0-4-1"), qsTr("FSRCNNX x2 16-0-4-1")]
                                    currentIndex: dialog.backend.settings.placeboUpscaler
                                    onActivated: index => dialog.backend.settings.placeboUpscaler = index
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Plane Upscaler:") + "\n" + qsTr("(Chosen Upscaler)")
                                    visible: true
                                }
                                C.ComboBox {
                                    id: placeboPlaneUpscaler
                                    objectName: "placeboPlaneUpscaler"
                                    Accessible.name: qsTr("Plane Upscaler:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredWidth: 400
                                    popup.width: width
                                    font.pixelSize: 14
                                    model: [qsTr("Chosen Upscaler"), qsTr("Nearest"), qsTr("Bilinear"), qsTr("Oversample"), qsTr("Bicubic"), qsTr("Gaussian"), qsTr("Catmull Rom"), qsTr("Lanczos"), qsTr("EwaLanczos"), qsTr("EwaLanczosSharp"), qsTr("EwaLanczosSharpest")]
                                    currentIndex: dialog.backend.settings.placeboPlaneUpscaler
                                    onActivated: index => dialog.backend.settings.placeboPlaneUpscaler = index
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Downscaler:") + "\n" + qsTr("(Hermite)")
                                    visible: true
                                }
                                C.ComboBox {
                                    id: placeboDownscaler
                                    objectName: "placeboDownscaler"
                                    Accessible.name: qsTr("Downscaler:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredWidth: 400
                                    popup.width: width
                                    font.pixelSize: 14
                                    model: [qsTr("Custom"), qsTr("Box"), qsTr("Hermite"), qsTr("Bilinear"), qsTr("Bicubic"), qsTr("Gaussian"), qsTr("Catmull Rom"), qsTr("Mitchell"), qsTr("Lanczos")]
                                    currentIndex: dialog.backend.settings.placeboDownscaler
                                    onActivated: index => dialog.backend.settings.placeboDownscaler = index
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Plane Downscaler:") + "\n" + qsTr("(Chosen Downscaler)")
                                    visible: true
                                }
                                C.ComboBox {
                                    id: placeboPlaneDownscaler
                                    objectName: "placeboPlaneDownscaler"
                                    Accessible.name: qsTr("Plane Downscaler:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredWidth: 400
                                    popup.width: width
                                    font.pixelSize: 14
                                    model: [qsTr("Chosen Downscaler"), qsTr("Box"), qsTr("Hermite"), qsTr("Bilinear"), qsTr("Bicubic"), qsTr("Gaussian"), qsTr("Catmull Rom"), qsTr("Mitchell"), qsTr("Lanczos")]
                                    currentIndex: dialog.backend.settings.placeboPlaneDownscaler
                                    onActivated: index => dialog.backend.settings.placeboPlaneDownscaler = index
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Frame Mixer:") + "\n" + qsTr("(Oversample)")
                                    visible: true
                                }
                                C.ComboBox {
                                    id: placeboFrameMixer
                                    objectName: "placeboFrameMixer"
                                    Accessible.name: qsTr("Frame Mixer:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredWidth: 400
                                    popup.width: width
                                    font.pixelSize: 14
                                    model: [qsTr("None"), qsTr("Oversample"), qsTr("Hermite"), qsTr("Linear"), qsTr("Cubic")]
                                    currentIndex: dialog.backend.settings.placeboFrameMixer
                                    onActivated: index => dialog.backend.settings.placeboFrameMixer = index
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Anti-ringing Strength:") + "\n" + qsTr("(0.00)")
                                    visible: true
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: true
                                    C.Slider {
                                        id: placeboAntiringingStrength
                                        objectName: "placeboAntiringingStrength"
                                        Accessible.name: qsTr("Anti-ringing Strength:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.00
                                        to: 1.00
                                        stepSize: 0.01
                                        value: dialog.backend.settings.placeboAntiringingStrength
                                        onMoved: dialog.backend.settings.placeboAntiringingStrength = value
                                    }
                                    Label { objectName: "placeboAntiringingStrengthValue"; text: qsTr(placeboAntiringingStrength.value.toFixed(2)); font.pixelSize: 14 }
                                }
                            }
                        }
                    }
                    ColumnLayout {
                        objectName: "rendererGroup2"
                        visible: dialog.currentSection === 2
                        Layout.fillWidth: true
                        spacing: 12
                        Label { text: qsTr("Debanding"); font.pixelSize: 16; font.bold: true }
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
                                    text: qsTr("Deband Enabled:") + "\n" + qsTr("(Checked)")
                                    visible: true
                                }
                                C.CheckBox {
                                    id: placeboDebandEnabled
                                    objectName: "placeboDebandEnabled"
                                    Accessible.name: qsTr("Deband Enabled:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    font.pixelSize: 14
                                    text: qsTr("Enable Debanding")
                                    checked: dialog.backend.settings.placeboDebandEnabled
                                    onToggled: dialog.backend.settings.placeboDebandEnabled = checked
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Deband Preset:") + "\n" + qsTr("(Custom)")
                                    visible: dialog.backend.settings.placeboDebandEnabled
                                }
                                C.ComboBox {
                                    id: placeboDebandPreset
                                    objectName: "placeboDebandPreset"
                                    Accessible.name: qsTr("Deband Preset:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredWidth: 400
                                    popup.width: width
                                    font.pixelSize: 14
                                    model: [qsTr("Custom"), qsTr("Default")]
                                    currentIndex: dialog.backend.settings.placeboDebandPreset
                                    onActivated: index => dialog.backend.settings.placeboDebandPreset = index
                                    visible: dialog.backend.settings.placeboDebandEnabled
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Deband Iterations:") + "\n" + qsTr("(1)")
                                    visible: dialog.backend.settings.placeboDebandEnabled && (dialog.backend.settings.placeboDebandPreset == 0)
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboDebandEnabled && (dialog.backend.settings.placeboDebandPreset == 0)
                                    C.Slider {
                                        id: placeboDebandIterations
                                        objectName: "placeboDebandIterations"
                                        Accessible.name: qsTr("Deband Iterations:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0
                                        to: 16
                                        stepSize: 1
                                        value: dialog.backend.settings.placeboDebandIterations
                                        onMoved: dialog.backend.settings.placeboDebandIterations = value
                                    }
                                    Label { objectName: "placeboDebandIterationsValue"; text: qsTr("%1").arg(placeboDebandIterations.value); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Deband Threshold:") + "\n" + qsTr("(3.0)")
                                    visible: dialog.backend.settings.placeboDebandEnabled && (dialog.backend.settings.placeboDebandPreset == 0)
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboDebandEnabled && (dialog.backend.settings.placeboDebandPreset == 0)
                                    C.Slider {
                                        id: placeboDebandThreshold
                                        objectName: "placeboDebandThreshold"
                                        Accessible.name: qsTr("Deband Threshold:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.0
                                        to: 1000.0
                                        stepSize: 0.1
                                        value: dialog.backend.settings.placeboDebandThreshold
                                        onMoved: dialog.backend.settings.placeboDebandThreshold = value
                                    }
                                    Label { objectName: "placeboDebandThresholdValue"; text: qsTr(placeboDebandThreshold.value.toFixed(1)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Deband Radius:") + "\n" + qsTr("(16.0)")
                                    visible: dialog.backend.settings.placeboDebandEnabled && (dialog.backend.settings.placeboDebandPreset == 0)
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboDebandEnabled && (dialog.backend.settings.placeboDebandPreset == 0)
                                    C.Slider {
                                        id: placeboDebandRadius
                                        objectName: "placeboDebandRadius"
                                        Accessible.name: qsTr("Deband Radius:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.0
                                        to: 1000.0
                                        stepSize: 0.1
                                        value: dialog.backend.settings.placeboDebandRadius
                                        onMoved: dialog.backend.settings.placeboDebandRadius = value
                                    }
                                    Label { objectName: "placeboDebandRadiusValue"; text: qsTr(placeboDebandRadius.value.toFixed(1)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Deband Grain:") + "\n" + qsTr("(4.0)")
                                    visible: dialog.backend.settings.placeboDebandEnabled && (dialog.backend.settings.placeboDebandPreset == 0)
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboDebandEnabled && (dialog.backend.settings.placeboDebandPreset == 0)
                                    C.Slider {
                                        id: placeboDebandGrain
                                        objectName: "placeboDebandGrain"
                                        Accessible.name: qsTr("Deband Grain:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.0
                                        to: 1000.0
                                        stepSize: 0.1
                                        value: dialog.backend.settings.placeboDebandGrain
                                        onMoved: dialog.backend.settings.placeboDebandGrain = value
                                    }
                                    Label { objectName: "placeboDebandGrainValue"; text: qsTr(placeboDebandGrain.value.toFixed(1)); font.pixelSize: 14 }
                                }
                            }
                        }
                    }
                    ColumnLayout {
                        objectName: "rendererGroup3"
                        visible: dialog.currentSection === 3
                        Layout.fillWidth: true
                        spacing: 12
                        Label { text: qsTr("Sigmoidization"); font.pixelSize: 16; font.bold: true }
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
                                    text: qsTr("Sigmoidization Enabled:") + "\n" + qsTr("(Checked)")
                                    visible: true
                                }
                                C.CheckBox {
                                    id: placeboSigmoidEnabled
                                    objectName: "placeboSigmoidEnabled"
                                    Accessible.name: qsTr("Sigmoidization Enabled:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    font.pixelSize: 14
                                    text: qsTr("Enable Sigmoidization")
                                    checked: dialog.backend.settings.placeboSigmoidEnabled
                                    onToggled: dialog.backend.settings.placeboSigmoidEnabled = checked
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Sigmoid Preset:") + "\n" + qsTr("(Custom)")
                                    visible: dialog.backend.settings.placeboSigmoidEnabled
                                }
                                C.ComboBox {
                                    id: placeboSigmoidPreset
                                    objectName: "placeboSigmoidPreset"
                                    Accessible.name: qsTr("Sigmoid Preset:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredWidth: 400
                                    popup.width: width
                                    font.pixelSize: 14
                                    model: [qsTr("Custom"), qsTr("Default")]
                                    currentIndex: dialog.backend.settings.placeboSigmoidPreset
                                    onActivated: index => dialog.backend.settings.placeboSigmoidPreset = index
                                    visible: dialog.backend.settings.placeboSigmoidEnabled
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Sigmoid Center:") + "\n" + qsTr("(0.75)")
                                    visible: dialog.backend.settings.placeboSigmoidEnabled && (dialog.backend.settings.placeboSigmoidPreset == 0)
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboSigmoidEnabled && (dialog.backend.settings.placeboSigmoidPreset == 0)
                                    C.Slider {
                                        id: placeboSigmoidCenter
                                        objectName: "placeboSigmoidCenter"
                                        Accessible.name: qsTr("Sigmoid Center:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.00
                                        to: 1.00
                                        stepSize: 0.01
                                        value: dialog.backend.settings.placeboSigmoidCenter
                                        onMoved: dialog.backend.settings.placeboSigmoidCenter = value
                                    }
                                    Label { objectName: "placeboSigmoidCenterValue"; text: qsTr(placeboSigmoidCenter.value.toFixed(2)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Sigmoid Slope:") + "\n" + qsTr("(6.5)")
                                    visible: dialog.backend.settings.placeboSigmoidEnabled && (dialog.backend.settings.placeboSigmoidPreset == 0)
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboSigmoidEnabled && (dialog.backend.settings.placeboSigmoidPreset == 0)
                                    C.Slider {
                                        id: placeboSigmoidSlope
                                        objectName: "placeboSigmoidSlope"
                                        Accessible.name: qsTr("Sigmoid Slope:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 1.0
                                        to: 20.0
                                        stepSize: 0.1
                                        value: dialog.backend.settings.placeboSigmoidSlope
                                        onMoved: dialog.backend.settings.placeboSigmoidSlope = value
                                    }
                                    Label { objectName: "placeboSigmoidSlopeValue"; text: qsTr(placeboSigmoidSlope.value.toFixed(1)); font.pixelSize: 14 }
                                }
                            }
                        }
                    }
                    ColumnLayout {
                        objectName: "rendererGroup4"
                        visible: dialog.currentSection === 4
                        Layout.fillWidth: true
                        spacing: 12
                        Label { text: qsTr("Color"); font.pixelSize: 16; font.bold: true }
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
                                    text: qsTr("Color Adjustment Enabled:") + "\n" + qsTr("(Checked)")
                                    visible: true
                                }
                                C.CheckBox {
                                    id: placeboColorAdjustmentEnabled
                                    objectName: "placeboColorAdjustmentEnabled"
                                    Accessible.name: qsTr("Color Adjustment Enabled:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    font.pixelSize: 14
                                    text: qsTr("Enable Color Adjustment")
                                    checked: dialog.backend.settings.placeboColorAdjustmentEnabled
                                    onToggled: dialog.backend.settings.placeboColorAdjustmentEnabled = checked
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Color Adjustment Preset:") + "\n" + qsTr("(Custom)")
                                    visible: dialog.backend.settings.placeboColorAdjustmentEnabled
                                }
                                C.ComboBox {
                                    id: placeboColorAdjustmentPreset
                                    objectName: "placeboColorAdjustmentPreset"
                                    Accessible.name: qsTr("Color Adjustment Preset:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredWidth: 400
                                    popup.width: width
                                    font.pixelSize: 14
                                    model: [qsTr("Custom"), qsTr("Neutral")]
                                    currentIndex: dialog.backend.settings.placeboColorAdjustmentPreset
                                    onActivated: index => dialog.backend.settings.placeboColorAdjustmentPreset = index
                                    visible: dialog.backend.settings.placeboColorAdjustmentEnabled
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Brightness:") + "\n" + qsTr("(0.00)")
                                    visible: dialog.backend.settings.placeboColorAdjustmentEnabled && (dialog.backend.settings.placeboColorAdjustmentPreset == 0)
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboColorAdjustmentEnabled && (dialog.backend.settings.placeboColorAdjustmentPreset == 0)
                                    C.Slider {
                                        id: placeboColorAdjustmentBrightness
                                        objectName: "placeboColorAdjustmentBrightness"
                                        Accessible.name: qsTr("Brightness:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: -1.00
                                        to: 1.00
                                        stepSize: 0.01
                                        value: dialog.backend.settings.placeboColorAdjustmentBrightness
                                        onMoved: dialog.backend.settings.placeboColorAdjustmentBrightness = value
                                    }
                                    Label { objectName: "placeboColorAdjustmentBrightnessValue"; text: qsTr(placeboColorAdjustmentBrightness.value.toFixed(2)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Contrast:") + "\n" + qsTr("(1.0)")
                                    visible: dialog.backend.settings.placeboColorAdjustmentEnabled && (dialog.backend.settings.placeboColorAdjustmentPreset == 0)
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboColorAdjustmentEnabled && (dialog.backend.settings.placeboColorAdjustmentPreset == 0)
                                    C.Slider {
                                        id: placeboColorAdjustmentContrast
                                        objectName: "placeboColorAdjustmentContrast"
                                        Accessible.name: qsTr("Contrast:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.0
                                        to: 100.0
                                        stepSize: 0.1
                                        value: dialog.backend.settings.placeboColorAdjustmentContrast
                                        onMoved: dialog.backend.settings.placeboColorAdjustmentContrast = value
                                    }
                                    Label { objectName: "placeboColorAdjustmentContrastValue"; text: qsTr(placeboColorAdjustmentContrast.value.toFixed(1)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Saturation:") + "\n" + qsTr("(1.0)")
                                    visible: dialog.backend.settings.placeboColorAdjustmentEnabled && (dialog.backend.settings.placeboColorAdjustmentPreset == 0)
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboColorAdjustmentEnabled && (dialog.backend.settings.placeboColorAdjustmentPreset == 0)
                                    C.Slider {
                                        id: placeboColorAdjustmentSaturation
                                        objectName: "placeboColorAdjustmentSaturation"
                                        Accessible.name: qsTr("Saturation:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.0
                                        to: 100.0
                                        stepSize: 0.1
                                        value: dialog.backend.settings.placeboColorAdjustmentSaturation
                                        onMoved: dialog.backend.settings.placeboColorAdjustmentSaturation = value
                                    }
                                    Label { objectName: "placeboColorAdjustmentSaturationValue"; text: qsTr(placeboColorAdjustmentSaturation.value.toFixed(1)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Hue:") + "\n" + qsTr("(0.00)")
                                    visible: dialog.backend.settings.placeboColorAdjustmentEnabled && (dialog.backend.settings.placeboColorAdjustmentPreset == 0)
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboColorAdjustmentEnabled && (dialog.backend.settings.placeboColorAdjustmentPreset == 0)
                                    C.Slider {
                                        id: placeboColorAdjustmentHue
                                        objectName: "placeboColorAdjustmentHue"
                                        Accessible.name: qsTr("Hue:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.00
                                        to: 6.28
                                        stepSize: 0.01
                                        value: dialog.backend.settings.placeboColorAdjustmentHue
                                        onMoved: dialog.backend.settings.placeboColorAdjustmentHue = value
                                    }
                                    Label { objectName: "placeboColorAdjustmentHueValue"; text: qsTr(placeboColorAdjustmentHue.value.toFixed(2)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Gamma:") + "\n" + qsTr("(1.0)")
                                    visible: dialog.backend.settings.placeboColorAdjustmentEnabled && (dialog.backend.settings.placeboColorAdjustmentPreset == 0)
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboColorAdjustmentEnabled && (dialog.backend.settings.placeboColorAdjustmentPreset == 0)
                                    C.Slider {
                                        id: placeboColorAdjustmentGamma
                                        objectName: "placeboColorAdjustmentGamma"
                                        Accessible.name: qsTr("Gamma:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.0
                                        to: 100.0
                                        stepSize: 0.1
                                        value: dialog.backend.settings.placeboColorAdjustmentGamma
                                        onMoved: dialog.backend.settings.placeboColorAdjustmentGamma = value
                                    }
                                    Label { objectName: "placeboColorAdjustmentGammaValue"; text: qsTr(placeboColorAdjustmentGamma.value.toFixed(1)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Temperature:") + "\n" + qsTr("(0.000)")
                                    visible: dialog.backend.settings.placeboColorAdjustmentEnabled && (dialog.backend.settings.placeboColorAdjustmentPreset == 0)
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboColorAdjustmentEnabled && (dialog.backend.settings.placeboColorAdjustmentPreset == 0)
                                    C.Slider {
                                        id: placeboColorAdjustmentTemperature
                                        objectName: "placeboColorAdjustmentTemperature"
                                        Accessible.name: qsTr("Temperature:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: -1.143
                                        to: 5.286
                                        stepSize: 0.001
                                        value: dialog.backend.settings.placeboColorAdjustmentTemperature
                                        onMoved: dialog.backend.settings.placeboColorAdjustmentTemperature = value
                                    }
                                    Label { objectName: "placeboColorAdjustmentTemperatureValue"; text: qsTr(placeboColorAdjustmentTemperature.value.toFixed(3)); font.pixelSize: 14 }
                                }
                            }
                        }
                    }
                    ColumnLayout {
                        objectName: "rendererGroup5"
                        visible: dialog.currentSection === 5
                        Layout.fillWidth: true
                        spacing: 12
                        Label { text: qsTr("Peak Detection"); font.pixelSize: 16; font.bold: true }
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
                                    text: qsTr("HDR Peak Detection Enabled:") + "\n" + qsTr("(Checked)")
                                    visible: true
                                }
                                C.CheckBox {
                                    id: placeboPeakDetectionEnabled
                                    objectName: "placeboPeakDetectionEnabled"
                                    Accessible.name: qsTr("HDR Peak Detection Enabled:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    font.pixelSize: 14
                                    text: qsTr("Enable HDR Peak Detection")
                                    checked: dialog.backend.settings.placeboPeakDetectionEnabled
                                    onToggled: dialog.backend.settings.placeboPeakDetectionEnabled = checked
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Peak Detection Preset:") + "\n" + qsTr("(High Quality)")
                                    visible: dialog.backend.settings.placeboPeakDetectionEnabled
                                }
                                C.ComboBox {
                                    id: placeboPeakDetectionPreset
                                    objectName: "placeboPeakDetectionPreset"
                                    Accessible.name: qsTr("Peak Detection Preset:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredWidth: 400
                                    popup.width: width
                                    font.pixelSize: 14
                                    model: [qsTr("Custom"), qsTr("Default"), qsTr("High Quality")]
                                    currentIndex: dialog.backend.settings.placeboPeakDetectionPreset
                                    onActivated: index => dialog.backend.settings.placeboPeakDetectionPreset = index
                                    visible: dialog.backend.settings.placeboPeakDetectionEnabled
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Peak Smoothing Period:") + "\n" + qsTr("(20.0)")
                                    visible: dialog.backend.settings.placeboPeakDetectionEnabled && (dialog.backend.settings.placeboPeakDetectionPreset == 0)
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboPeakDetectionEnabled && (dialog.backend.settings.placeboPeakDetectionPreset == 0)
                                    C.Slider {
                                        id: placeboPeakDetectionPeakSmoothingPeriod
                                        objectName: "placeboPeakDetectionPeakSmoothingPeriod"
                                        Accessible.name: qsTr("Peak Smoothing Period:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.0
                                        to: 1000.0
                                        stepSize: 0.1
                                        value: dialog.backend.settings.placeboPeakDetectionPeakSmoothingPeriod
                                        onMoved: dialog.backend.settings.placeboPeakDetectionPeakSmoothingPeriod = value
                                    }
                                    Label { objectName: "placeboPeakDetectionPeakSmoothingPeriodValue"; text: qsTr(placeboPeakDetectionPeakSmoothingPeriod.value.toFixed(1)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Scene Threshold Low:") + "\n" + qsTr("(1.0)")
                                    visible: dialog.backend.settings.placeboPeakDetectionEnabled && (dialog.backend.settings.placeboPeakDetectionPreset == 0)
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboPeakDetectionEnabled && (dialog.backend.settings.placeboPeakDetectionPreset == 0)
                                    C.Slider {
                                        id: placeboPeakDetectionSceneThresholdLow
                                        objectName: "placeboPeakDetectionSceneThresholdLow"
                                        Accessible.name: qsTr("Scene Threshold Low:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.0
                                        to: 100.0
                                        stepSize: 0.1
                                        value: dialog.backend.settings.placeboPeakDetectionSceneThresholdLow
                                        onMoved: dialog.backend.settings.placeboPeakDetectionSceneThresholdLow = value
                                    }
                                    Label { objectName: "placeboPeakDetectionSceneThresholdLowValue"; text: qsTr(placeboPeakDetectionSceneThresholdLow.value.toFixed(1)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Scene Threshold High:") + "\n" + qsTr("(3.0)")
                                    visible: dialog.backend.settings.placeboPeakDetectionEnabled && (dialog.backend.settings.placeboPeakDetectionPreset == 0)
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboPeakDetectionEnabled && (dialog.backend.settings.placeboPeakDetectionPreset == 0)
                                    C.Slider {
                                        id: placeboPeakDetectionSceneThresholdHigh
                                        objectName: "placeboPeakDetectionSceneThresholdHigh"
                                        Accessible.name: qsTr("Scene Threshold High:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.0
                                        to: 100.0
                                        stepSize: 0.1
                                        value: dialog.backend.settings.placeboPeakDetectionSceneThresholdHigh
                                        onMoved: dialog.backend.settings.placeboPeakDetectionSceneThresholdHigh = value
                                    }
                                    Label { objectName: "placeboPeakDetectionSceneThresholdHighValue"; text: qsTr(placeboPeakDetectionSceneThresholdHigh.value.toFixed(1)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Peak Percentile:") + "\n" + qsTr("(99.995)")
                                    visible: dialog.backend.settings.placeboPeakDetectionEnabled && (dialog.backend.settings.placeboPeakDetectionPreset == 0)
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboPeakDetectionEnabled && (dialog.backend.settings.placeboPeakDetectionPreset == 0)
                                    C.Slider {
                                        id: placeboPeakDetectionPeakPercentile
                                        objectName: "placeboPeakDetectionPeakPercentile"
                                        Accessible.name: qsTr("Peak Percentile:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.0
                                        to: 100.0
                                        stepSize: 0.001
                                        value: dialog.backend.settings.placeboPeakDetectionPeakPercentile
                                        onMoved: dialog.backend.settings.placeboPeakDetectionPeakPercentile = value
                                    }
                                    Label { objectName: "placeboPeakDetectionPeakPercentileValue"; text: qsTr(placeboPeakDetectionPeakPercentile.value.toFixed(3)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Black Cutoff:") + "\n" + qsTr("(1.0)")
                                    visible: dialog.backend.settings.placeboPeakDetectionEnabled && (dialog.backend.settings.placeboPeakDetectionPreset == 0)
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.placeboPeakDetectionEnabled && (dialog.backend.settings.placeboPeakDetectionPreset == 0)
                                    C.Slider {
                                        id: placeboPeakDetectionBlackCutoff
                                        objectName: "placeboPeakDetectionBlackCutoff"
                                        Accessible.name: qsTr("Black Cutoff:")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        from: 0.0
                                        to: 100.0
                                        stepSize: 0.1
                                        value: dialog.backend.settings.placeboPeakDetectionBlackCutoff
                                        onMoved: dialog.backend.settings.placeboPeakDetectionBlackCutoff = value
                                    }
                                    Label { objectName: "placeboPeakDetectionBlackCutoffValue"; text: qsTr(placeboPeakDetectionBlackCutoff.value.toFixed(1)); font.pixelSize: 14 }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Allow Delayed Peak:") + "\n" + qsTr("(Unchecked)")
                                    visible: dialog.backend.settings.placeboPeakDetectionEnabled && (dialog.backend.settings.placeboPeakDetectionPreset == 0)
                                }
                                C.CheckBox {
                                    id: placeboPeakDetectionAllowDelayedPeak
                                    objectName: "placeboPeakDetectionAllowDelayedPeak"
                                    Accessible.name: qsTr("Allow Delayed Peak:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    font.pixelSize: 14
                                    text: qsTr("Enable 1-Frame Delayed Peak")
                                    checked: dialog.backend.settings.placeboPeakDetectionAllowDelayedPeak
                                    onToggled: dialog.backend.settings.placeboPeakDetectionAllowDelayedPeak = checked
                                    visible: dialog.backend.settings.placeboPeakDetectionEnabled && (dialog.backend.settings.placeboPeakDetectionPreset == 0)
                                }
                            }
                        }
                    }
                    ColumnLayout {
                        objectName: "rendererGroup6"
                        visible: dialog.currentSection === 6
                        Layout.fillWidth: true
                        spacing: 12
                        Label { text: qsTr("Color Mapping"); font.pixelSize: 16; font.bold: true }
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
                                    text: qsTr("Color Mapping Enabled:") + "\n" + qsTr("(Checked)")
                                    visible: true
                                }
                                C.CheckBox {
                                    id: placeboColorMappingEnabled
                                    objectName: "placeboColorMappingEnabled"
                                    Accessible.name: qsTr("Color Mapping Enabled:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    font.pixelSize: 14
                                    text: qsTr("Use Color Mapping Settings")
                                    checked: dialog.backend.settings.placeboColorMappingEnabled
                                    onToggled: dialog.backend.settings.placeboColorMappingEnabled = checked
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Color Mapping Preset:") + "\n" + qsTr("(High Quality)")
                                    visible: dialog.backend.settings.placeboColorMappingEnabled
                                }
                                C.ComboBox {
                                    id: placeboColorMappingPreset
                                    objectName: "placeboColorMappingPreset"
                                    Accessible.name: qsTr("Color Mapping Preset:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredWidth: 400
                                    popup.width: width
                                    font.pixelSize: 14
                                    model: [qsTr("Custom"), qsTr("Default"), qsTr("High Quality")]
                                    currentIndex: dialog.backend.settings.placeboColorMappingPreset
                                    onActivated: index => dialog.backend.settings.placeboColorMappingPreset = index
                                    visible: dialog.backend.settings.placeboColorMappingEnabled
                                }
                                Label {
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 220
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Color Mapping Settings:")
                                    visible: dialog.backend.settings.placeboColorMappingEnabled && dialog.backend.settings.placeboColorMappingPreset == 0
                                }
                                C.Button {
                                    id: openPlaceboColorMapping
                                    objectName: "openPlaceboColorMapping"
                                    Accessible.name: qsTr("Color Mapping Settings:")
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    font.pixelSize: 14
                                    text: qsTr("Open")
                                    onClicked: dialog.navigation.showPlaceboColorMappingDialog()
                                    visible: dialog.backend.settings.placeboColorMappingEnabled && dialog.backend.settings.placeboColorMappingPreset == 0
                                    Material.roundedScale: Material.SmallScale
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
