import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Window

import org.streetpea.chiaking

FocusScope {
    id: content
    required property var backend
    required property var settings
    required property var streamWindow
    required property var navigation
    required property var menuController
    property bool separate: false
    property Item initialFocusItem: resumeButton
    readonly property int presetIndex: Math.max(0, presetRepeater.model.findIndex(preset => preset.preset === streamWindow.videoPreset))
    implicitHeight: toolbar.implicitHeight + 24 + (keyboardMouseSettingsButton.visible ? 44 : 0)
    focus: true
    anchors.fill: parent
    Keys.onMenuPressed: menuController.close()
    Keys.onEscapePressed: menuController.dismiss()

    component Flyout: Popup {
        id: popup
        property Item opener
        width: Math.min(320, content.width)
        parent: content.separate ? nativeWindow.contentItem : content
        x: content.separate ? 0 : content.width - width
        y: content.separate ? 0 : -height - 9
        padding: 17
        modal: false
        dim: false
        focus: true
        closePolicy: Popup.NoAutoClose
        enter: Transition {}
        exit: Transition {}
        background: Rectangle { radius: 8; color: "#a6090e15"; border.color: "#24ffffff" }
        function focusItem(item) {
            if (content.separate) nativeWindow.requestActivate();
            item.forceActiveFocus(Qt.TabFocusReason);
        }
        property Window nativeHost: Window {
            id: nativeWindow
            objectName: "streamMenuPopupWindow"
            transientParent: content.Window.window
            flags: Qt.Tool | Qt.FramelessWindowHint
            color: "transparent"
            visible: content.separate && popup.visible
            x: content.Window.window ? content.Window.window.x + content.width - width : 0
            y: content.Window.window ? content.Window.window.y - height - 9 : 0
            width: popup.width
            height: popup.height
            onVisibleChanged: if (visible) requestActivate()
            Shortcut {
                enabled: nativeWindow.visible && menuController.open
                sequence: "Ctrl+O"
                onActivated: menuController.close()
            }
            property Item lastFocusItem: null
            onActiveFocusItemChanged: {
                var item = activeFocusItem;
                while (item && item !== popup.contentItem) item = item.parent;
                if (item && activeFocusItem !== popup.contentItem) lastFocusItem = activeFocusItem;
            }
        }
        onClosed: Qt.callLater(() => {
            if (menuController.open && !menuController.flyout) {
                if (content.separate) content.Window.window.requestActivate();
                opener.forceActiveFocus(Qt.TabFocusReason);
            }
        })
    }

    component MenuButton: ToolButton {
        property string description
        property bool disclosure: false
        font.pixelSize: 14
        padding: 9
        Layout.fillWidth: true
        Layout.fillHeight: true
        Accessible.name: description + ": " + text
        contentItem: ColumnLayout {
            spacing: 5
            Label { text: description; color: "#dbe1e9"; font.pixelSize: 11; wrapMode: Text.Wrap; Layout.fillWidth: true }
            Label { text: parent.parent.text + (parent.parent.disclosure ? "  ▴" : ""); color: parent.parent.enabled ? "white" : "#aab6c6"; font.pixelSize: 14; wrapMode: Text.Wrap; Layout.fillWidth: true; Accessible.ignored: true }
        }
        background: Rectangle {
            radius: 5
            color: parent.hovered || parent.down ? "#33ffffff" : "transparent"
            border.color: parent.activeFocus ? "#a9d9ff" : "transparent"
            border.width: 2
        }
        Keys.onReturnPressed: clicked()
    }

    component FlyoutButton: ToolButton {
        font.pixelSize: 14
        Layout.fillWidth: true
        background: Rectangle {
            radius: 5
            color: parent.checked ? "#447cadd6" : parent.hovered || parent.down ? "#33ffffff" : "transparent"
            border.color: parent.activeFocus ? "#a9d9ff" : "transparent"
            border.width: 2
        }
        contentItem: Label {
            text: parent.text
            color: "white"
            font: parent.font
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
        Keys.onReturnPressed: clicked()
    }

    Rectangle {
        anchors.fill: parent
        color: "#a6090e15"
        radius: 8
        border.color: "#24ffffff"
    }
    RowLayout {
        id: toolbar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 12
        height: implicitHeight
        spacing: 8
        MenuButton {
            id: resumeButton
            objectName: "streamResume"
            description: qsTr("Back to game")
            text: qsTr("Resume")
            Layout.preferredWidth: 90
            Layout.minimumHeight: 54
            onClicked: menuController.close()
            KeyNavigation.right: volumeSlider
            KeyNavigation.tab: volumeSlider
        }
        ColumnLayout {
            Layout.preferredWidth: 164
            Layout.fillWidth: true
            spacing: 1
            RowLayout {
                Layout.fillWidth: true
                Label { objectName: "streamVolumeLabel"; text: qsTr("Volume"); color: "#dbe1e9"; font.pixelSize: 11; wrapMode: Text.Wrap; Layout.fillWidth: true; Layout.minimumWidth: 0 }
                Label { objectName: "streamVolumePercentage"; text: (volumeSlider.value / 128 * 100).toFixed(0) + "%"; color: "white"; font.pixelSize: 13 }
            }
            Slider {
                id: volumeSlider
                objectName: "streamVolume"
                Layout.fillWidth: true
                implicitHeight: 22
                from: 0; to: 128; stepSize: 1
                value: settings.audioVolume
                onMoved: settings.audioVolume = value
                Accessible.name: qsTr("Volume")
                Material.accent: "#d5eaff"
                Keys.onUpPressed: { increase(); moved(); }
                Keys.onDownPressed: { decrease(); moved(); }
                KeyNavigation.priority: KeyNavigation.BeforeItem
                KeyNavigation.left: resumeButton
                KeyNavigation.right: muteButton
                KeyNavigation.tab: muteButton
                KeyNavigation.backtab: resumeButton
            }
        }
        MenuButton {
            id: muteButton
            objectName: "streamMicrophone"
            description: qsTr("Microphone")
            text: checked ? qsTr("On") : qsTr("Muted")
            Layout.preferredWidth: 106
            checkable: true
            enabled: !!backend.session && backend.session.connected
            checked: !!backend.session && !backend.session.muted
            onClicked: { if (backend.session && backend.session.connected) backend.session.muted = !backend.session.muted; }
            KeyNavigation.left: volumeSlider
            KeyNavigation.right: pictureButton
            KeyNavigation.tab: pictureButton
            KeyNavigation.backtab: volumeSlider
        }
        MenuButton {
            id: pictureButton
            objectName: "streamPicture"
            description: qsTr("Picture mode")
            disclosure: true
            text: streamWindow.videoMode === ChiakiWindow.VideoMode.Zoom ? qsTr("Zoom") : streamWindow.videoMode === ChiakiWindow.VideoMode.Stretch ? qsTr("Stretch") : qsTr("Fit")
            Layout.preferredWidth: 106
            onClicked: menuController.flyout = menuController.flyout === "picture" ? "" : "picture"
            KeyNavigation.left: muteButton
            KeyNavigation.right: qualityButton
            KeyNavigation.tab: qualityButton
            KeyNavigation.backtab: muteButton
        }
        MenuButton {
            id: qualityButton
            objectName: "streamQuality"
            description: qsTr("Quality preset")
            disclosure: true
            text: presetRepeater.model[content.presetIndex].label
            Layout.preferredWidth: 178
            onClicked: menuController.flyout = menuController.flyout === "quality" ? "" : "quality"
            KeyNavigation.left: pictureButton
            KeyNavigation.right: disconnectButton
            KeyNavigation.tab: disconnectButton
            KeyNavigation.backtab: pictureButton
        }
        MenuButton {
            id: disconnectButton
            objectName: "streamDisconnect"
            description: qsTr("End session")
            text: qsTr("Disconnect")
            Layout.preferredWidth: 112
            onClicked: {
                menuController.close();
                if (backend.session) streamWindow.close();
                else navigation.showMainView();
            }
            KeyNavigation.left: qualityButton
            KeyNavigation.tab: keyboardMouseSettingsButton.visible ? keyboardMouseSettingsButton : resumeButton
            KeyNavigation.backtab: qualityButton
        }
    }

    FlyoutButton {
        id: keyboardMouseSettingsButton
        objectName: "streamKeyboardMouseSettings"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 12
        height: 36
        visible: !!settings.mouseCameraSupported
        text: qsTr("Keyboard and mouse settings")
        Accessible.name: text
        onClicked: menuController.openSettings("keyboard-mouse")
        KeyNavigation.up: disconnectButton
        KeyNavigation.backtab: disconnectButton
        KeyNavigation.tab: resumeButton
    }

    Flyout {
        id: picturePopup
        opener: pictureButton
        objectName: "streamPicturePopup"
        visible: menuController.open && menuController.flyout === "picture"
        onOpened: focusItem(fitButton)
        contentItem: ColumnLayout {
            Keys.onEscapePressed: menuController.dismiss()
            Keys.onMenuPressed: menuController.close()
            spacing: 9
            Label { text: qsTr("Picture mode"); color: "white"; font.pixelSize: 14; font.bold: true }
            RowLayout {
                Layout.fillWidth: true
                FlyoutButton {
                    id: fitButton
                    objectName: "streamFit"
                    text: qsTr("Fit")
                    Layout.fillWidth: true
                    checkable: true
                    checked: streamWindow.videoMode === ChiakiWindow.VideoMode.Normal
                    onClicked: streamWindow.videoMode = ChiakiWindow.VideoMode.Normal
                    KeyNavigation.right: zoomButton
                    KeyNavigation.down: zoomFactor.visible ? zoomFactor : displaySettingsButton
                }
                FlyoutButton {
                    id: zoomButton
                    objectName: "streamZoom"
                    text: qsTr("Zoom")
                    Layout.fillWidth: true
                    checkable: true
                    checked: streamWindow.videoMode === ChiakiWindow.VideoMode.Zoom
                    onClicked: streamWindow.videoMode = ChiakiWindow.VideoMode.Zoom
                    KeyNavigation.left: fitButton
                    KeyNavigation.right: stretchButton
                    KeyNavigation.down: zoomFactor.visible ? zoomFactor : displaySettingsButton
                }
                FlyoutButton {
                    id: stretchButton
                    objectName: "streamStretch"
                    text: qsTr("Stretch")
                    Layout.fillWidth: true
                    checkable: true
                    checked: streamWindow.videoMode === ChiakiWindow.VideoMode.Stretch
                    onClicked: streamWindow.videoMode = ChiakiWindow.VideoMode.Stretch
                    KeyNavigation.left: zoomButton
                    KeyNavigation.down: displaySettingsButton
                }
            }
            Label {
                text: qsTr("Fit keeps the picture proportions. Zoom fills the window. Stretch can change proportions.")
                color: "#e0e5ed"; font.pixelSize: 12
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
            RowLayout {
                visible: streamWindow.videoMode === ChiakiWindow.VideoMode.Zoom
                Layout.fillWidth: true
                Slider {
                    id: zoomFactor
                    objectName: "streamZoomFactor"
                    Layout.fillWidth: true
                    from: -1; to: 4; stepSize: 0.01
                    value: streamWindow.ZoomFactor
                    onMoved: { streamWindow.ZoomFactor = value; settings.sZoomFactor = value; }
                    Accessible.name: qsTr("Zoom")
                    Keys.onUpPressed: { increase(); moved(); }
                    Keys.onDownPressed: { decrease(); moved(); }
                    KeyNavigation.priority: KeyNavigation.BeforeItem
                    KeyNavigation.left: zoomButton
                    KeyNavigation.right: displaySettingsButton
                }
                Label {
                    objectName: "streamZoomValue"
                    text: zoomFactor.value === -1 ? qsTr("No black bars") : (zoomFactor.value >= 0 ? (zoomFactor.value + 1).toFixed(2) : zoomFactor.value.toFixed(2)) + qsTr(" x")
                    color: "white"; font.pixelSize: 12
                }
            }
            FlyoutButton {
                id: displaySettingsButton
                objectName: "streamDisplaySettings"
                text: qsTr("Display settings")
                Layout.fillWidth: true
                onClicked: menuController.openSettings("display")
                KeyNavigation.up: zoomFactor.visible ? zoomFactor : fitButton
                KeyNavigation.down: fitButton
            }
        }
    }

    Flyout {
        id: qualityPopup
        objectName: "streamQualityPopup"
        opener: qualityButton
        visible: menuController.open && menuController.flyout === "quality"
        onOpened: focusItem(presetRepeater.itemAt(content.presetIndex))
        contentItem: ColumnLayout {
            spacing: 4
            Keys.onEscapePressed: menuController.dismiss()
            Keys.onMenuPressed: menuController.close()
            Label { text: qsTr("Quality preset"); color: "white"; font.pixelSize: 14; font.bold: true }
            Repeater {
                id: presetRepeater
                model: [
                    {name: "streamDefault", label: qsTr("Default"), preset: ChiakiWindow.VideoPreset.Default},
                    {name: "streamHighQuality", label: qsTr("High quality"), preset: ChiakiWindow.VideoPreset.HighQuality},
                    {name: "streamSpatial", label: qsTr("HQ + Spatial"), preset: ChiakiWindow.VideoPreset.HighQualitySpatial},
                    {name: "streamAdvancedSpatial", label: qsTr("HQ + Advanced spatial"), preset: ChiakiWindow.VideoPreset.HighQualityAdvancedSpatial},
                    {name: "streamCustom", label: qsTr("Custom"), preset: ChiakiWindow.VideoPreset.Custom}
                ]
                delegate: FlyoutButton {
                    required property var modelData
                    required property int index
                    objectName: modelData.name
                    text: modelData.label
                    Layout.fillWidth: true
                    checkable: true
                    checked: streamWindow.videoPreset === modelData.preset
                    onClicked: { streamWindow.videoPreset = modelData.preset; settings.videoPreset = modelData.preset; }
                    KeyNavigation.up: index > 0 ? presetRepeater.itemAt(index - 1) : rendererSettingsButton.visible ? rendererSettingsButton : presetRepeater.itemAt(4)
                    KeyNavigation.down: index < 4 ? presetRepeater.itemAt(index + 1) : rendererSettingsButton.visible ? rendererSettingsButton : presetRepeater.itemAt(0)
                }
            }
            FlyoutButton {
                id: rendererSettingsButton
                objectName: "streamRendererSettings"
                text: qsTr("Renderer settings")
                Layout.fillWidth: true
                visible: streamWindow.videoPreset === ChiakiWindow.VideoPreset.Custom
                onClicked: menuController.openSettings("renderer")
                KeyNavigation.up: presetRepeater.itemAt(4)
                KeyNavigation.down: presetRepeater.itemAt(0)
            }
        }
    }
}
