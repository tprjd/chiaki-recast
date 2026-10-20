import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs

import org.streetpea.chiaking

import "controls" as C

FocusScope {
    enum Console {
        PS4,
        PS5
    }
    property int selectedConsole: SettingsDialog.Console.PS5
    property bool quitControllerMapping: true
    id: dialog
    objectName: "settingsDialog"
    property var backend: Chiaki
    property var navigation: root
    property var openExternal: function(url) { Qt.openUrlExternally(url); }
    property alias currentCategory: bar.currentIndex
    property Item restoreFocusItem
    readonly property bool compact: width < 760
    readonly property var categories: [qsTr("General"), qsTr("Video"), qsTr("Stream"),
        qsTr("Audio/Wifi"), qsTr("Consoles"), qsTr("Keys"), qsTr("Controllers"), qsTr("Remote"), qsTr("Config")]
    function close() { navigation.closeDialog(); }
    Keys.onEscapePressed: close()
    StackView.onDeactivating: restoreFocusItem = Window.window.activeFocusItem
    StackView.onActivated: {
        if (restoreFocusItem && restoreFocusItem.visible && restoreFocusItem.enabled)
            restoreFocusItem.forceActiveFocus(Qt.TabFocusReason);
        else
            focusCurrentTabFirstItem();
        restoreFocusItem = null;
    }
    function flickContainsItem(flick, item) {
        let current = item;
        while (current) {
            if (current === flick || current === flick.contentItem)
                return true;
            current = current.parent;
        }
        return false;
    }
    function flickVisibilityTarget(flick, item) {
        let current = item;
        while (current && current !== flick && current !== flick.contentItem) {
            if (current.parent === flick.contentItem)
                return current;
            if (current.parent && current.parent.parent === flick.contentItem)
                return current;
            current = current.parent;
        }
        return item;
    }
    function ensureItemVisibleInFlick(flick, item) {
        if (!flick || !item || !flickContainsItem(flick, item))
            return;
        const target = flickVisibilityTarget(flick, item);
        const top = target.mapToItem(flick.contentItem, 0, 0).y;
        const bottom = top + target.height;
        if (top < flick.contentY)
            flick.contentY = top;
        else if (bottom > flick.contentY + flick.height)
            flick.contentY = bottom - flick.height;
    }
    function itemIsInsideNestedScrollable(flick, item) {
        let current = item ? item.parent : null;
        while (current && current !== flick && current !== flick.contentItem) {
            if (current.contentY !== undefined && current.contentHeight !== undefined && current !== flick)
                return true;
            current = current.parent;
        }
        return false;
    }
    function nestedScrollableTarget(flick, item) {
        let current = item;
        while (current && current !== flick && current !== flick.contentItem) {
            if (current.contentY !== undefined && current.contentHeight !== undefined && current !== flick)
                return current;
            current = current.parent;
        }
        return item;
    }
    function ensureActiveFocusVisible() {
        const flick = activeSettingsFlick();
        const window = dialog.Window.window;
        if (!flick || !window || !window.activeFocusItem)
            return;
        const target = itemIsInsideNestedScrollable(flick, window.activeFocusItem)
            ? nestedScrollableTarget(flick, window.activeFocusItem)
            : window.activeFocusItem;
        ensureItemVisibleInFlick(flick, target);
    }
    function scrollFlickKeepingItemVisible(flick, item, delta) {
        if (!flick || !item)
            return;
        const maxContentY = Math.max(0, flick.contentHeight - flick.height);
        if (maxContentY <= 0)
            return;
        flick.contentY = Math.max(0, Math.min(maxContentY, flick.contentY + delta));
    }
    function focusCurrentTabFirstItem() {
        let item = null;
        switch (bar.currentIndex) {
        case 0: item = disconnectAction; break;
        case 1: item = hwDecoderCombo; break;
        case 2: item = consoleSelection; break;
        case 3: item = audioOutDevice; break;
        case 4: item = registerNewButton; break;
        case 5: item = resetAllKeys; break;
        case 6: item = controllerMappingChange; break;
        case 7: item = firstRemoteFocusableItem(); break;
        case 8: item = profile; break;
        }
        if (item)
            item.forceActiveFocus(Qt.TabFocusReason);
    }
    function activeSettingsFlick() {
        switch (bar.currentIndex) {
        case 0: return generalFlick;
        case 1: return videoFlick;
        case 2: return streamFlick;
        case 3: return audiowifiFlick;
        case 4: return consolesFlick;
        case 5: return keysFlick;
        case 6: return controllersFlick;
        case 7: return remoteFlick;
        case 8: return configFlick;
        default: return null;
        }
    }

    function firstRemoteFocusableItem() {
        if (openPsnLogin.visible)
            return openPsnLogin;
        if (resetPsnTokens.visible)
            return resetPsnTokens;
        if (holePunchGuessingCheckbox.visible)
            return holePunchGuessingCheckbox;
        if (portGuessCountSlider.visible)
            return portGuessCountSlider;
        if (portGuessSocketSlider.visible)
            return portGuessSocketSlider;
        return remoteFlick;
    }
    Keys.onPressed: (event) => {
        if (event.modifiers)
            return;
        switch (event.key) {
        case Qt.Key_PageUp:
            bar.decrementCurrentIndex();
            event.accepted = true;
            break;
        case Qt.Key_PageDown:
            bar.incrementCurrentIndex();
            event.accepted = true;
            break;
        case Qt.Key_Up:
        {
            const flick = activeSettingsFlick();
            if (!flick || flick.contentHeight <= flick.height || flick.contentY <= 0.001)
                return;
            flick.flick(0, 500);
            event.accepted = true;
            break;
        }
        case Qt.Key_Down:
        {
            const flick = activeSettingsFlick();
            if (!flick || flick.contentHeight <= flick.height || flick.contentY >= flick.contentHeight - flick.height - 0.001)
                return;
            flick.flick(0, -500);
            event.accepted = true;
            break;
        }
        }
    }

    Rectangle { anchors.fill: parent; color: "#11151c" }
    Item {
        anchors.fill: parent
        anchors.margins: dialog.compact ? 12 : 24

        RowLayout {
            id: heading
            anchors { top: parent.top; left: parent.left; right: parent.right }
            spacing: 20
            C.Button {
                objectName: "settingsBack"
                text: qsTr("Consoles")
                Accessible.name: qsTr("Back to consoles")
                onClicked: dialog.close()
            }
            Label {
                text: qsTr("Settings")
                font.pixelSize: dialog.compact ? 24 : 30
                font.bold: true
                Layout.fillWidth: true
            }
            Label {
                objectName: "settingsProfile"
                visible: !dialog.compact
                text: dialog.backend.settings.streamerMode ? qsTr("Profile hidden")
                    : qsTr("Profile: %1").arg(dialog.backend.settings.currentProfile || qsTr("default"))
                textFormat: Text.PlainText
                elide: Text.ElideRight
                Layout.maximumWidth: 250
            }
        }
        Label {
            id: profileCompact
            anchors { top: heading.bottom; left: parent.left; right: parent.right }
            visible: dialog.compact
            text: dialog.backend.settings.streamerMode ? qsTr("Profile hidden")
                : qsTr("Profile: %1").arg(dialog.backend.settings.currentProfile || qsTr("default"))
            textFormat: Text.PlainText
            elide: Text.ElideRight
        }
        QtObject {
            id: bar
            property int currentIndex: 0
            function decrementCurrentIndex() { currentIndex = Math.max(0, currentIndex - 1); }
            function incrementCurrentIndex() { currentIndex = Math.min(8, currentIndex + 1); }
        }
        C.ComboBox {
            id: categorySelector
            objectName: "settingsCategorySelector"
            anchors { top: profileCompact.bottom; left: parent.left; right: parent.right; topMargin: 8 }
            visible: dialog.compact
            model: dialog.categories
            currentIndex: bar.currentIndex
            Accessible.name: qsTr("Settings category")
            onActivated: index => bar.currentIndex = index
        }
        Flickable {
            id: sidebar
            anchors { top: heading.bottom; left: parent.left; bottom: parent.bottom; topMargin: 24 }
            width: 180
            visible: !dialog.compact
            contentHeight: categoryColumn.height
            clip: true
            ScrollBar.vertical: ScrollBar {}
            ColumnLayout {
                id: categoryColumn
                width: parent.width
                spacing: 4
                Label { text: qsTr("Categories"); opacity: 0.65; bottomPadding: 10 }
                Repeater {
                    model: dialog.categories
                    C.Button {
                        required property int index
                        required property string modelData
                        objectName: "settingsCategory" + index
                        text: modelData
                        Layout.fillWidth: true
                        checkable: true
                        autoExclusive: true
                        checked: bar.currentIndex === index
                        highlighted: checked
                        font.pixelSize: 16
                        Layout.preferredHeight: 44
                        background: Rectangle {
                            radius: 7
                            color: parent.checked ? "#24394c" : parent.hovered ? "#1b2633" : "transparent"
                            border.color: parent.visualFocus ? Material.accent : "transparent"
                            border.width: 2
                        }
                        Accessible.name: modelData
                        Accessible.description: highlighted ? qsTr("Selected category") : ""
                        onClicked: bar.currentIndex = index
                        onActiveFocusChanged: if (activeFocus) dialog.ensureItemVisibleInFlick(sidebar, this)
                    }
                }
                Label {
                    text: qsTr("L1 / R1: Categories")
                    font.pixelSize: 12
                    opacity: 0.65
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    topPadding: 16
                }
            }
        }
        StackLayout {
            anchors {
                top: dialog.compact ? categorySelector.bottom : heading.bottom
                left: dialog.compact ? parent.left : sidebar.right
                right: parent.right
                bottom: parent.bottom
                leftMargin: dialog.compact ? 0 : 24
                topMargin: 20
            }
            currentIndex: bar.currentIndex
            onCurrentIndexChanged: {
                dialog.focusCurrentTabFirstItem()
                Qt.callLater(dialog.ensureActiveFocusVisible)
            }

            Item {
                // General
                Flickable {
                    id: generalFlick
                    objectName: "generalFlick"
                    anchors.fill: parent
                    clip: true
                    contentWidth: width
                    contentHeight: generalLayout.height
                    flickableDirection: Flickable.VerticalFlick
                    ScrollBar.vertical: ScrollBar {}
                    ColumnLayout {
                        id: generalLayout
                        width: Math.max(0, generalFlick.width - 14)
                        spacing: 16
                        Label { text: qsTr("General"); font.pixelSize: 24; font.bold: true }
                        Label { font.pixelSize: 14; text: qsTr("Session behavior"); font.bold: true }
                        Frame {
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            Layout.fillWidth: true
                            GridLayout {
                                anchors.fill: parent
                                columns: generalFlick.width < 650 ? 1 : 2
                                columnSpacing: 24
                                rowSpacing: 12
                                Label { font.pixelSize: 14; text: qsTr("Action on disconnect\nDefault: Ask"); wrapMode: Text.Wrap; Layout.fillWidth: true }
                                C.ComboBox {
                                    font.pixelSize: 14
                                    id: disconnectAction
                                    objectName: "disconnectAction"
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredWidth: 300
                                    firstInFocusChain: true
                                    Accessible.name: qsTr("Action on disconnect")
                                    model: [qsTr("Do Nothing"), qsTr("Enter Sleep Mode"), qsTr("Ask")]
                                    currentIndex: dialog.backend.settings.disconnectAction
                                    onActivated: index => dialog.backend.settings.disconnectAction = index
                                }
                                Label { font.pixelSize: 14; text: qsTr("Action on suspend\nDefault: Do Nothing"); wrapMode: Text.Wrap; Layout.fillWidth: true }
                                C.ComboBox {
                                    font.pixelSize: 14
                                    objectName: "suspendAction"
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredWidth: 300
                                    Accessible.name: qsTr("Action on suspend")
                                    model: [qsTr("Do Nothing"), qsTr("Enter Sleep Mode")]
                                    currentIndex: dialog.backend.settings.suspendAction
                                    onActivated: index => dialog.backend.settings.suspendAction = index
                                }
                                Label { font.pixelSize: 14; text: qsTr("Audio/Video\nDefault: Audio and Video Enabled"); wrapMode: Text.Wrap; Layout.fillWidth: true }
                                C.ComboBox {
                                    font.pixelSize: 14
                                    objectName: "audioVideoDisabled"
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredWidth: 300
                                    Accessible.name: qsTr("Audio/Video")
                                    model: [qsTr("Audio and Video Enabled"), qsTr("Audio Disabled"), qsTr("Video Disabled"), qsTr("Audio and Video Disabled")]
                                    currentIndex: dialog.backend.settings.audioVideoDisabled
                                    onActivated: index => dialog.backend.settings.audioVideoDisabled = index
                                }
                            }
                        }
                        Label { font.pixelSize: 14;
                            text: qsTr("Steam Deck")
                            font.bold: true
                            visible: typeof dialog.backend.settings.steamDeckHaptics !== "undefined" || typeof dialog.backend.settings.verticalDeck !== "undefined"
                        }
                        Frame {
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            Layout.fillWidth: true
                            visible: typeof dialog.backend.settings.steamDeckHaptics !== "undefined" || typeof dialog.backend.settings.verticalDeck !== "undefined"
                            ColumnLayout {
                                anchors.fill: parent
                                C.CheckBox {
                                    font.pixelSize: 14
                                    objectName: "steamDeckHaptics"
                                    Layout.fillWidth: true
                                    text: qsTr("True haptics for Steam Deck, better quality but noisier")
                                    contentItem: Label { font.pixelSize: 14; text: parent.text; wrapMode: Text.Wrap; leftPadding: parent.indicator.width + parent.spacing; verticalAlignment: Text.AlignVCenter }
                                    checked: dialog.backend.settings.steamDeckHaptics || false
                                    onToggled: dialog.backend.settings.steamDeckHaptics = checked
                                    visible: typeof dialog.backend.settings.steamDeckHaptics !== "undefined"
                                }
                                C.CheckBox {
                                    font.pixelSize: 14
                                    objectName: "verticalDeck"
                                    Layout.fillWidth: true
                                    text: qsTr("Use Steam Deck in vertical orientation for motion controls")
                                    contentItem: Label { font.pixelSize: 14; text: parent.text; wrapMode: Text.Wrap; leftPadding: parent.indicator.width + parent.spacing; verticalAlignment: Text.AlignVCenter }
                                    checked: dialog.backend.settings.verticalDeck || false
                                    onToggled: dialog.backend.settings.verticalDeck = checked
                                    visible: typeof dialog.backend.settings.verticalDeck !== "undefined"
                                }
                                Label { font.pixelSize: 14; text: qsTr("Default: Unchecked"); opacity: 0.65 }
                            }
                        }
                        Label { font.pixelSize: 14; text: qsTr("Privacy and logs"); font.bold: true }
                        Frame {
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            Layout.fillWidth: true
                            ColumnLayout {
                                anchors.fill: parent
                                C.CheckBox {
                                    font.pixelSize: 14
                                    id: streamerMode
                                    objectName: "streamerMode"
                                    text: qsTr("Streamer mode, hide private information")
                                    Layout.fillWidth: true
                                    contentItem: Label { font.pixelSize: 14; text: parent.text; wrapMode: Text.Wrap; leftPadding: parent.indicator.width + parent.spacing; verticalAlignment: Text.AlignVCenter }
                                    checked: dialog.backend.settings.streamerMode
                                    onToggled: dialog.backend.settings.streamerMode = checked
                                }
                                Label { font.pixelSize: 14; text: qsTr("Default: Unchecked"); opacity: 0.65 }
                                Label { font.pixelSize: 14;
                                    objectName: "logDirectory"
                                    Layout.fillWidth: true
                                    text: dialog.backend.settings.streamerMode ? qsTr("Log directory hidden") : dialog.backend.settings.logDirectory
                                    textFormat: Text.PlainText
                                    wrapMode: Text.WrapAnywhere
                                }
                                C.Button {
                                    font.pixelSize: 14
                                    objectName: "openLogDirectory"
                                    text: qsTr("Open log directory")
                                    onClicked: dialog.openExternal("file://" + dialog.backend.settings.logDirectory)
                                }
                            }
                        }
                        Label { font.pixelSize: 14; text: qsTr("Stream menu shortcut"); font.bold: true }
                        Frame {
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            Layout.fillWidth: true
                            ColumnLayout {
                                anchors.fill: parent
                                C.CheckBox {
                                    font.pixelSize: 14
                                    id: streamMenu
                                    objectName: "streamMenuEnabled"
                                    text: qsTr("Enable stream menu shortcut")
                                    Layout.fillWidth: true
                                    contentItem: Label { font.pixelSize: 14; text: parent.text; wrapMode: Text.Wrap; leftPadding: parent.indicator.width + parent.spacing; verticalAlignment: Text.AlignVCenter }
                                    checked: dialog.backend.settings.streamMenuEnabled
                                    onToggled: dialog.backend.settings.streamMenuEnabled = checked
                                }
                                Label { font.pixelSize: 14; text: qsTr("Default: Enabled, L1 + R1 + L3 + R3"); wrapMode: Text.Wrap; Layout.fillWidth: true; opacity: 0.65 }
                                GridLayout {
                                    columns: generalFlick.width < 650 ? 2 : 4
                                    Layout.fillWidth: true
                                    visible: dialog.backend.settings.streamMenuEnabled
                                    Repeater {
                                        model: 4
                                        C.ComboBox {
                                    font.pixelSize: 14
                                            required property int index
                                            objectName: "streamMenuShortcut" + (index + 1)
                                            Layout.fillWidth: true
                                            Layout.minimumWidth: 0
                                            Layout.preferredWidth: 150
                                            Accessible.name: qsTr("Stream menu shortcut button %1").arg(index + 1)
                                            model: [qsTr("Not Used"), qsTr("Cross"), qsTr("Moon"), qsTr("Box"), qsTr("Pyramid"), qsTr("Dpad Left"), qsTr("Dpad Right"), qsTr("Dpad Up"), qsTr("Dpad Down"), qsTr("L1"), qsTr("R1"), qsTr("L3"), qsTr("R3"), qsTr("Options"), qsTr("Share"), qsTr("Touchpad"), qsTr("PS")]
                                            currentIndex: dialog.backend.settings["streamMenuShortcut" + (index + 1)]
                                            onActivated: value => dialog.backend.settings["streamMenuShortcut" + (index + 1)] = value
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Item {
                // Video
                Flickable {
                    id: videoFlick
                    anchors {
                        fill: parent
                        topMargin: 20
                        bottomMargin: 20
                    }
                    clip: true
                    contentWidth: Math.max(width, videoGrid.width)
                    contentHeight: videoGrid.height
                    flickableDirection: Flickable.AutoFlickIfNeeded
                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AlwaysOn
                        visible: videoFlick.contentHeight > videoFlick.height
                    }
                    GridLayout {
                        id: videoGrid
                        anchors {
                            top: parent.top
                            horizontalCenter: parent.horizontalCenter
                        }
                        columns: 3
                        rowSpacing: 10
                        columnSpacing: 20

                        Label {
                            Layout.alignment: Qt.AlignRight
                            text: qsTr("Hardware Decoder:")
                        }

                    RowLayout {
                        Layout.preferredWidth: 400
                        spacing: 12

                        C.ComboBox {
                            id: hwDecoderCombo
                            Layout.preferredWidth: 220
                            model: dialog.backend.settings.availableDecoders
                            currentIndex: Math.max(0, model.indexOf(dialog.backend.settings.decoder))
                            KeyNavigation.priority: KeyNavigation.BeforeItem
                            KeyNavigation.up: hwDecoderCombo
                            KeyNavigation.right: zeroCopyCheck
                            KeyNavigation.down: windowTypeCombo
                            onActivated: (index) => dialog.backend.settings.decoder = index ? model[index] : ""
                        }

                        Label {
                            text: qsTr("Zero-Copy")
                        }

                        C.CheckBox {
                            id: zeroCopyCheck
                            KeyNavigation.priority: KeyNavigation.BeforeItem
                            KeyNavigation.up: zeroCopyCheck
                            KeyNavigation.left: hwDecoderCombo
                            KeyNavigation.down: windowTypeCombo
                            checked: dialog.backend.settings.useZeroCopy
                            onToggled: dialog.backend.settings.useZeroCopy = checked
                        }
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("(Auto / On)")
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("Window Type:")
                    }

                    C.ComboBox {
                        id: windowTypeCombo
                        Layout.preferredWidth: 400
                        popup.width: 500
                        model: [qsTr("Stream Resolution"), qsTr("Custom Resolution"), qsTr("Adjust Resolution Manually"), qsTr("Fullscreen"), qsTr("Zoom [adjust zoom using slider in stream menu]"), qsTr("Stretch")]
                        currentIndex: dialog.backend.settings.windowType
                        onActivated: (index) => dialog.backend.settings.windowType = index;
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("(Fullscreen)")
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("Custom Resolution Width")
                        visible: dialog.backend.settings.windowType == 1
                    }

                    C.TextField {
                        id: customResolutionWidth
                        Layout.preferredWidth: 400
                        visible: dialog.backend.settings.windowType == 1
                        text: dialog.backend.settings.customResolutionWidth
                        Material.accent: text && !validate() ? Material.Red : undefined
                        onEditingFinished: {
                            if (validate()) {
                                dialog.backend.settings.customResolutionWidth = parseInt(text);
                            } else {
                                dialog.backend.settings.customResolutionWidth = 0;
                                text = "";
                            }
                        }
                        function validate() {
                            var num = parseInt(text);
                            return num >= 0 && num <= 9999;
                        }
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("(1920)")
                        visible: dialog.backend.settings.windowType == 1
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("Custom Resolution Height")
                        visible: dialog.backend.settings.windowType == 1
                    }

                    C.TextField {
                        id: customResolutionHeight
                        Layout.preferredWidth: 400
                        visible: dialog.backend.settings.windowType == 1
                        text: dialog.backend.settings.customResolutionHeight
                        Material.accent: text && !validate() ? Material.Red : undefined
                        onEditingFinished: {
                            if (validate()) {
                                dialog.backend.settings.customResolutionHeight = parseInt(text);
                            } else {
                                dialog.backend.settings.customResolutionHeight = 0;
                                text = "";
                            }
                        }
                        function validate() {
                            var num = parseInt(text);
                            return num >= 0 && num <= 9999;
                        }
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("(1080)")
                        visible: dialog.backend.settings.windowType == 1
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("Toggle Fullscreen on Double-click:")
                    }

                    C.CheckBox {
                        checked: dialog.backend.settings.fullscreenDoubleClick
                        onToggled: dialog.backend.settings.fullscreenDoubleClick = checked
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("(Unchecked)")
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("Hide Cursor during Stream")
                    }
                    C.CheckBox {
                        checked: dialog.backend.settings.hideCursor
                        onToggled: dialog.backend.settings.hideCursor = !dialog.backend.settings.hideCursor
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("(Checked)")
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("Vertical Sync:")
                    }

                    C.CheckBox {
                        checked: dialog.backend.settings.vSyncEnabled
                        onToggled: {
                            dialog.backend.settings.vSyncEnabled = checked
                            if (dialog.backend.window.runtimeRendererBackend === 1)
                                dialog.backend.settings.restartApplication()
                        }
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("(Checked)")
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("Render Preset:")
                    }

                    C.ComboBox {
                        Layout.preferredWidth: 520
                        model: [qsTr("Fast"), qsTr("Default"), qsTr("High Quality"), qsTr("High Quality + Spatial Upscaling"), qsTr("High Quality + Advanced Spatial Upscaling"), qsTr("Custom")]
                        currentIndex: dialog.backend.settings.videoPreset
                        onActivated: (index) => {
                            dialog.backend.settings.videoPreset = index;
                            switch (index) {
                            case 0: dialog.backend.window.videoPreset = ChiakiWindow.VideoPreset.Fast; break;
                            case 1: dialog.backend.window.videoPreset = ChiakiWindow.VideoPreset.Default; break;
                            case 2: dialog.backend.window.videoPreset = ChiakiWindow.VideoPreset.HighQuality; break;
                            case 3: dialog.backend.window.videoPreset = ChiakiWindow.VideoPreset.HighQualitySpatial; break;
                            case 4: dialog.backend.window.videoPreset = ChiakiWindow.VideoPreset.HighQualityAdvancedSpatial; break;
                            case 5: dialog.backend.window.videoPreset = ChiakiWindow.VideoPreset.Custom; break;
                            }
                        }
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("(High Quality)")
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("Placebo Queue Depth Limit")
                    }

                    C.Slider {
                        Layout.preferredWidth: 360
                        from: 2
                        to: 5
                        stepSize: 1
                        value: dialog.backend.settings.queueDepthLimit
                        onMoved: dialog.backend.settings.queueDepthLimit = value

                        Label {
                            anchors {
                                left: parent.right
                                verticalCenter: parent.verticalCenter
                                leftMargin: 10
                            }
                            text: qsTr("%1 frames").arg(parent.value)
                        }
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("(2)")
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("Renderer Backend:")
                    }

                    C.ComboBox {
                        Layout.preferredWidth: 400
                        model: [qsTr("Vulkan"), qsTr("OpenGL")]
                        currentIndex: dialog.backend.settings.rendererBackend
                        onActivated: (index) => {
                            if (index === dialog.backend.settings.rendererBackend)
                                return;
                            dialog.backend.settings.rendererBackend = index;
                            dialog.backend.settings.restartApplication();
                        }
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("(Vulkan)")
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("Custom Renderer Settings")
                        visible: dialog.backend.window.videoPreset == ChiakiWindow.VideoPreset.Custom
                    }

                    C.Button {
                        id: customRendererSettings
                        text: qsTr("Open")
                        onClicked: dialog.navigation.showPlaceboSettingsDialog()
                        Material.roundedScale: Material.SmallScale
                        visible: dialog.backend.window.videoPreset == ChiakiWindow.VideoPreset.Custom
                    }

                    Label { visible: dialog.backend.window.videoPreset == ChiakiWindow.VideoPreset.Custom }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("Display Settings")
                    }

                    C.Button {
                        id: displaySettings
                        text: qsTr("Open")
                        onClicked: dialog.navigation.showDisplaySettingsDialog()
                        Material.roundedScale: Material.SmallScale
                        lastInFocusChain: true
                    }

                        Label {}
                    }
                }
            }

            Item {
                // Stream
                Flickable {
                    id: streamFlick
                    anchors {
                        fill: parent
                        topMargin: 20
                        bottomMargin: 20
                    }
                    clip: true
                    contentWidth: Math.max(width, streamGrid.width)
                    contentHeight: streamGrid.height
                    flickableDirection: Flickable.AutoFlickIfNeeded
                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AlwaysOn
                        visible: streamFlick.contentHeight > streamFlick.height
                    }
                    GridLayout {
                        id: streamGrid
                        anchors {
                            top: parent.top
                            horizontalCenter: parent.horizontalCenter
                        }
                        columns: 3
                        rowSpacing: 10
                        columnSpacing: 20

                        Label {
                            Layout.alignment: Qt.AlignRight
                            text: qsTr("Settings for:")
                        }


                    C.ComboBox {
                        id: consoleSelection
                        Layout.preferredWidth: 400
                        Layout.alignment: Qt.AlignLeft
                        model: [qsTr("PS4"), qsTr("PS5")]
                        currentIndex: selectedConsole
                        onActivated: (index) => selectedConsole = index
                        firstInFocusChain: true
                        KeyNavigation.priority: {
                            if(!popup.visible)
                                KeyNavigation.BeforeItem
                            else
                                KeyNavigation.AfterItem
                        }
                        KeyNavigation.down: {
                            if(selectedConsole == SettingsDialog.Console.PS4)
                                resolutionLocalPS4
                            else
                                resolutionLocalPS5

                        }

                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("")
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("")
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("")
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("")
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("")
                    }

                    Label {
                        Layout.alignment: Qt.AlignCenter
                        text: qsTr("Local")
                    }

                    Label {
                        Layout.alignment: Qt.AlignCenter
                        text: qsTr("Remote")
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("Resolution:")
                    }

                    C.ComboBox {
                        id: resolutionLocalPS4
                        Layout.preferredWidth: 400
                        model: [qsTr("360p"), qsTr("540p"), qsTr("720p (Default)"), qsTr("1080p (PS5 and PS4 Pro)")]
                        currentIndex: dialog.backend.settings.resolutionLocalPS4 - 1
                        onActivated: (index) => {
                            dialog.backend.settings.resolutionLocalPS4 = index + 1
                            dialog.backend.settings.bitrateLocalPS4 = 0
                        }
                        visible: selectedConsole == SettingsDialog.Console.PS4
                        KeyNavigation.right: resolutionRemotePS4
                        KeyNavigation.down: fpsLocalPS4
                        KeyNavigation.up: consoleSelection
                        KeyNavigation.priority: {
                            if(!popup.visible)
                                KeyNavigation.BeforeItem
                            else
                                KeyNavigation.AfterItem
                        }
                    }

                    C.ComboBox {
                        id: resolutionRemotePS4
                        Layout.preferredWidth: 400
                        model: [qsTr("360p"), qsTr("540p"), qsTr("720p (Default)"), qsTr("1080p (PS5 and PS4 Pro)")]
                        currentIndex: dialog.backend.settings.resolutionRemotePS4 - 1
                        onActivated: (index) => {
                            dialog.backend.settings.resolutionRemotePS4 = index + 1
                            dialog.backend.settings.bitrateRemotePS4 = 0
                        }
                        visible: selectedConsole == SettingsDialog.Console.PS4
                        KeyNavigation.left: resolutionLocalPS4
                        KeyNavigation.down: fpsRemotePS4
                        KeyNavigation.up: consoleSelection
                        KeyNavigation.priority: {
                            if(!popup.visible)
                                KeyNavigation.BeforeItem
                            else
                                KeyNavigation.AfterItem
                        }
                    }

                    C.ComboBox {
                        id: resolutionLocalPS5
                        Layout.preferredWidth: 400
                        model: [qsTr("360p"), qsTr("540p"), qsTr("720p"), qsTr("1080p (Default)")]
                        currentIndex: dialog.backend.settings.resolutionLocalPS5 - 1
                        onActivated: (index) => {
                            dialog.backend.settings.resolutionLocalPS5 = index + 1
                            dialog.backend.settings.bitrateLocalPS5 = 0
                        }
                        visible: selectedConsole == SettingsDialog.Console.PS5
                        KeyNavigation.right: resolutionRemotePS5
                        KeyNavigation.up: consoleSelection
                        KeyNavigation.down: fpsLocalPS5
                        KeyNavigation.priority: {
                            if(!popup.visible)
                                KeyNavigation.BeforeItem
                            else
                                KeyNavigation.AfterItem
                        }
                    }

                    C.ComboBox {
                        id: resolutionRemotePS5
                        Layout.preferredWidth: 400
                        model: [qsTr("360p"), qsTr("540p"), qsTr("720p (Default)"), qsTr("1080p")]
                        currentIndex: dialog.backend.settings.resolutionRemotePS5 - 1
                        onActivated: (index) => {
                            dialog.backend.settings.resolutionRemotePS5 = index + 1
                            dialog.backend.settings.bitrateRemotePS5 = 0
                        }
                        visible: selectedConsole == SettingsDialog.Console.PS5
                        KeyNavigation.left: resolutionLocalPS5
                        KeyNavigation.up: consoleSelection
                        KeyNavigation.down: fpsRemotePS5
                        KeyNavigation.priority: {
                            if(!popup.visible)
                                KeyNavigation.BeforeItem
                            else
                                KeyNavigation.AfterItem
                        }
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("FPS:")
                    }

                    C.ComboBox {
                        id: fpsLocalPS4
                        Layout.preferredWidth: 400
                        model: [qsTr("30 fps"), qsTr("60 fps (Default)")]
                        currentIndex: (dialog.backend.settings.fpsLocalPS4 / 30) - 1
                        onActivated: (index) => dialog.backend.settings.fpsLocalPS4 = (index + 1) * 30
                        visible: selectedConsole == SettingsDialog.Console.PS4
                        KeyNavigation.up: resolutionLocalPS4
                        KeyNavigation.right: fpsRemotePS4
                        KeyNavigation.down: bitrateLocalPS4
                        KeyNavigation.priority: {
                            if(!popup.visible)
                                KeyNavigation.BeforeItem
                            else
                                KeyNavigation.AfterItem
                        }
                    }

                    C.ComboBox {
                        id: fpsRemotePS4
                        Layout.preferredWidth: 400
                        model: [qsTr("30 fps"), qsTr("60 fps (Default)")]
                        currentIndex: (dialog.backend.settings.fpsRemotePS4 / 30) - 1
                        onActivated: (index) => dialog.backend.settings.fpsRemotePS4 = (index + 1) * 30
                        visible: selectedConsole == SettingsDialog.Console.PS4
                        KeyNavigation.up: resolutionRemotePS4
                        KeyNavigation.left: fpsLocalPS4
                        KeyNavigation.down: bitrateRemotePS4
                        KeyNavigation.priority: {
                            if(!popup.visible)
                                KeyNavigation.BeforeItem
                            else
                                KeyNavigation.AfterItem
                        }
                    }

                    C.ComboBox {
                        id: fpsLocalPS5
                        Layout.preferredWidth: 400
                        model: [qsTr("30 fps"), qsTr("60 fps (Default)")]
                        currentIndex: (dialog.backend.settings.fpsLocalPS5 / 30) - 1
                        onActivated: (index) => dialog.backend.settings.fpsLocalPS5 = (index + 1) * 30
                        visible: selectedConsole == SettingsDialog.Console.PS5
                        KeyNavigation.up: resolutionLocalPS5
                        KeyNavigation.right: fpsRemotePS5
                        KeyNavigation.down: bitrateLocalPS5
                        KeyNavigation.priority: {
                            if(!popup.visible)
                                KeyNavigation.BeforeItem
                            else
                                KeyNavigation.AfterItem
                        }
                    }

                    C.ComboBox {
                        id: fpsRemotePS5
                        Layout.preferredWidth: 400
                        model: [qsTr("30 fps"), qsTr("60 fps (Default)")]
                        currentIndex: (dialog.backend.settings.fpsRemotePS5 / 30) - 1
                        onActivated: (index) => dialog.backend.settings.fpsRemotePS5 = (index + 1) * 30
                        visible: selectedConsole == SettingsDialog.Console.PS5
                        KeyNavigation.up: resolutionRemotePS5
                        KeyNavigation.left: fpsLocalPS5
                        KeyNavigation.down: bitrateRemotePS5
                        KeyNavigation.priority: {
                            if(!popup.visible)
                                KeyNavigation.BeforeItem
                            else
                                KeyNavigation.AfterItem
                        }
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("Bitrate:")
                    }

                    C.Slider {
                        id: bitrateLocalPS4
                        visible: selectedConsole == SettingsDialog.Console.PS4
                        property var bitrate: {
                            var rate = 0;
                            switch (dialog.backend.settings.resolutionLocalPS4) {
                            case 1: rate = 2; break; // 360p
                            case 2: rate = 6; break; // 540p
                            case 3: rate = 10; break; // 720p
                            case 4: rate = 15; break; // 1080p
                            }
                            return rate;
                        }
                        Layout.preferredWidth: 200
                        from: 2
                        to: 100
                        stepSize: 1
                        value: dialog.backend.settings.bitrateLocalPS4 / 1000 ? (dialog.backend.settings.bitrateLocalPS4 / 1000) : bitrate
                        onMoved: dialog.backend.settings.bitrateLocalPS4 = value * 1000;
                        KeyNavigation.up: fpsLocalPS4
                        KeyNavigation.down: bitrateLocalPS4
                        KeyNavigation.priority: KeyNavigation.BeforeItem
                        Label {
                            anchors {
                                left: parent.right
                                verticalCenter: parent.verticalCenter
                                leftMargin: 10
                            }
                            text: (parent.value) + qsTr(" Mbps") + qsTr(" (%1 Mbps)").arg(parent.bitrate.toFixed(0))
                        }
                    }

                    C.Slider {
                        id: bitrateRemotePS4
                        visible: selectedConsole == SettingsDialog.Console.PS4
                        property var bitrate: {
                            var rate = 0;
                            switch (dialog.backend.settings.resolutionRemotePS4) {
                            case 1: rate = 2; break; // 360p
                            case 2: rate = 6; break; // 540p
                            case 3: rate = 10; break; // 720p
                            case 4: rate = 15; break; // 1080p
                            }
                            return rate;
                        }
                        Layout.preferredWidth: 200
                        from: 2
                        to: 100
                        stepSize: 1
                        value: dialog.backend.settings.bitrateRemotePS4 / 1000 ? (dialog.backend.settings.bitrateRemotePS4 / 1000) : bitrate
                        onMoved: dialog.backend.settings.bitrateRemotePS4 = value * 1000;
                        KeyNavigation.up: fpsRemotePS4
                        KeyNavigation.down: bitrateRemotePS4
                        KeyNavigation.priority: KeyNavigation.BeforeItem
                        lastInFocusChain: true

                        Label {
                            anchors {
                                left: parent.right
                                verticalCenter: parent.verticalCenter
                                leftMargin: 10
                            }
                            text: (parent.value) + qsTr(" Mbps") + qsTr(" (%1 Mbps)").arg(parent.bitrate.toFixed(0))
                        }
                    }

                    C.Slider {
                        id: bitrateLocalPS5
                        visible: selectedConsole == SettingsDialog.Console.PS5
                        property var bitrate: {
                            var rate = 0;
                            switch (dialog.backend.settings.resolutionLocalPS5) {
                            case 1: rate = 2; break; // 360p
                            case 2: rate = 6; break; // 540p
                            case 3: rate = 10; break; // 720p
                            case 4: rate = 15; break; // 1080p
                            }
                            return rate;
                        }
                        Layout.preferredWidth: 200
                        from: 2
                        to: 100
                        stepSize: 1
                        value: dialog.backend.settings.bitrateLocalPS5 / 1000 ? (dialog.backend.settings.bitrateLocalPS5 / 1000) : bitrate
                        onMoved: dialog.backend.settings.bitrateLocalPS5 = value * 1000;
                        KeyNavigation.up: fpsLocalPS5
                        KeyNavigation.down: codecLocalPS5
                        KeyNavigation.priority: KeyNavigation.BeforeItem

                        Label {
                            anchors {
                                left: parent.right
                                verticalCenter: parent.verticalCenter
                                leftMargin: 10
                            }
                            text: (parent.value) + qsTr(" Mbps") + qsTr(" (%1 Mbps)").arg(parent.bitrate.toFixed(0))
                        }
                    }

                    C.Slider {
                        id: bitrateRemotePS5
                        visible: selectedConsole == SettingsDialog.Console.PS5
                        property var bitrate: {
                            var rate = 0;
                            switch (dialog.backend.settings.resolutionRemotePS5) {
                            case 1: rate = 2; break; // 360p
                            case 2: rate = 6; break; // 540p
                            case 3: rate = 10; break; // 720p
                            case 4: rate = 15; break; // 1080p
                            }
                            return rate;
                        }
                        Layout.preferredWidth: 200
                        from: 2
                        to: 100
                        stepSize: 1
                        value: dialog.backend.settings.bitrateRemotePS5 / 1000 ? (dialog.backend.settings.bitrateRemotePS5 / 1000) : bitrate
                        onMoved: dialog.backend.settings.bitrateRemotePS5 = value * 1000;
                        KeyNavigation.up: fpsRemotePS5
                        KeyNavigation.down: codecRemotePS5
                        KeyNavigation.priority: KeyNavigation.BeforeItem
                        lastInFocusChain: true

                        Label {
                            anchors {
                                left: parent.right
                                verticalCenter: parent.verticalCenter
                                leftMargin: 10
                            }
                            text: (parent.value) + qsTr(" Mbps") + qsTr(" (%1 Mbps)").arg(parent.bitrate.toFixed(0))
                        }
                    }

                    Label {
                        Layout.alignment: Qt.AlignRight
                        text: qsTr("Codec:")
                        visible: selectedConsole == SettingsDialog.Console.PS5
                    }

                    C.ComboBox {
                        id: codecLocalPS5
                        Layout.preferredWidth: 400
                        model: [qsTr("H264"), qsTr("H265 (Default)"), qsTr("H265 HDR")]
                        currentIndex: dialog.backend.settings.codecLocalPS5
                        onActivated: (index) => dialog.backend.settings.codecLocalPS5 = index
                        visible: selectedConsole == SettingsDialog.Console.PS5
                        Keys.onReturnPressed: {
                            if (popup.visible) {
                                activated(highlightedIndex);
                                popup.close();
                            } else
                                popup.open();
                        }
                        KeyNavigation.up: bitrateLocalPS5
                        KeyNavigation.right: codecRemotePS5
                        KeyNavigation.down: codecLocalPS5
                        KeyNavigation.priority: {
                            if(!popup.visible)
                                KeyNavigation.BeforeItem
                            else
                                KeyNavigation.AfterItem
                        }
                    }

                        C.ComboBox {
                            id: codecRemotePS5
                            Layout.preferredWidth: 400
                            model: [qsTr("H264"), qsTr("H265 (Default)"), qsTr("H265 HDR")]
                            currentIndex: dialog.backend.settings.codecRemotePS5
                            onActivated: (index) => dialog.backend.settings.codecRemotePS5 = index
                            visible: selectedConsole == SettingsDialog.Console.PS5
                            lastInFocusChain: true
                            Keys.onReturnPressed: {
                                if (popup.visible) {
                                    activated(highlightedIndex);
                                    popup.close();
                                } else
                                    popup.open();
                            }
                            KeyNavigation.up: bitrateRemotePS5
                            KeyNavigation.left: codecLocalPS5
                            KeyNavigation.priority: {
                                if(!popup.visible)
                                    KeyNavigation.BeforeItem
                                else
                                    KeyNavigation.AfterItem
                            }
                        }
                    }
                }
            }

            Item {
                // Audio and Wifi
                Flickable {
                    id: audiowifiFlick
                    objectName: "audiowifiFlick"
                    anchors.fill: parent
                    clip: true
                    contentWidth: width
                    contentHeight: audiowifiLayout.height
                    flickableDirection: Flickable.VerticalFlick
                    ScrollBar.vertical: ScrollBar {}
                    ColumnLayout {
                        id: audiowifiLayout
                        width: Math.max(0, audiowifiFlick.width - 14)
                        spacing: 16
                        onVisibleChanged: if (visible) dialog.backend.settings.refreshAudioDevices()
                        Label { text: qsTr("Audio/Wifi"); font.pixelSize: 24; font.bold: true }

                        Label { font.pixelSize: 14; text: qsTr("Audio"); font.bold: true }
                        Frame {
                            Layout.fillWidth: true
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            GridLayout {
                                anchors.fill: parent
                                columns: audiowifiFlick.width < 650 ? 1 : 2
                                columnSpacing: 24
                                rowSpacing: 12
                                Label {
                                    Layout.fillWidth: true
                                    font.pixelSize: 14
                                    wrapMode: Text.Wrap
                                    text: qsTr("Output device\nDefault: Auto")
                                }
                                C.ComboBox {
                                    id: audioOutDevice
                                    objectName: "audioOutDevice"
                                    Accessible.name: qsTr("Output device")
                                    font.pixelSize: 14
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredWidth: 300
                                    popup.x: (width - popup.width) / 2
                                    popup.width: Math.min(700, dialog.width - 24)
                                    popup.font.pixelSize: 14
                                    firstInFocusChain: true
                                    model: [qsTr("Auto")].concat(dialog.backend.settings.availableAudioOutDevices)
                                    currentIndex: Math.max(0, model.indexOf(dialog.backend.settings.audioOutDevice))
                                    onModelChanged: if (popup.visible) popup.close()
                                    onActivated: index => {
                                        if (index >= 0 && index < model.length)
                                            dialog.backend.settings.audioOutDevice = index ? model[index] : "";
                                    }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    font.pixelSize: 14
                                    wrapMode: Text.Wrap
                                    text: qsTr("Input device\nDefault: Auto")
                                }
                                C.ComboBox {
                                    id: audioInDevice
                                    objectName: "audioInDevice"
                                    Accessible.name: qsTr("Input device")
                                    font.pixelSize: 14
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    Layout.preferredWidth: 300
                                    popup.x: (width - popup.width) / 2
                                    popup.width: Math.min(700, dialog.width - 24)
                                    popup.font.pixelSize: 14
                                    firstInFocusChain: false
                                    model: [qsTr("Auto")].concat(dialog.backend.settings.availableAudioInDevices)
                                    currentIndex: Math.max(0, model.indexOf(dialog.backend.settings.audioInDevice))
                                    onModelChanged: if (popup.visible) popup.close()
                                    onActivated: index => {
                                        if (index >= 0 && index < model.length)
                                            dialog.backend.settings.audioInDevice = index ? model[index] : "";
                                    }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    font.pixelSize: 14
                                    wrapMode: Text.Wrap
                                    text: qsTr("Audio buffer size\nDefault: 50 ms")
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    C.Slider {
                                        id: audioBufferSizeSlider
                                        objectName: "audioBufferSize"
                                        Accessible.name: qsTr("Audio buffer size")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        Layout.preferredWidth: 300
                                        from: 1
                                        to: 10
                                        stepSize: 1
                                        value: dialog.backend.settings.audioBufferSize / 1920 || 5
                                        onMoved: dialog.backend.settings.audioBufferSize = value * 1920
                                    }
                                    Label {
                                        objectName: "audioBufferSizeValue"
                                        Layout.fillWidth: true
                                        font.pixelSize: 14
                                        wrapMode: Text.Wrap
                                        text: qsTr("%1 ms").arg((audioBufferSizeSlider.value * 10).toFixed(0))
                                    }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    font.pixelSize: 14
                                    wrapMode: Text.Wrap
                                    text: qsTr("Audio volume\nDefault: 100%")
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    C.Slider {
                                        id: audioVolumeSlider
                                        objectName: "audioVolume"
                                        Accessible.name: qsTr("Audio volume")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        Layout.preferredWidth: 300
                                        from: 0
                                        to: 128
                                        stepSize: 1
                                        value: dialog.backend.settings.audioVolume
                                        onMoved: dialog.backend.settings.audioVolume = value
                                    }
                                    Label {
                                        objectName: "audioVolumeValue"
                                        Layout.fillWidth: true
                                        font.pixelSize: 14
                                        wrapMode: Text.Wrap
                                        text: qsTr("%1% volume").arg((audioVolumeSlider.value / 128 * 100).toFixed(0))
                                    }
                                }
                            }
                        }
                        Label { font.pixelSize: 14; text: qsTr("Microphone"); font.bold: true }
                        Frame {
                            Layout.fillWidth: true
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            GridLayout {
                                anchors.fill: parent
                                columns: audiowifiFlick.width < 650 ? 1 : 2
                                columnSpacing: 24
                                rowSpacing: 12
                                Label {
                                    Layout.fillWidth: true
                                    font.pixelSize: 14
                                    wrapMode: Text.Wrap
                                    text: qsTr("Start mic unmuted\nDefault: Unchecked")
                                }
                                C.CheckBox {
                                    objectName: "startMicUnmuted"
                                    Accessible.name: qsTr("Start mic unmuted")
                                    font.pixelSize: 14
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    text: qsTr("Start mic unmuted")
                                    contentItem: Label { font.pixelSize: 14; text: parent.text; wrapMode: Text.Wrap; leftPadding: parent.indicator.width + parent.spacing; verticalAlignment: Text.AlignVCenter }
                                    checked: dialog.backend.settings.startMicUnmuted
                                    onToggled: dialog.backend.settings.startMicUnmuted = checked
                                }
                                Label {
                                    Layout.fillWidth: true
                                    font.pixelSize: 14
                                    wrapMode: Text.Wrap
                                    text: qsTr("Speech processing\nDefault: Unchecked")
                                    visible: typeof dialog.backend.settings.speechProcessing !== "undefined"
                                }
                                C.CheckBox {
                                    objectName: "speechProcessing"
                                    Accessible.name: qsTr("Speech processing")
                                    font.pixelSize: 14
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    text: qsTr("Noise suppression + echo cancellation")
                                    contentItem: Label { font.pixelSize: 14; text: parent.text; wrapMode: Text.Wrap; leftPadding: parent.indicator.width + parent.spacing; verticalAlignment: Text.AlignVCenter }
                                    checked: dialog.backend.settings.speechProcessing || false
                                    onToggled: dialog.backend.settings.speechProcessing = checked
                                    visible: typeof dialog.backend.settings.speechProcessing !== "undefined"
                                }
                                Label {
                                    Layout.fillWidth: true
                                    font.pixelSize: 14
                                    wrapMode: Text.Wrap
                                    text: qsTr("Noise to suppress\nDefault: 6 dB")
                                    visible: dialog.backend.settings.speechProcessing || false
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.speechProcessing || false
                                    C.Slider {
                                        id: noiseSuppressLevelSlider
                                        objectName: "noiseSuppressLevel"
                                        Accessible.name: qsTr("Noise to suppress")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        Layout.preferredWidth: 300
                                        from: 0
                                        to: 60
                                        stepSize: 1
                                        value: typeof dialog.backend.settings.noiseSuppressLevel !== "undefined" ? dialog.backend.settings.noiseSuppressLevel : 6
                                        onMoved: dialog.backend.settings.noiseSuppressLevel = value
                                    }
                                    Label {
                                        objectName: "noiseSuppressLevelValue"
                                        Layout.fillWidth: true
                                        font.pixelSize: 14
                                        wrapMode: Text.Wrap
                                        text: qsTr("%1 dB").arg(noiseSuppressLevelSlider.value)
                                    }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    font.pixelSize: 14
                                    wrapMode: Text.Wrap
                                    text: qsTr("Echo to suppress\nDefault: 30 dB")
                                    visible: dialog.backend.settings.speechProcessing || false
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    visible: dialog.backend.settings.speechProcessing || false
                                    C.Slider {
                                        id: echoSuppressLevelSlider
                                        objectName: "echoSuppressLevel"
                                        Accessible.name: qsTr("Echo to suppress")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        Layout.preferredWidth: 300
                                        from: 0
                                        to: 60
                                        stepSize: 1
                                        value: typeof dialog.backend.settings.echoSuppressLevel !== "undefined" ? dialog.backend.settings.echoSuppressLevel : 30
                                        onMoved: dialog.backend.settings.echoSuppressLevel = value
                                    }
                                    Label {
                                        objectName: "echoSuppressLevelValue"
                                        Layout.fillWidth: true
                                        font.pixelSize: 14
                                        wrapMode: Text.Wrap
                                        text: qsTr("%1 dB").arg(echoSuppressLevelSlider.value)
                                    }
                                }
                            }
                        }
                        Label { font.pixelSize: 14; text: qsTr("Connection feedback"); font.bold: true }
                        Frame {
                            Layout.fillWidth: true
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            GridLayout {
                                anchors.fill: parent
                                columns: audiowifiFlick.width < 650 ? 1 : 2
                                columnSpacing: 24
                                rowSpacing: 12
                                Label {
                                    Layout.fillWidth: true
                                    font.pixelSize: 14
                                    wrapMode: Text.Wrap
                                    text: qsTr("Weak Wifi notification\nDefault: 3%")
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    C.Slider {
                                        id: wifiDroppedNotifSlider
                                        objectName: "wifiDroppedNotif"
                                        Accessible.name: qsTr("Weak Wifi notification")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        Layout.preferredWidth: 300
                                        from: 0
                                        to: 100
                                        stepSize: 1
                                        value: dialog.backend.settings.wifiDroppedNotif
                                        onMoved: dialog.backend.settings.wifiDroppedNotif = value
                                    }
                                    Label {
                                        objectName: "wifiDroppedNotifValue"
                                        Layout.fillWidth: true
                                        font.pixelSize: 14
                                        wrapMode: Text.Wrap
                                        text: qsTr(">= %1% dropped packets").arg(wifiDroppedNotifSlider.value)
                                    }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    font.pixelSize: 14
                                    wrapMode: Text.Wrap
                                    text: qsTr("Packet loss reported max\nDefault: 5%")
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    C.Slider {
                                        id: packetLossReportedMaxSlider
                                        objectName: "packetLossReportedMax"
                                        Accessible.name: qsTr("Packet loss reported max")
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        Layout.preferredWidth: 300
                                        from: 0
                                        to: 100
                                        stepSize: 1
                                        value: dialog.backend.settings.packetLossReportedMax
                                        onMoved: dialog.backend.settings.packetLossReportedMax = value
                                    }
                                    Label {
                                        objectName: "packetLossReportedMaxValue"
                                        Layout.fillWidth: true
                                        font.pixelSize: 14
                                        wrapMode: Text.Wrap
                                        text: qsTr("%1% packet loss").arg(packetLossReportedMaxSlider.value)
                                    }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    font.pixelSize: 14
                                    wrapMode: Text.Wrap
                                    text: qsTr("Request IDR frame on FEC failure\nDefault: Unchecked")
                                }
                                C.CheckBox {
                                    objectName: "iDROnFECFailureEnabled"
                                    Accessible.name: qsTr("Request IDR frame on FEC failure")
                                    font.pixelSize: 14
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    text: qsTr("Request IDR frame on FEC failure")
                                    contentItem: Label { font.pixelSize: 14; text: parent.text; wrapMode: Text.Wrap; leftPadding: parent.indicator.width + parent.spacing; verticalAlignment: Text.AlignVCenter }
                                    checked: dialog.backend.settings.iDROnFECFailureEnabled
                                    onToggled: dialog.backend.settings.iDROnFECFailureEnabled = checked
                                }
                                Label {
                                    Layout.fillWidth: true
                                    font.pixelSize: 14
                                    wrapMode: Text.Wrap
                                    text: qsTr("Show stream stats during gameplay\nDefault: Unchecked")
                                }
                                C.CheckBox {
                                    objectName: "showStreamStats"
                                    Accessible.name: qsTr("Show stream stats during gameplay")
                                    font.pixelSize: 14
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    text: qsTr("Show stream stats during gameplay")
                                    contentItem: Label { font.pixelSize: 14; text: parent.text; wrapMode: Text.Wrap; leftPadding: parent.indicator.width + parent.spacing; verticalAlignment: Text.AlignVCenter }
                                    checked: dialog.backend.settings.showStreamStats
                                    onToggled: dialog.backend.settings.showStreamStats = checked
                                    lastInFocusChain: true
                                }
                            }
                        }
                    }
                }
            }

            Item {
                // Consoles
                Flickable {
                    id: consolesFlick
                    anchors {
                        fill: parent
                        topMargin: 20
                        bottomMargin: 20
                    }
                    clip: true
                    contentWidth: Math.max(width, consolesLayout.width)
                    contentHeight: consolesLayout.height
                    flickableDirection: Flickable.AutoFlickIfNeeded
                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AlwaysOn
                        visible: consolesFlick.contentHeight > consolesFlick.height
                    }
                    ColumnLayout {
                        id: consolesLayout
                        anchors {
                            top: parent.top
                            horizontalCenter: parent.horizontalCenter
                        }
                        spacing: 10

                        Label {
                            text: ""
                        }

                        Label {
                            text: ""
                        }

                        C.Button {
                            id: registerNewButton
                            Layout.alignment: Qt.AlignHCenter
                            Layout.topMargin: 10
                            topPadding: 26
                            leftPadding: 30
                            rightPadding: 30
                            bottomPadding: 26
                            firstInFocusChain: true
                            text: qsTr("Register New")
                            onClicked: dialog.navigation.showRegistDialog("255.255.255.255", true)
                            Material.roundedScale: Material.SmallScale
                        }

                        Label {
                            id: consolesLabel
                            Layout.alignment: Qt.AlignHCenter
                            text: qsTr("Registered Consoles")
                            font.bold: true
                        }

                        ListView {
                            id: consolesView
                            Layout.alignment: Qt.AlignHCenter
                            height: 170
                            onCountChanged: {
                                consolesView.contentHeight = consolesView.count * 80 + consolesView.anchors.topMargin;
                            }
                            keyNavigationEnabled: false
                            width: 700
                            ScrollBar.vertical: ScrollBar {
                                id: consolesScrollbar
                                policy: ScrollBar.AlwaysOn
                                visible: consolesView.contentHeight > consolesView.height
                            }
                            clip: true
                            model: dialog.backend.settings.registeredHosts
                            delegate: ItemDelegate {
                                text: "%1 (%2, %3)".arg(dialog.backend.settings.streamerMode ? "hidden" : modelData.mac).arg(modelData.ps5 ? "PS5" : "PS4").arg(modelData.name)
                                height: 80
                                width: parent ? parent.width : 0
                                leftPadding: autoConnectButton.width + 40

                                CheckBox {
                                    property bool firstInFocusChain: false
                                    property bool lastInFocusChain: false
                                    property bool lastDownInFocusChain: index > consolesView.count + hiddenConsolesView.count - 2

                                    id: autoConnectButton
                                    anchors {
                                        left: parent.left
                                        verticalCenter: parent.verticalCenter
                                        leftMargin: 20
                                    }
                                    text: qsTr("Auto-Connect")
                                    checked: dialog.backend.settings.autoConnectMac == modelData.mac
                                    onToggled: dialog.backend.settings.autoConnectMac = checked ? modelData.mac : "";

                                    Keys.onPressed: (event) => {
                                        switch (event.key) {
                                        case Qt.Key_Right:
                                            if (!lastInFocusChain) {
                                                let item = nextItemInFocusChain();
                                                if (item)
                                                    item.forceActiveFocus(Qt.TabFocusReason);
                                                event.accepted = true;
                                            }
                                            break;
                                        case Qt.Key_Up:
                                            if (!firstInFocusChain) {
                                                let item = nextItemInFocusChain(false);
                                                if (item) {
                                                    item.forceActiveFocus(Qt.TabFocusReason);
                                                    dialog.ensureItemVisibleInFlick(consolesView, item);
                                                }
                                                let count = index > 0 ? 2: 0;
                                                for(var i = 0; i < count; i++)
                                                {
                                                    let item2 = item.nextItemInFocusChain(false);
                                                    if (item)
                                                    {
                                                        item.forceActiveFocus(Qt.TabFocusReason);
                                                        dialog.ensureItemVisibleInFlick(consolesView, item);
                                                        item = item2;
                                                    }
                                                }
                                                event.accepted = true;
                                            }
                                            break;
                                        case Qt.Key_Down:
                                            if (!lastDownInFocusChain) {
                                                let item = nextItemInFocusChain();
                                                if (item) {
                                                    item.forceActiveFocus(Qt.TabFocusReason);
                                                    dialog.ensureItemVisibleInFlick(consolesView, item);
                                                }
                                                let count = 2;
                                                for(var i = 0; i < count; i++)
                                                {
                                                    let item2 = item.nextItemInFocusChain();
                                                    if (item)
                                                    {
                                                        item.forceActiveFocus(Qt.TabFocusReason);
                                                        dialog.ensureItemVisibleInFlick(consolesView, item);
                                                        item = item2;
                                                    }
                                                }
                                                event.accepted = true;
                                            }
                                            break;
                                        case Qt.Key_Return:
                                            if (visualFocus) {
                                                toggle();
                                                toggled();
                                            }
                                            event.accepted = true;
                                            break;
                                        }
                                    }
                                }

                                Button {
                                    property bool firstInFocusChain: false
                                    property bool lastInFocusChain: index > consolesView.count + hiddenConsolesView.count - 2
                                    Material.background: visualFocus ? Material.accent : undefined

                                    Component.onDestruction: {
                                        if (visualFocus) {
                                            let item = nextItemInFocusChain();
                                            if (item)
                                                item.forceActiveFocus(Qt.TabFocusReason);
                                        }
                                    }
                                    Keys.onPressed: (event) => {
                                        switch (event.key) {
                                            case Qt.Key_Left:
                                                if (!firstInFocusChain) {
                                                    let item = nextItemInFocusChain(false);
                                                    if (item)
                                                        item.forceActiveFocus(Qt.TabFocusReason);
                                                    event.accepted = true;
                                                }
                                                break;
                                            case Qt.Key_Up:
                                                if (!firstInFocusChain)
                                                {
                                                    let item = nextItemInFocusChain(false);
                                                    if (item) {
                                                        item.forceActiveFocus(Qt.TabFocusReason);
                                                        dialog.ensureItemVisibleInFlick(consolesView, item);
                                                    }
                                                    let count = 2;
                                                    for(var i = 0; i < count; i++)
                                                    {
                                                        let item2 = item.nextItemInFocusChain(false);
                                                        if (item)
                                                        {
                                                            item.forceActiveFocus(Qt.TabFocusReason);
                                                            dialog.ensureItemVisibleInFlick(consolesView, item);
                                                            item = item2;
                                                        }
                                                    }
                                                    event.accepted = true;
                                                }
                                                break;
                                            case Qt.Key_Down:
                                                if (!lastInFocusChain) {
                                                    let item = nextItemInFocusChain();
                                                    if (item) {
                                                        item.forceActiveFocus(Qt.TabFocusReason);
                                                        dialog.ensureItemVisibleInFlick(consolesView, item);
                                                    }
                                                    let count = index < consolesView.count - 1 ? 2: 0;
                                                    for(var i = 0; i < count; i++)
                                                    {
                                                        let item2 = item.nextItemInFocusChain();
                                                        if (item)
                                                        {
                                                            item.forceActiveFocus(Qt.TabFocusReason);
                                                            dialog.ensureItemVisibleInFlick(consolesView, item);
                                                            item = item2;
                                                        }
                                                    }
                                                    event.accepted = true;
                                                }
                                                break;
                                            case Qt.Key_Return:
                                                if (visualFocus) {
                                                    clicked();
                                                }
                                                event.accepted = true;
                                                break;
                                        }
                                    }
                                    anchors {
                                        right: parent.right
                                        verticalCenter: parent.verticalCenter
                                        rightMargin: 20
                                    }
                                    text: qsTr("Delete")
                                    onClicked: dialog.navigation.showConfirmDialog(qsTr("Delete Console"), qsTr("Are you sure you want to delete this console?"), () => dialog.backend.settings.deleteRegisteredHost(index));
                                    Material.roundedScale: Material.SmallScale
                                    Material.accent: Material.Red
                                }
                            }
                        }

                        Label {
                            id: hiddenConsolesLabel
                            Layout.alignment: Qt.AlignHCenter
                            text: qsTr("Hidden Consoles")
                            font.bold: true
                        }
                        ListView {
                            id: hiddenConsolesView
                            Layout.alignment: Qt.AlignHCenter
                            keyNavigationEnabled: false
                            height: 170
                            onCountChanged: {
                                hiddenConsolesView.contentHeight = hiddenConsolesView.count * 80 + hiddenConsolesView.anchors.topMargin;
                            }
                            clip: true
                            width: 500
                            ScrollBar.vertical: ScrollBar {
                                id: hiddenConsolesScrollbar
                                policy: ScrollBar.AlwaysOn
                                visible: hiddenConsolesView.contentHeight > hiddenConsolesView.height
                            }
                            model: dialog.backend.hiddenHosts
                            delegate: ItemDelegate {
                                text: "%1 (%2)".arg(dialog.backend.settings.streamerMode ? "hidden" : modelData.mac).arg(modelData.name)
                                height: 80
                                width: parent ? parent.width : 0

                                Button {
                                    property bool firstInFocusChain: false
                                    property bool lastInFocusChain: index > hiddenConsolesView.count - 2
                                    Material.background: visualFocus ? Material.accent : undefined

                                    Component.onDestruction: {
                                        if (visualFocus) {
                                            let item = nextItemInFocusChain();
                                            if (item)
                                                item.forceActiveFocus(Qt.TabFocusReason);
                                        }
                                    }
                                    Keys.onPressed: (event) => {
                                        switch (event.key) {
                                            case Qt.Key_Up:
                                                if (!firstInFocusChain)
                                                {
                                                    let item = nextItemInFocusChain(false);
                                                    if (item) {
                                                        item.forceActiveFocus(Qt.TabFocusReason);
                                                        dialog.ensureItemVisibleInFlick(hiddenConsolesView, item);
                                                    }
                                                    event.accepted = true;
                                                }
                                                break;
                                            case Qt.Key_Down:
                                                if (!lastInFocusChain) {
                                                    let item = nextItemInFocusChain();
                                                    if (item) {
                                                        item.forceActiveFocus(Qt.TabFocusReason);
                                                        dialog.ensureItemVisibleInFlick(hiddenConsolesView, item);
                                                    }
                                                    event.accepted = true;
                                                }
                                                break;
                                            case Qt.Key_Return:
                                                if (visualFocus) {
                                                    clicked();
                                                }
                                                event.accepted = true;
                                                break;
                                        }
                                    }
                                    anchors {
                                        right: parent.right
                                        verticalCenter: parent.verticalCenter
                                        rightMargin: 20
                                    }
                                    text: qsTr("Unhide")
                                    onClicked: dialog.navigation.showConfirmDialog(qsTr("Unhide Console"), qsTr("Are you sure you want to unhide this console?"), () => dialog.backend.unhideHost(modelData.mac));
                                    Material.roundedScale: Material.SmallScale
                                    Material.accent: Material.Red
                                }
                            }
                        }
                    }
                }
            }

            Item {
                // Keys
                id: controllerMapping
                Flickable {
                    id: keysFlick
                    anchors {
                        fill: parent
                        topMargin: 20
                        bottomMargin: 20
                    }
                    clip: true
                    contentWidth: Math.max(width, keysGrid.width)
                    contentHeight: keysGrid.height
                    flickableDirection: Flickable.AutoFlickIfNeeded
                    function ensureItemVisible(item) {
                        if (!item)
                            return;
                        const top = item.mapToItem(keysGrid, 0, 0).y;
                        const bottom = top + item.height;
                        if (top < contentY)
                            contentY = top;
                        else if (bottom > contentY + height)
                            contentY = bottom - height;
                    }
                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AlwaysOn
                        visible: keysFlick.contentHeight > keysFlick.height
                    }
                    GridLayout {
                        id: keysGrid
                        anchors {
                            top: parent.top
                            horizontalCenter: parent.horizontalCenter
                        }
                        columns: 3
                        rowSpacing: 3
                        columnSpacing: 10

                        Button {
                            id: resetAllKeys
                            text: "Reset All Keys"
                            Layout.alignment: Qt.AlignRight
                            property bool firstInFocusChain: true
                            property bool lastInFocusChain: false
                            onClicked: dialog.backend.settings.clearKeyMapping()
                            Material.roundedScale: Material.SmallScale
                            Material.background: visualFocus ? Material.accent : undefined
                            onActiveFocusChanged: if (activeFocus) keysFlick.ensureItemVisible(this)

                            Component.onDestruction: {
                                if (visualFocus) {
                                    let item = nextItemInFocusChain();
                                    if (item)
                                        item.forceActiveFocus(Qt.TabFocusReason);
                                }
                            }
                            Keys.onPressed: (event) => {
                                switch (event.key) {
                                case Qt.Key_Down:
                                    if (!lastInFocusChain) {
                                        let item = nextItemInFocusChain();
                                        if (item) {
                                            item.forceActiveFocus(Qt.TabFocusReason);
                                            keysFlick.ensureItemVisible(item);
                                        }
                                        for(var i = 0; i < 3; i++)
                                        {
                                            let item2 = item.nextItemInFocusChain();
                                            if (item)
                                            {
                                                item.forceActiveFocus(Qt.TabFocusReason);
                                                keysFlick.ensureItemVisible(item);
                                                item = item2;
                                            }
                                        }
                                        event.accepted = true;
                                    }
                                    break;
                                case Qt.Key_Right:
                                    if (!lastInFocusChain) {
                                        let item = nextItemInFocusChain();
                                        if (item) {
                                            item.forceActiveFocus(Qt.TabFocusReason);
                                            keysFlick.ensureItemVisible(item);
                                        }
                                        event.accepted = true;
                                    }
                                    break;
                                case Qt.Key_Return:
                                    if (visualFocus) {
                                        clicked();
                                    }
                                    event.accepted = true;
                                    break;
                                }
                            }
                        }

                        CheckBox {
                            text: qsTr("Keyboard as controller")
                            checked: {
                                dialog.backend.settings.keyboardEnabled
                            }
                            onToggled: dialog.backend.settings.keyboardEnabled = checked
                            Layout.alignment: Qt.AlignRight
                            property bool firstInFocusChain: false
                            property bool lastInFocusChain: false
                            Material.roundedScale: Material.SmallScale
                            Material.background: visualFocus ? Material.accent : undefined
                            onActiveFocusChanged: if (activeFocus) keysFlick.ensureItemVisible(this)

                            Component.onDestruction: {
                                if (visualFocus) {
                                    let item = nextItemInFocusChain();
                                    if (item)
                                        item.forceActiveFocus(Qt.TabFocusReason);
                                }
                            }
                            Keys.onPressed: (event) => {
                                switch (event.key) {
                                    case Qt.Key_Left:
                                        if (!firstInFocusChain) {
                                            let item = nextItemInFocusChain(false);
                                            if (item) {
                                                item.forceActiveFocus(Qt.TabFocusReason);
                                                keysFlick.ensureItemVisible(item);
                                            }
                                            event.accepted = true;
                                        }
                                        break;
                                    case Qt.Key_Right:
                                        if  (!lastInFocusChain) {
                                            let item = nextItemInFocusChain();
                                            if (item) {
                                                item.forceActiveFocus(Qt.TabFocusReason);
                                                keysFlick.ensureItemVisible(item);
                                            }
                                            event.accepted = true;
                                        }
                                        break;
                                    case Qt.Key_Down:
                                        if (!lastInFocusChain) {
                                            let item = nextItemInFocusChain();
                                            if (item) {
                                                item.forceActiveFocus(Qt.TabFocusReason);
                                                keysFlick.ensureItemVisible(item);
                                            }
                                            for(var i = 0; i < 3; i++)
                                            {
                                                let item2 = item.nextItemInFocusChain();
                                                if (item)
                                                {
                                                    item.forceActiveFocus(Qt.TabFocusReason);
                                                    keysFlick.ensureItemVisible(item);
                                                    item = item2;
                                                }
                                            }
                                            event.accepted = true;
                                        }
                                        break;
                                    case Qt.Key_Return:
                                        if (visualFocus) {
                                            toggle();
                                            toggled();
                                        }
                                        event.accepted = true;
                                        break;
                                }
                            }
                        }
                        CheckBox {
                            text: qsTr("Enable Mouse Touchpad")
                            checked: {
                                dialog.backend.settings.mouseTouchEnabled
                            }
                            onToggled: dialog.backend.settings.mouseTouchEnabled = checked
                            Layout.alignment: Qt.AlignRight
                            property bool firstInFocusChain: false
                            property bool lastInFocusChain: false
                            Material.roundedScale: Material.SmallScale
                            Material.background: visualFocus ? Material.accent : undefined
                            onActiveFocusChanged: if (activeFocus) keysFlick.ensureItemVisible(this)

                            Component.onDestruction: {
                                if (visualFocus) {
                                    let item = nextItemInFocusChain();
                                    if (item)
                                        item.forceActiveFocus(Qt.TabFocusReason);
                                }
                            }
                            Keys.onPressed: (event) => {
                                switch (event.key) {
                                    case Qt.Key_Left:
                                        if (!firstInFocusChain) {
                                            let item = nextItemInFocusChain(false);
                                            if (item) {
                                                item.forceActiveFocus(Qt.TabFocusReason);
                                                keysFlick.ensureItemVisible(item);
                                            }
                                            event.accepted = true;
                                        }
                                        break;
                                    case Qt.Key_Down:
                                        if (!lastInFocusChain) {
                                            let item = nextItemInFocusChain();
                                            if (item) {
                                                item.forceActiveFocus(Qt.TabFocusReason);
                                                keysFlick.ensureItemVisible(item);
                                            }
                                            for(var i = 0; i < 3; i++)
                                            {
                                                let item2 = item.nextItemInFocusChain();
                                                if (item)
                                                {
                                                    item.forceActiveFocus(Qt.TabFocusReason);
                                                    keysFlick.ensureItemVisible(item);
                                                    item = item2;
                                                }
                                            }
                                            event.accepted = true;
                                        }
                                        break;
                                    case Qt.Key_Return:
                                        if (visualFocus) {
                                            toggle();
                                            toggled();
                                        }
                                        event.accepted = true;
                                        break;
                                }
                            }
                        }
                        Repeater {
                            id: chiakiKeys
                            model: dialog.backend.settings.controllerMapping

                            RowLayout {
                                spacing: 20

                                Label {
                                    Layout.preferredWidth: 200
                                    horizontalAlignment: Text.AlignRight
                                    text: modelData.buttonName
                                }

                                Button {
                                    property bool firstInFocusChain: false
                                    property bool lastInFocusChain: index == (chiakiKeys.count - 1)
                                    Layout.preferredWidth: 170
                                    Layout.preferredHeight: 52
                                    text: modelData.keyName
                                    Material.roundedScale: Material.SmallScale
                                    Material.background: visualFocus ? Material.accent : undefined
                                    onActiveFocusChanged: if (activeFocus) keysFlick.ensureItemVisible(this)
                                    Component.onDestruction: {
                                        if (visualFocus) {
                                            let item = nextItemInFocusChain();
                                            if (item)
                                                item.forceActiveFocus(Qt.TabFocusReason);
                                        }
                                    }
                                    onClicked: {
                                        keyDialog.show({
                                            value: modelData.buttonValue,
                                            mappingIndex: index,
                                            callback: (name) => text = name,
                                        });
                                    }
                                    Keys.onPressed: (event) => {
                                        switch (event.key) {
                                            case Qt.Key_Left:
                                                if (!firstInFocusChain && ((index % 3) != 0)) {
                                                    let item = nextItemInFocusChain(false);
                                                    if (item) {
                                                        item.forceActiveFocus(Qt.TabFocusReason);
                                                        keysFlick.ensureItemVisible(item);
                                                    }
                                                    event.accepted = true;
                                                }
                                                break;
                                            case Qt.Key_Right:
                                                if  (!lastInFocusChain && (index % 3) != 2) {
                                                    let item = nextItemInFocusChain();
                                                    if (item) {
                                                        item.forceActiveFocus(Qt.TabFocusReason);
                                                        keysFlick.ensureItemVisible(item);
                                                    }
                                                    event.accepted = true;
                                                }
                                                break;
                                            case Qt.Key_Up:
                                                if (!firstInFocusChain)
                                                {
                                                    let item = nextItemInFocusChain(false);
                                                    if (item) {
                                                        item.forceActiveFocus(Qt.TabFocusReason);
                                                        keysFlick.ensureItemVisible(item);
                                                    }
                                                    for(var i = 0; i < 3; i++)
                                                    {
                                                        let item2 = item.nextItemInFocusChain(false);
                                                        if (item)
                                                        {
                                                            item.forceActiveFocus(Qt.TabFocusReason);
                                                            keysFlick.ensureItemVisible(item);
                                                            item = item2;
                                                        }
                                                    }
                                                    event.accepted = true;
                                                }
                                                break;
                                            case Qt.Key_Down:
                                                if (!lastInFocusChain && index < (chiakiKeys.count - 3)) {
                                                    let item = nextItemInFocusChain();
                                                    if (item) {
                                                        item.forceActiveFocus(Qt.TabFocusReason);
                                                        keysFlick.ensureItemVisible(item);
                                                    }
                                                    for(var i = 0; i < 3; i++)
                                                    {
                                                        let item2 = item.nextItemInFocusChain();
                                                        if (item)
                                                        {
                                                            item.forceActiveFocus(Qt.TabFocusReason);
                                                            keysFlick.ensureItemVisible(item);
                                                            item = item2;
                                                        }
                                                    }
                                                    event.accepted = true;
                                                }
                                                break;
                                            case Qt.Key_Return:
                                                if (visualFocus) {
                                                    clicked();
                                                }
                                                break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Item {
                // Controllers
                Flickable {
                    id: controllersFlick
                    anchors {
                        fill: parent
                        topMargin: 20
                        bottomMargin: 20
                    }
                    clip: true
                    contentWidth: Math.max(width, controllersLayout.width)
                    contentHeight: controllersLayout.height
                    flickableDirection: Flickable.AutoFlickIfNeeded
                    ScrollBar.vertical: ScrollBar {
                        id: controllersScrollbar
                        policy: ScrollBar.AlwaysOn
                        visible: controllersFlick.contentHeight > controllersFlick.height
                    }
                    ColumnLayout {
                        id: controllersLayout
                        anchors {
                            top: parent.top
                            horizontalCenter: parent.horizontalCenter
                        }
                        spacing: 10
                        C.Button {
                            Layout.alignment: Qt.AlignHCenter
                            id: controllerMappingChange
                            firstInFocusChain: true
                            text: "Change Controller Mapping"
                            onClicked: controllerMappingDialog.show({
                                reset: false
                            });
                        }
                        C.Button {
                            Layout.alignment: Qt.AlignHCenter
                            id: controllerMappingReset
                            text: "Reset Controller Mapping"
                            onClicked: controllerMappingDialog.show({
                                reset: true
                            });
                        }

                        RowLayout {
                            spacing: 10
                            Layout.alignment: Qt.AlignHCenter
                            Label {
                                Layout.alignment: Qt.AlignRight
                                text: qsTr("Background Controller Events:")
                            }
                            C.CheckBox {
                                id: backgroundController
                                text: qsTr("Process controller input when application is in background")
                                checked: {
                                    dialog.backend.settings.allowJoystickBackgroundEvents
                                }
                                onToggled: dialog.backend.settings.allowJoystickBackgroundEvents = checked
                            }

                            Label {
                                Layout.alignment: Qt.AlignRight
                                text: qsTr("(Checked)")
                            }
                        }
                        RowLayout {
                            spacing: 10
                            Layout.alignment: Qt.AlignHCenter
                            Label {
                                Layout.alignment: Qt.AlignRight
                                text: qsTr("Dpad Touchpad Emulation")
                            }
                            C.CheckBox {
                                id: dpadTouch
                                checked: dialog.backend.settings.dpadTouchEnabled
                                onToggled: dialog.backend.settings.dpadTouchEnabled = !dialog.backend.settings.dpadTouchEnabled
                                KeyNavigation.priority: KeyNavigation.BeforeItem
                                KeyNavigation.up: backgroundController
                                KeyNavigation.left: dpadTouch
                                KeyNavigation.right: dpadTouch
                                KeyNavigation.down: {
                                    if(touchIncrement.visible)
                                        touchIncrement
                                    else
                                        posButtons
                                }
                            }

                            Label {
                                Layout.alignment: Qt.AlignRight
                                text: qsTr("(Checked)")
                            }
                        }

                        RowLayout {
                            id: touchIncrementLayout
                            spacing: 10
                            Layout.alignment: Qt.AlignHCenter
                            visible: dialog.backend.settings.dpadTouchEnabled
                            Label {
                                Layout.alignment: Qt.AlignRight
                                text: qsTr("Dpad Touch Increment:")
                            }

                            C.Slider {
                                id: touchIncrement
                                Layout.preferredWidth: 250
                                from: 1
                                to: 1079
                                stepSize: 1
                                value: dialog.backend.settings.dpadTouchIncrement
                                onMoved: dialog.backend.settings.dpadTouchIncrement = value

                                Label {
                                    anchors {
                                        left: parent.right
                                        verticalCenter: parent.verticalCenter
                                        leftMargin: 10
                                    }
                                    text: qsTr("%1 mm").arg(parent.value / 100)
                                }
                                KeyNavigation.priority: KeyNavigation.BeforeItem
                                KeyNavigation.up: dpadTouch
                                KeyNavigation.down: {
                                    if(dpadShortcut1.visible)
                                        dpadShortcut1
                                    else
                                        posButtons;
                                }
                            }

                            Label {
                                Layout.alignment: Qt.AlignRight
                                Layout.leftMargin: 100
                                text: qsTr("(0.3 mm)")
                            }
                        }
                        RowLayout {
                            spacing: 10
                            visible: dialog.backend.settings.dpadTouchEnabled
                            Layout.alignment: Qt.AlignHCenter
                            Label {
                                Layout.alignment: Qt.AlignRight
                                text: qsTr("Dpad Regular/Touch Combo:")
                            }

                            C.ComboBox {
                                id: dpadShortcut1
                                implicitContentWidthPolicy: ComboBox.WidestText
                                firstInFocusChain: false
                                model: [qsTr("Not Used"), qsTr("Cross"), qsTr("Moon"), qsTr("Box"), qsTr("Pyramid"), qsTr("Dpad Left"), qsTr("Dpad Right"), qsTr("Dpad Up"), qsTr("Dpad Down"), qsTr("L1"), qsTr("R1"), qsTr("L3"), qsTr("R3"), qsTr("Options"), qsTr("Share"), qsTr("Touchpad"), qsTr("PS")]
                                currentIndex: dialog.backend.settings.dpadTouchShortcut1
                                onActivated: index => dialog.backend.settings.dpadTouchShortcut1 = index
                                KeyNavigation.priority: {
                                    if(!popup.visible)
                                        KeyNavigation.BeforeItem
                                    else
                                        KeyNavigation.AfterItem
                                }
                                KeyNavigation.up: touchIncrement
                                KeyNavigation.down: posButtons
                                KeyNavigation.left: dpadShortcut1
                                KeyNavigation.right: dpadShortcut2
                            }

                            C.ComboBox {
                                id: dpadShortcut2
                                implicitContentWidthPolicy: ComboBox.WidestText
                                firstInFocusChain: false
                                model: [qsTr("Not Used"), qsTr("Cross"), qsTr("Moon"), qsTr("Box"), qsTr("Pyramid"), qsTr("Dpad Left"), qsTr("Dpad Right"), qsTr("Dpad Up"), qsTr("Dpad Down"), qsTr("L1"), qsTr("R1"), qsTr("L3"), qsTr("R3"), qsTr("Options"), qsTr("Share"), qsTr("Touchpad"), qsTr("PS")]
                                currentIndex: dialog.backend.settings.dpadTouchShortcut2
                                onActivated: index => dialog.backend.settings.dpadTouchShortcut2 = index
                                KeyNavigation.priority: {
                                    if(!popup.visible)
                                        KeyNavigation.BeforeItem
                                    else
                                        KeyNavigation.AfterItem
                                }
                                KeyNavigation.up: touchIncrement
                                KeyNavigation.down: posButtons
                                KeyNavigation.left: dpadShortcut1
                                KeyNavigation.right: dpadShortcut3
                            }

                            C.ComboBox {
                                id: dpadShortcut3
                                implicitContentWidthPolicy: ComboBox.WidestText
                                firstInFocusChain: false
                                model: [qsTr("Not Used"), qsTr("Cross"), qsTr("Moon"), qsTr("Box"), qsTr("Pyramid"), qsTr("Dpad Left"), qsTr("Dpad Right"), qsTr("Dpad Up"), qsTr("Dpad Down"), qsTr("L1"), qsTr("R1"), qsTr("L3"), qsTr("R3"), qsTr("Options"), qsTr("Share"), qsTr("Touchpad"), qsTr("PS")]
                                currentIndex: dialog.backend.settings.dpadTouchShortcut3
                                onActivated: index => dialog.backend.settings.dpadTouchShortcut3 = index
                                KeyNavigation.priority: {
                                    if(!popup.visible)
                                        KeyNavigation.BeforeItem
                                    else
                                        KeyNavigation.AfterItem
                                }
                                KeyNavigation.up: touchIncrement
                                KeyNavigation.down: posButtons
                                KeyNavigation.left: dpadShortcut2
                                KeyNavigation.right: dpadShortcut4
                            }

                            C.ComboBox {
                                id: dpadShortcut4
                                implicitContentWidthPolicy: ComboBox.WidestText
                                firstInFocusChain: false
                                model: [qsTr("Not Used"), qsTr("Cross"), qsTr("Moon"), qsTr("Box"), qsTr("Pyramid"), qsTr("Dpad Left"), qsTr("Dpad Right"), qsTr("Dpad Up"), qsTr("Dpad Down"), qsTr("L1"), qsTr("R1"), qsTr("L3"), qsTr("R3"), qsTr("Options"), qsTr("Share"), qsTr("Touchpad"), qsTr("PS")]
                                currentIndex: dialog.backend.settings.dpadTouchShortcut4
                                onActivated: index => dialog.backend.settings.dpadTouchShortcut4 = index
                                KeyNavigation.priority: {
                                    if(!popup.visible)
                                        KeyNavigation.BeforeItem
                                    else
                                        KeyNavigation.AfterItem
                                }
                                KeyNavigation.up: touchIncrement
                                KeyNavigation.down: posButtons
                                KeyNavigation.left: dpadShortcut3
                                KeyNavigation.right: dpadShortcut4
                            }

                            Label {
                                Layout.alignment: Qt.AlignRight
                                text: qsTr("(L1+R1+dpad Up)")
                            }
                        }
                        RowLayout {
                            spacing: 10
                            Layout.alignment: Qt.AlignHCenter
                            Label {
                                Layout.alignment: Qt.AlignRight
                                text: qsTr("Buttons By Position:")
                            }

                            C.CheckBox {
                                id: posButtons
                                text: qsTr("Use buttons by position instead of by label")
                                checked: dialog.backend.settings.buttonsByPosition
                                onToggled: dialog.backend.settings.buttonsByPosition = checked
                                KeyNavigation.priority: KeyNavigation.BeforeItem
                                KeyNavigation.up: {
                                    if(dpadShortcut1.visible)
                                        dpadShortcut1
                                    else
                                        dpadTouch;
                                }
                                KeyNavigation.down: rumbleHaptics
                                KeyNavigation.left: posButtons
                                KeyNavigation.right: posButtons
                            }

                            Label {
                                Layout.alignment: Qt.AlignRight
                                text: qsTr("(Unchecked)")
                            }
                        }
                        RowLayout {
                            spacing: 10
                            Layout.alignment: Qt.AlignHCenter
                            Label {
                                Layout.alignment: Qt.AlignRight
                                text: qsTr("Rumble Haptics:")
                            }

                            C.ComboBox {
                                id: rumbleHaptics
                                Layout.preferredWidth: 400
                                model: [qsTr("Off"), qsTr("Very Weak"), qsTr("Weak"), qsTr("Normal"), qsTr("Strong"), qsTr("Very Strong")]
                                currentIndex: dialog.backend.settings.rumbleHapticsIntensity
                                onActivated: (index) => dialog.backend.settings.rumbleHapticsIntensity = index;
                            }

                            Label {
                                Layout.alignment: Qt.AlignRight
                                text: qsTr("(Normal)")
                            }
                        }
                        RowLayout {
                            spacing: 10
                            Layout.alignment: Qt.AlignHCenter
                            Label {
                                Layout.alignment: Qt.AlignRight
                                text: qsTr("True Haptics Intensity:")
                            }

                            C.Slider {
                                id: hapticOverride
                                Layout.preferredWidth: 250
                                from: 0
                                to: 2
                                stepSize: 0.1
                                value: dialog.backend.settings.hapticOverride
                                onMoved: dialog.backend.settings.hapticOverride = value;
                                lastInFocusChain: true
                                Label {
                                    anchors {
                                        left: parent.right
                                        verticalCenter: parent.verticalCenter
                                        leftMargin: 10
                                    }
                                    text: {
                                        if(parent.value > 0.99 && parent.value < 1.01)
                                            qsTr("console setting")
                                        else
                                            (parent.value * 100).toFixed(0) + qsTr(" % console setting")
                                    }
                                }
                            }

                            Label {
                                Layout.alignment: Qt.AlignRight
                                Layout.leftMargin: 250
                                text: qsTr("(console setting)")
                            }
                        }
                    }
                }
            }

            Item {
                // Remote (PSN and Hole Punching)
                Flickable {
                    id: remoteFlick
                    anchors {
                        fill: parent
                        topMargin: 20
                        bottomMargin: 20
                    }
                    clip: true
                    contentWidth: Math.max(width, remoteGrid.width)
                    contentHeight: remoteGrid.y + remoteGrid.height
                    flickableDirection: Flickable.AutoFlickIfNeeded
                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AlwaysOn
                        visible: remoteFlick.contentHeight > remoteFlick.height
                    }
                    GridLayout {
                        id: remoteGrid
                        anchors {
                            top: parent.top
                            horizontalCenter: parent.horizontalCenter
                            topMargin: 30
                        }
                        columns: 3
                        rowSpacing: 20
                        columnSpacing: 10

                        C.Button {
                            id: openPsnLogin
                            firstInFocusChain: visible
                            Layout.alignment: Qt.AlignHCenter
                            Layout.columnSpan: 3

                            text: qsTr("Login to PSN")
                            onClicked: {
                                dialog.navigation.showPSNTokenDialog(false)
                            }
                            Material.roundedScale: Material.SmallScale
                            visible: !dialog.backend.settings.psnRefreshToken || !dialog.backend.settings.psnAuthToken || !dialog.backend.settings.psnAuthTokenExpiry || !dialog.backend.settings.psnAccountId
                        }

                        C.Button {
                            id: resetPsnTokens
                            topPadding: 26
                            leftPadding: 30
                            rightPadding: 30
                            bottomPadding: 26
                            text: qsTr("Clear PSN Token")
                            firstInFocusChain: !openPsnLogin.visible
                            Layout.alignment: Qt.AlignHCenter
                            Layout.columnSpan: 3
                            onClicked: {
                                dialog.backend.settings.psnRefreshToken = ""
                                dialog.backend.settings.psnAuthToken = ""
                                dialog.backend.settings.psnAuthTokenExpiry = ""
                                dialog.backend.settings.psnAccountId = ""
                                openPsnLogin.forceActiveFocus(Qt.TabFocusReason);
                            }
                            Material.roundedScale: Material.SmallScale
                            visible: dialog.backend.settings.psnRefreshToken && dialog.backend.settings.psnAuthToken && dialog.backend.settings.psnAuthTokenExpiry && dialog.backend.settings.psnAccountId
                        }


                        Label {
                            Layout.alignment: Qt.AlignRight
                            text: qsTr("Hole Punching Port Guessing:")
                        }

                        C.CheckBox {
                            id: holePunchGuessingCheckbox
                            text: qsTr("Force STUN port guessing")
                            checked: dialog.backend.settings.portGuessingEnabled
                            onToggled: dialog.backend.settings.portGuessingEnabled = checked
                        }

                        Label {
                            Layout.alignment: Qt.AlignRight
                            text: qsTr("(Unchecked)")
                        }

                        Label {
                            Layout.alignment: Qt.AlignRight
                            text: qsTr("Port Guess Count:")
                        }

                        C.Slider {
                            id: portGuessCountSlider
                            Layout.preferredWidth: 250
                            from: 0
                            to: 75
                            stepSize: 1
                            value: dialog.backend.settings.portGuessCount
                            onMoved: dialog.backend.settings.portGuessCount = value

                            Label {
                                anchors {
                                    left: parent.right
                                    verticalCenter: parent.verticalCenter
                                    leftMargin: 10
                                }
                                text: parent.value + qsTr(" guesses")
                            }
                        }

                        Label {
                            Layout.alignment: Qt.AlignRight
                            Layout.leftMargin: 100
                            text: qsTr("(75)")
                        }

                        Label {
                            Layout.alignment: Qt.AlignRight
                            text: qsTr("Port Guess Socket Count:")
                        }

                        C.Slider {
                            id: portGuessSocketSlider
                            Layout.preferredWidth: 250
                            from: 0
                            to: 500
                            stepSize: 1
                            value: dialog.backend.settings.portGuessSocketCount
                            onMoved: dialog.backend.settings.portGuessSocketCount = value
                            lastInFocusChain: true

                            Label {
                                anchors {
                                    left: parent.right
                                    verticalCenter: parent.verticalCenter
                                    leftMargin: 10
                                }
                                text: parent.value + qsTr(" sockets")
                            }
                        }

                        Label {
                            Layout.alignment: Qt.AlignRight
                            text: qsTr("(250)")
                            Layout.leftMargin: 100
                        }
                    }
                }
            }

            Item {
                // Config (Options such as Import/Export and Logging)
                Flickable {
                    id: configFlick
                    anchors {
                        fill: parent
                        topMargin: 20
                        bottomMargin: 20
                    }
                    clip: true
                    contentWidth: Math.max(width, configGrid.width)
                    contentHeight: configGrid.y + configGrid.height
                    flickableDirection: Flickable.AutoFlickIfNeeded
                    ScrollBar.vertical: ScrollBar {
                        policy: ScrollBar.AlwaysOn
                        visible: configFlick.contentHeight > configFlick.height
                    }
                    GridLayout {
                        id: configGrid
                        anchors {
                            top: parent.top
                            horizontalCenter: parent.horizontalCenter
                            topMargin: 30
                        }
                        columns: 1
                        rowSpacing: 20
                        columnSpacing: 10

                        Label {
                            text: {
                                if(dialog.backend.settings.currentProfile)
                                    qsTr("Current Profile: ") + dialog.backend.settings.currentProfile
                                else
                                    qsTr("Current Profile: default")
                            }
                        }

                        C.Button {
                            id: profile
                            firstInFocusChain: true
                            text: qsTr("Manage Profiles")
                            onClicked: {
                                dialog.navigation.showProfileDialog()
                            }
                            Material.roundedScale: Material.SmallScale
                        }

                    C.Button {
                        id: exportButton
                        text: qsTr("Export settings to file")
                        onClicked: {
                            dialog.backend.settings.exportSettings();
                        }
                        Material.roundedScale: Material.SmallScale
                    }

                    C.Button {
                        id: importButton
                        text: qsTr("Import settings from file")
                        onClicked: {
                            dialog.backend.settings.importSettings();
                        }
                        Material.roundedScale: Material.SmallScale
                    }

                    C.Button {
                        id: aboutButton
                        text: qsTr("About %1-ng").arg(Qt.application.name)
                        onClicked: aboutDialog.open()
                        Material.roundedScale: Material.SmallScale
                    }

                    C.CheckBox {
                        text: qsTr("Sanitize Logs (checked)")
                        checked: dialog.backend.settings.logSanitize
                        onToggled: dialog.backend.settings.logSanitize = checked
                    }

                        C.CheckBox {
                            text: qsTr("Verbose Logging (unchecked)")
                            checked: dialog.backend.settings.logVerbose
                            lastInFocusChain: true
                            onToggled: dialog.backend.settings.logVerbose = checked
                        }
                    }
                }
            }
        }

        Connections {
            target: dialog.Window.window
            function onActiveFocusItemChanged() {
                dialog.ensureActiveFocusVisible()
            }
        }

        Item {
            Timer {
                id: openTimer
                interval: 100
                running: false
                onTriggered: {
                    if(controllerMappingDialog.resetMapping)
                    {
                        if(!dialog.backend.controllerMappingDefaultMapping)
                        {
                            quitControllerMapping = false;
                            dialog.backend.controllerMappingReset();
                        }
                        controllerMappingDialog.close();
                    }
                    else
                    {
                        controllerMappingChange.forceActiveFocus(Qt.TabFocusReason);
                        dialog.navigation.showControllerMappingDialog();
                        quitControllerMapping = false;
                        controllerMappingDialog.resetFocus = false;
                        controllerMappingDialog.close();
                    }
                }
            }
        }

        Dialog {
            id: aboutDialog
            parent: Overlay.overlay
            x: Math.round((dialog.navigation.width - width) / 2)
            y: Math.round((dialog.navigation.height - height) / 2)
            title: qsTr("About %1-ng").arg(Qt.application.name)
            modal: true
            standardButtons: Dialog.Ok
            Material.roundedScale: Material.MediumScale
            onAboutToHide: aboutButton.forceActiveFocus(Qt.TabFocusReason)

            RowLayout {
                spacing: 50
                onVisibleChanged: if (visible) forceActiveFocus(Qt.TabFocusReason)
                Keys.onReturnPressed: aboutDialog.close()
                Keys.onEscapePressed: aboutDialog.close()

                Image {
                    Layout.preferredWidth: 200
                    fillMode: Image.PreserveAspectFit
                    verticalAlignment: Image.AlignTop
                    source: "qrc:icons/chiaking-logo.svg"
                }

                Label {
                    Layout.preferredWidth: 400
                    verticalAlignment: Text.AlignTop
                    wrapMode: Text.Wrap
                    text: "<h1>chiaki-ng</h1> by Street Pea, version %1
                        <h2>Fork of Chiaki</h2> by Florian Markl at version 2.1.1

                        <p>This program is free software: you can redistribute it and/or modify
                        it under the terms of the GNU Affero General Public License version 3
                        as published by the Free Software Foundation.</p>

                        <p>This program is distributed in the hope that it will be useful,
                        but WITHOUT ANY WARRANTY; without even the implied warranty of
                        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
                        GNU General Public License for more details.</p>".arg(Qt.application.version)
                }
            }
        }

        Dialog {
            id: keyDialog
            focus: false
            property int buttonValue
            property var buttonCallback
            property var keysIndex
            parent: Overlay.overlay
            x: Math.round((dialog.navigation.width - width) / 2)
            y: Math.round((dialog.navigation.height - height) / 2)
            title: qsTr("Key Capture")
            modal: true
            standardButtons: Dialog.Close
            closePolicy: Popup.CloseOnPressOutside
            onOpened: keyLabel.forceActiveFocus(Qt.TabFocusReason)
            onClosed: {
                let item = chiakiKeys.itemAt(keysIndex)
                if(item)
                {
                    let item2 = item.children[1];
                    if(item2)
                        item2.forceActiveFocus(Qt.TabFocusReason);
                }
                keyLabel.focus = false;
                focus = false;
            }
            Material.roundedScale: Material.MediumScale

            function show(opts) {
                buttonValue = opts.value;
                buttonCallback = opts.callback;
                keysIndex = opts.mappingIndex;
                open();
            }

            Label {
                id: keyLabel
                focus: true
                text: qsTr("Press any key to configure button or click close")
                Keys.onReleased: (event) => {
                    var name = dialog.backend.settings.changeControllerKey(keyDialog.buttonValue, event.key);
                    keyDialog.buttonCallback(name);
                    keyDialog.close();
                }
            }
        }

        Dialog {
            id: controllerMappingDialog
            property bool resetFocus: true
            property bool resetMapping: false
            parent: Overlay.overlay
            x: Math.round((dialog.navigation.width - width) / 2)
            y: Math.round((dialog.navigation.height - height) / 2)
            title: qsTr("Controller Capture")
            modal: true
            standardButtons: Dialog.Close
            closePolicy: Popup.CloseOnPressOutside
            onOpened: {
                controllerLabel.forceActiveFocus(Qt.TabFocusReason);
                dialog.backend.beginControllerMapping(resetMapping);
            }
            onClosed: {
                if(resetFocus)
                {
                    if(resetMapping)
                        controllerMappingReset.forceActiveFocus(Qt.TabFocusReason);
                    else
                        controllerMappingChange.forceActiveFocus(Qt.TabFocusReason);
                    focus = false;
                }
                else
                {
                    resetFocus = true;
                    focus = false;
                }
                if(quitControllerMapping)
                    dialog.backend.controllerMappingQuit();
                else
                    quitControllerMapping = true;
            }
            Material.roundedScale: Material.MediumScale

            function show(opts) {
                resetMapping = opts.reset;
                open();
            }
            Label {
                id: controllerLabel
                text: qsTr("Choose the controller by pressing any button on the controller")
            }
        }

        Dialog {
            id: steamControllerMappingDialog
            property bool resetMapping: false
            parent: Overlay.overlay
            x: Math.round((dialog.navigation.width - width) / 2)
            y: Math.round((dialog.navigation.height - height) / 2)
            title: qsTr("Controller Managed by Steam")
            modal: true
            standardButtons: Dialog.Close
            closePolicy: Popup.NoAutoClose
            onOpened: {
                steamLabel.forceActiveFocus(Qt.TabFocusReason);
            }
            onClosed: {
                if(resetMapping)
                    controllerMappingReset.forceActiveFocus(Qt.TabFocusReason);
                else
                    controllerMappingChange.forceActiveFocus(Qt.TabFocusReason);
                focus = false;
            }
            Material.roundedScale: Material.MediumScale

            Label {
                id: steamLabel
                wrapMode: TextEdit.Wrap
                text: qsTr("This controller is managed by Steam.\nPlease use Steam to map controller or disable Steam Input for the controller before mapping here.")
                Keys.onReturnPressed: steamControllerMappingDialog.close();
                Keys.onEscapePressed: steamControllerMappingDialog.close();
            }
        }

        Connections {
            target: dialog.backend

            function onControllerMappingInProgressChanged()
            {
                if(dialog.backend.controllerMappingInProgress)
                    openTimer.start();
            }

            function onControllerMappingSteamControllerSelected()
            {
                controllerMappingDialog.resetFocus = false;
                quitControllerMapping = false;
                steamControllerMappingDialog.resetMapping = controllerMappingDialog.resetMapping;
                controllerMappingDialog.close();
                steamControllerMappingDialog.open();
            }
        }
    }
}
