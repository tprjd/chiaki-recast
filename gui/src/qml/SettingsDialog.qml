import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs

import org.streetpea.chiaking

import "controls" as C
import "SettingsSearch.js" as Search

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
    property var pendingMappingSearch: null
    readonly property string searchQuery: searchField.text.trim().toLocaleLowerCase()
    readonly property var searchResults: {
        const query = searchQuery;
        if (!query) return [];
        return Search.entries(backend.settings).map(entry => {
            if ((entry.page === "renderer" || entry.page === "color") && backend.window.videoPreset !== ChiakiWindow.VideoPreset.Custom)
                entry.description += " " + qsTr("Requires the Custom render preset in Video.");
            if (entry.page === "color" && (!backend.settings.placeboColorMappingEnabled || backend.settings.placeboColorMappingPreset !== 0))
                entry.description += " " + qsTr("Requires Color mapping enabled and the Custom color mapping preset in Renderer.");
            return entry;
        }).filter(entry =>
            (entry.title + " " + entry.description + " " + (entry.keywords || "") + " " + searchContext(entry))
                .toLocaleLowerCase().indexOf(query) >= 0);
    }
    function searchContext(entry) {
        const page = entry.page === "renderer" ? qsTr("Renderer")
            : entry.page === "color" ? qsTr("Renderer") + " / " + qsTr("Color Mapping")
            : entry.page === "display" ? qsTr("Display") : "";
        return categories[entry.category] + " / " + (page ? page + " / " : "") + entry.group;
    }
    function selectCategory(index) {
        searchField.clear();
        bar.currentIndex = index;
        focusCurrentTabFirstItem();
    }
    function revealSetting(category, name, consoleType, fallback, afterReveal) {
        searchField.clear();
        bar.currentIndex = category;
        if (consoleType !== undefined) selectedConsole = consoleType;
        Qt.callLater(function() {
            let item = Search.findItem(dialog, name);
            const consoleAction = ["autoConnectConsole", "deleteRegisteredConsole", "unhideConsole"]
                .find(prefix => name === prefix + "0");
            if (consoleAction) {
                const hosts = consoleAction === "unhideConsole" ? backend.hiddenHosts : backend.settings.registeredHosts;
                for (let index = 0; index < hosts.length; ++index) {
                    const candidate = Search.findItem(dialog, consoleAction + index);
                    if (candidate && candidate.visible && candidate.enabled) { item = candidate; break; }
                }
            }
            if ((!item || !item.visible || !item.enabled) && fallback) item = Search.findItem(dialog, fallback);
            if (item && item.visible && item.enabled) item.forceActiveFocus(Qt.TabFocusReason);
            else focusCurrentTabFirstItem();
            ensureActiveFocusVisible();
            if (afterReveal) afterReveal(item);
        });
    }
    function openSearchResult(index) {
        const entry = searchResults[index];
        if (!entry) return;
        pendingMappingSearch = null;
        if (!entry.page) {
            revealSetting(entry.category, entry.target, entry.consoleType, entry.fallback);
            return;
        }
        const opener = entry.page === "display" ? "displaySettings"
            : entry.page === "profiles" ? "manageProfiles"
            : entry.page === "mapping" ? "controllerMappingChange" : "customRendererSettings";
        revealSetting(entry.category, opener, undefined, "videoPreset", function(item) {
            if (!item || item.objectName !== opener || !item.visible || !item.enabled) return;
            if (entry.page === "display") navigation.showDisplaySettingsDialog(entry);
            else if (entry.page === "profiles") navigation.showProfileDialog(entry);
            else if (entry.page === "mapping") {
                pendingMappingSearch = entry;
                controllerMappingDialog.show({reset: false});
            } else navigation.showPlaceboSettingsDialog(entry);
        });
    }
    readonly property bool compact: width < 760
    readonly property var categories: [qsTr("General"), qsTr("Video"), qsTr("Stream"),
        qsTr("Audio/Wifi"), qsTr("Consoles"), qsTr("Keyboard and mouse"), qsTr("Controllers"), qsTr("Remote"), qsTr("Config")]
    property int consoleProfileEpoch: 0
    property string consoleProfileKey: backend.settings.currentProfile
    property var pendingConsoleAction: null
    onConsoleProfileKeyChanged: consoleProfileEpoch++
    function consoleIndex(hosts, mac) {
        if (!mac) return -1;
        let result = -1;
        for (let index = 0; index < hosts.length; index++) {
            if (hosts[index].mac !== mac) continue;
            if (result !== -1) return -1;
            result = index;
        }
        return result;
    }
    function requestConsoleAction(action, mac, opener) {
        const hosts = action === "delete" ? backend.settings.registeredHosts : backend.hiddenHosts;
        if (consoleIndex(hosts, mac) < 0) return;
        const request = { mac: mac, epoch: consoleProfileEpoch, opener: opener };
        pendingConsoleAction = request;
        const restoreFocus = function() {
            Qt.callLater(function() {
                if (request.opener && request.opener.visible && request.opener.enabled)
                    request.opener.forceActiveFocus(Qt.TabFocusReason);
                else if (bar.currentIndex === 4)
                    registerNewButton.forceActiveFocus(Qt.TabFocusReason);
            });
        };
        const accept = function() {
            if (pendingConsoleAction !== request) return;
            pendingConsoleAction = null;
            if (request.epoch === consoleProfileEpoch) {
                const currentHosts = action === "delete" ? backend.settings.registeredHosts : backend.hiddenHosts;
                const index = consoleIndex(currentHosts, request.mac);
                if (index >= 0) {
                    if (action === "delete") backend.settings.deleteRegisteredHost(index);
                    else backend.unhideHost(request.mac);
                }
            }
            restoreFocus();
        };
        const cancel = function() {
            if (pendingConsoleAction !== request) return;
            pendingConsoleAction = null;
            restoreFocus();
        };
        navigation.showConfirmDialog(action === "delete" ? qsTr("Delete Console") : qsTr("Unhide Console"),
            action === "delete" ? qsTr("Are you sure you want to delete this console?") : qsTr("Are you sure you want to unhide this console?"),
            action === "delete" ? qsTr("Delete console") : qsTr("Unhide console"), accept, cancel);
    }
    function close() { navigation.closeDialog(); }
    Keys.onEscapePressed: {
        if (keyDialog.visible) keyDialog.close();
        else if (searchField.text.length) { searchField.clear(); searchField.forceActiveFocus(); }
        else close();
    }
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
        let target = flickVisibilityTarget(flick, item);
        if (target.height > flick.height) {
            target = item;
            while (target.parent && target.parent !== flick.contentItem && target.parent.height <= flick.height)
                target = target.parent;
        }
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
        case 5: item = dialog.backend.settings.mouseCameraSupported ? mouseCameraEnabled : resetAllKeys; break;
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
        if (searchField.activeFocus || searchQuery.length) return;
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
        RowLayout {
            id: searchRow
            anchors { top: dialog.compact ? profileCompact.bottom : heading.bottom; left: parent.left; right: parent.right; topMargin: 12 }
            spacing: 12
            TextField {
                id: searchField
                objectName: "settingsSearch"
                Layout.fillWidth: true
                placeholderText: qsTr("Search settings")
                Accessible.name: qsTr("Search settings")
                onActiveFocusChanged: if (activeFocus) resultList.navigating = false
                Keys.onPressed: event => {
                    if (event.key === Qt.Key_Down && resultList.count) {
                        resultList.currentIndex = 0;
                        if (resultList.currentItem) resultList.currentItem.forceActiveFocus(Qt.TabFocusReason);
                        event.accepted = true;
                    } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                        dialog.openSearchResult(0);
                        event.accepted = true;
                    } else if (event.key === Qt.Key_Escape && text.length) {
                        clear();
                        event.accepted = true;
                    } else if (event.key === Qt.Key_Up || event.key === Qt.Key_PageUp || event.key === Qt.Key_PageDown) {
                        event.accepted = true;
                    }
                }
            }
            C.Button {
                objectName: "clearSettingsSearch"
                text: qsTr("Clear")
                enabled: searchField.text.length > 0
                onClicked: { searchField.clear(); searchField.forceActiveFocus(); }
            }
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
            anchors { top: searchRow.bottom; left: parent.left; right: parent.right; topMargin: 8 }
            visible: dialog.compact
            model: dialog.categories
            currentIndex: bar.currentIndex
            Accessible.name: qsTr("Settings category")
            onActivated: index => dialog.selectCategory(index)
        }
        Flickable {
            id: sidebar
            anchors { top: searchRow.bottom; left: parent.left; bottom: parent.bottom; topMargin: 20 }
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
                        contentItem: Label {
                            text: parent.text
                            font: parent.font
                            wrapMode: Text.Wrap
                            verticalAlignment: Text.AlignVCenter
                        }
                        Layout.fillWidth: true
                        checkable: true
                        autoExclusive: true
                        checked: bar.currentIndex === index
                        highlighted: checked
                        font.pixelSize: 16
                        topPadding: 8
                        bottomPadding: 8
                        Layout.preferredHeight: Math.max(44, contentItem.implicitHeight + topPadding + bottomPadding)
                        background: Rectangle {
                            radius: 7
                            color: parent.checked ? "#24394c" : parent.hovered ? "#1b2633" : "transparent"
                            border.color: parent.visualFocus ? Material.accent : "transparent"
                            border.width: 2
                        }
                        Accessible.name: modelData
                        Accessible.description: highlighted ? qsTr("Selected category") : ""
                        onClicked: dialog.selectCategory(index)
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
            visible: !dialog.searchQuery.length
            anchors {
                top: dialog.compact ? categorySelector.bottom : searchRow.bottom
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
                    objectName: "videoFlick"
                    anchors.fill: parent
                    anchors.margins: 16
                    clip: true
                    contentWidth: width
                    contentHeight: videoGrid.height
                    flickableDirection: Flickable.VerticalFlick
                    ScrollBar.vertical: ScrollBar {}
                    ColumnLayout {
                        id: videoGrid
                        width: Math.max(0, videoFlick.width - 16)
                        spacing: 14
                        Label { text: qsTr("Decoding"); font.pixelSize: 14; font.bold: true }
                        Frame {
                            Layout.fillWidth: true
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            GridLayout {
                                anchors.fill: parent
                                columns: videoFlick.width < 650 ? 1 : 2
                                columnSpacing: 24
                                rowSpacing: 12
                            Label { text: qsTr("Hardware decoder\nDefault: Auto"); font.pixelSize: 14; wrapMode: Text.Wrap; Layout.fillWidth: true }
                            C.ComboBox {
                                id: hwDecoderCombo
                                objectName: "decoder"
                                Accessible.name: qsTr("Hardware decoder")
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                                Layout.preferredWidth: 300
                                font.pixelSize: 14
                                popup.width: width
                                model: dialog.backend.settings.availableDecoders
                                currentIndex: Math.max(0, model.indexOf(dialog.backend.settings.decoder))
                                onActivated: index => { if (index >= 0 && index < model.length) dialog.backend.settings.decoder = index ? model[index] : ""; }
                                firstInFocusChain: true
                            }
                            C.CheckBox {
                                objectName: "useZeroCopy"
                                Layout.columnSpan: videoFlick.width < 650 ? 1 : 2
                                Layout.fillWidth: true
                                font.pixelSize: 14
                                text: qsTr("Zero-copy, default: On")
                                contentItem: Label { text: parent.text; font.pixelSize: 14; wrapMode: Text.Wrap; leftPadding: parent.indicator.width + parent.spacing; verticalAlignment: Text.AlignVCenter }
                                checked: dialog.backend.settings.useZeroCopy
                                onToggled: dialog.backend.settings.useZeroCopy = checked
                            }
                            }
                        }
                        Label { text: qsTr("Stream window"); font.pixelSize: 14; font.bold: true }
                        Frame {
                            Layout.fillWidth: true
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            GridLayout {
                                anchors.fill: parent
                                columns: videoFlick.width < 650 ? 1 : 2
                                columnSpacing: 24
                                rowSpacing: 12
                            Label { text: qsTr("Window type\nDefault: Fullscreen"); font.pixelSize: 14; wrapMode: Text.Wrap; Layout.fillWidth: true }
                            C.ComboBox {
                                id: windowTypeCombo
                                objectName: "windowType"
                                Accessible.name: qsTr("Window type")
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                                Layout.preferredWidth: 300
                                font.pixelSize: 14
                                popup.width: width
                                model: [qsTr("Stream Resolution"), qsTr("Custom Resolution"), qsTr("Adjust Resolution Manually"), qsTr("Fullscreen"), qsTr("Zoom [adjust zoom using slider in stream menu]"), qsTr("Stretch")]
                                currentIndex: dialog.backend.settings.windowType
                                onActivated: index => { dialog.backend.settings.windowType = index; }

                            }
                            Label { text: qsTr("Custom resolution width\nDefault: 1920 pixels"); font.pixelSize: 14; wrapMode: Text.Wrap; Layout.fillWidth: true; visible: dialog.backend.settings.windowType === 1 }
                            C.TextField {
                                id: customResolutionWidth
                                objectName: "customResolutionWidth"
                                Accessible.name: qsTr("Custom resolution width in pixels")
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                                font.pixelSize: 14
                                visible: dialog.backend.settings.windowType === 1
                                text: dialog.backend.settings.customResolutionWidth
                                property bool edited: false
                                property string editProfile: ""
                                onTextEdited: { edited = true; editProfile = dialog.backend.settings.currentProfile; }
                                Material.accent: text && !validate() ? Material.Red : undefined
                                function validate() { const number = parseInt(text); return number >= 0 && number <= 9999; }
                                onEditingFinished: {
                                    if (!edited) return;
                                    edited = false;
                                    if (editProfile === dialog.backend.settings.currentProfile) {
                                        if (!validate()) {
                                            dialog.backend.settings.customResolutionWidth = 0;
                                            text = "";
                                            return;
                                        }
                                        dialog.backend.settings.customResolutionWidth = parseInt(text);
                                    }
                                    text = Qt.binding(function() { return dialog.backend.settings.customResolutionWidth; });
                                }
                                Connections {
                                    target: dialog.backend.settings
                                    function onCurrentProfileChanged() {
                                        customResolutionWidth.edited = false;
                                        customResolutionWidth.text = Qt.binding(function() { return dialog.backend.settings.customResolutionWidth; });
                                    }
                                }
                            }
                            Label { text: qsTr("Custom resolution height\nDefault: 1080 pixels"); font.pixelSize: 14; wrapMode: Text.Wrap; Layout.fillWidth: true; visible: dialog.backend.settings.windowType === 1 }
                            C.TextField {
                                id: customResolutionHeight
                                objectName: "customResolutionHeight"
                                Accessible.name: qsTr("Custom resolution height in pixels")
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                                font.pixelSize: 14
                                visible: dialog.backend.settings.windowType === 1
                                text: dialog.backend.settings.customResolutionHeight
                                property bool edited: false
                                property string editProfile: ""
                                onTextEdited: { edited = true; editProfile = dialog.backend.settings.currentProfile; }
                                Material.accent: text && !validate() ? Material.Red : undefined
                                function validate() { const number = parseInt(text); return number >= 0 && number <= 9999; }
                                onEditingFinished: {
                                    if (!edited) return;
                                    edited = false;
                                    if (editProfile === dialog.backend.settings.currentProfile) {
                                        if (!validate()) {
                                            dialog.backend.settings.customResolutionHeight = 0;
                                            text = "";
                                            return;
                                        }
                                        dialog.backend.settings.customResolutionHeight = parseInt(text);
                                    }
                                    text = Qt.binding(function() { return dialog.backend.settings.customResolutionHeight; });
                                }
                                Connections {
                                    target: dialog.backend.settings
                                    function onCurrentProfileChanged() {
                                        customResolutionHeight.edited = false;
                                        customResolutionHeight.text = Qt.binding(function() { return dialog.backend.settings.customResolutionHeight; });
                                    }
                                }
                            }
                            C.CheckBox {
                                objectName: "fullscreenDoubleClick"
                                Layout.columnSpan: videoFlick.width < 650 ? 1 : 2
                                Layout.fillWidth: true
                                font.pixelSize: 14
                                text: qsTr("Toggle fullscreen on double-click, default: Off")
                                contentItem: Label { text: parent.text; font.pixelSize: 14; wrapMode: Text.Wrap; leftPadding: parent.indicator.width + parent.spacing; verticalAlignment: Text.AlignVCenter }
                                checked: dialog.backend.settings.fullscreenDoubleClick
                                onToggled: dialog.backend.settings.fullscreenDoubleClick = checked
                            }
                            C.CheckBox {
                                objectName: "hideCursor"
                                Layout.columnSpan: videoFlick.width < 650 ? 1 : 2
                                Layout.fillWidth: true
                                font.pixelSize: 14
                                text: qsTr("Hide cursor during stream, default: On")
                                contentItem: Label { text: parent.text; font.pixelSize: 14; wrapMode: Text.Wrap; leftPadding: parent.indicator.width + parent.spacing; verticalAlignment: Text.AlignVCenter }
                                checked: dialog.backend.settings.hideCursor
                                onToggled: dialog.backend.settings.hideCursor = checked
                            }
                            }
                        }
                        Label { text: qsTr("Rendering"); font.pixelSize: 14; font.bold: true }
                        Frame {
                            Layout.fillWidth: true
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            GridLayout {
                                anchors.fill: parent
                                columns: videoFlick.width < 650 ? 1 : 2
                                columnSpacing: 24
                                rowSpacing: 12
                            C.CheckBox {
                                objectName: "vSyncEnabled"
                                Layout.columnSpan: videoFlick.width < 650 ? 1 : 2
                                Layout.fillWidth: true
                                font.pixelSize: 14
                                text: qsTr("Vertical sync, default: On")
                                contentItem: Label { text: parent.text; font.pixelSize: 14; wrapMode: Text.Wrap; leftPadding: parent.indicator.width + parent.spacing; verticalAlignment: Text.AlignVCenter }
                                checked: dialog.backend.settings.vSyncEnabled
                                onToggled: {
                                    dialog.backend.settings.vSyncEnabled = checked;
                                    if (dialog.backend.window.runtimeRendererBackend === 1) dialog.backend.settings.restartApplication();
                                }
                            }
                            Label { text: qsTr("Render preset\nDefault: High Quality"); font.pixelSize: 14; wrapMode: Text.Wrap; Layout.fillWidth: true }
                            C.ComboBox {
                                id: videoPreset
                                objectName: "videoPreset"
                                Accessible.name: qsTr("Render preset")
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                                Layout.preferredWidth: 300
                                font.pixelSize: 14
                                popup.width: width
                                model: [qsTr("Fast"), qsTr("Default"), qsTr("High Quality"), qsTr("High Quality + Spatial Upscaling"), qsTr("High Quality + Advanced Spatial Upscaling"), qsTr("Custom")]
                                currentIndex: dialog.backend.settings.videoPreset
                                onActivated: index => { dialog.backend.settings.videoPreset = index;
                                    const presets = [ChiakiWindow.VideoPreset.Fast, ChiakiWindow.VideoPreset.Default, ChiakiWindow.VideoPreset.HighQuality, ChiakiWindow.VideoPreset.HighQualitySpatial, ChiakiWindow.VideoPreset.HighQualityAdvancedSpatial, ChiakiWindow.VideoPreset.Custom];
                                    dialog.backend.window.videoPreset = presets[index]; }

                            }
                            Label { text: qsTr("Placebo queue depth limit\nDefault: 2 frames"); font.pixelSize: 14; wrapMode: Text.Wrap; Layout.fillWidth: true }
                            ColumnLayout {
                                Layout.fillWidth: true
                                C.Slider {
                                    objectName: "queueDepthLimit"
                                    Accessible.name: qsTr("Placebo queue depth limit in frames")
                                    Layout.fillWidth: true
                                    from: 2
                                    to: 5
                                    stepSize: 1
                                    value: dialog.backend.settings.queueDepthLimit
                                    onMoved: dialog.backend.settings.queueDepthLimit = value
                                }
                                Label { objectName: "queueDepthLimitValue"; text: qsTr("%1 frames").arg(dialog.backend.settings.queueDepthLimit); font.pixelSize: 14 }
                            }
                            Label { text: qsTr("Renderer backend\nDefault: Vulkan"); font.pixelSize: 14; wrapMode: Text.Wrap; Layout.fillWidth: true }
                            C.ComboBox {
                                id: rendererBackend
                                objectName: "rendererBackend"
                                Accessible.name: qsTr("Renderer backend")
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                                Layout.preferredWidth: 300
                                font.pixelSize: 14
                                popup.width: width
                                model: [qsTr("Vulkan"), qsTr("OpenGL")]
                                currentIndex: dialog.backend.settings.rendererBackend
                                onActivated: index => { if (index !== dialog.backend.settings.rendererBackend) { dialog.backend.settings.rendererBackend = index; dialog.backend.settings.restartApplication(); } }

                            }
                            C.Button {
                                id: customRendererSettings
                                objectName: "customRendererSettings"
                                text: qsTr("Custom renderer settings")
                                font.pixelSize: 14
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                                visible: dialog.backend.window.videoPreset === ChiakiWindow.VideoPreset.Custom
                                onClicked: dialog.navigation.showPlaceboSettingsDialog()
                            }
                            C.Button {
                                id: displaySettings
                                objectName: "displaySettings"
                                text: qsTr("Display settings")
                                font.pixelSize: 14
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                                lastInFocusChain: true
                                onClicked: dialog.navigation.showDisplaySettingsDialog()
                            }
                            }
                        }
                    }
                }
            }

            Item {
                // Stream
                Flickable {
                    id: streamFlick
                    objectName: "streamFlick"
                    anchors.fill: parent
                    clip: true
                    contentWidth: width
                    contentHeight: streamGrid.height
                    flickableDirection: Flickable.VerticalFlick
                    ScrollBar.vertical: ScrollBar {}
                    ColumnLayout {
                        id: streamGrid
                        width: Math.max(0, streamFlick.width - 14)
                        spacing: 16
                        Label { text: qsTr("Stream"); font.pixelSize: 24; font.bold: true }
                        Label { text: qsTr("Settings for"); font.pixelSize: 14 }
                        C.ComboBox {
                            id: consoleSelection
                            objectName: "consoleSelection"
                            Accessible.name: qsTr("Console type")
                            Layout.fillWidth: true
                            Layout.maximumWidth: 400
                            font.pixelSize: 14
                            model: [qsTr("PS4"), qsTr("PS5")]
                            currentIndex: dialog.selectedConsole
                            onActivated: index => dialog.selectedConsole = index
                            firstInFocusChain: true
                            KeyNavigation.priority: popup.visible ? KeyNavigation.AfterItem : KeyNavigation.BeforeItem
                            KeyNavigation.down: localStreamRoute.resolutionControl
                        }
                        GridLayout {
                            Layout.fillWidth: true
                            columns: streamFlick.width < 700 ? 1 : 2
                            columnSpacing: 16
                            rowSpacing: 16
                            component StreamRoute: Frame {
                                id: route
                                required property int index
                                property Item peer
                                readonly property bool ps5: dialog.selectedConsole === SettingsDialog.Console.PS5
                                readonly property string suffix: (index === 0 ? "Local" : "Remote") + (ps5 ? "PS5" : "PS4")
                                readonly property string routeName: index === 0 ? qsTr("Local") : qsTr("Remote")
                                property alias resolutionControl: resolution
                                property alias fpsControl: fps
                                property alias codecControl: codec
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                                Layout.alignment: Qt.AlignTop
                                background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                                ColumnLayout {
                                    anchors.fill: parent
                                    spacing: 8
                                    Label { text: route.routeName; font.pixelSize: 18; font.bold: true }
                                    Label { text: qsTr("Resolution"); font.pixelSize: 14 }
                                    C.ComboBox {
                                        id: resolution
                                        objectName: "resolution" + route.suffix
                                        Accessible.name: qsTr("%1 resolution").arg(route.routeName)
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        font.pixelSize: 14
                                        model: [qsTr("360p"), qsTr("540p"), qsTr("720p"), qsTr("1080p")]
                                        currentIndex: dialog.backend.settings["resolution" + route.suffix] - 1
                                        onActivated: index => {
                                            dialog.backend.settings["resolution" + route.suffix] = index + 1;
                                            dialog.backend.settings["bitrate" + route.suffix] = 0;
                                        }
                                        KeyNavigation.priority: popup.visible ? KeyNavigation.AfterItem : KeyNavigation.BeforeItem
                                        KeyNavigation.up: consoleSelection
                                        KeyNavigation.down: fps
                                        KeyNavigation.left: route.index === 1 && route.peer ? route.peer.resolutionControl : null
                                        KeyNavigation.right: route.index === 0 && route.peer ? route.peer.resolutionControl : null
                                    }
                                    Label {
                                        Layout.fillWidth: true
                                        font.pixelSize: 14
                                        wrapMode: Text.Wrap
                                        opacity: 0.65
                                        text: !route.ps5 ? qsTr("Default: 720p. 1080p requires PS4 Pro.")
                                            : route.index === 0 ? qsTr("Default: 1080p") : qsTr("Default: 720p")
                                    }
                                    Label { text: qsTr("Frame rate"); font.pixelSize: 14 }
                                    C.ComboBox {
                                        id: fps
                                        objectName: "fps" + route.suffix
                                        Accessible.name: qsTr("%1 frame rate").arg(route.routeName)
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        font.pixelSize: 14
                                        model: [qsTr("30 fps"), qsTr("60 fps (Default)")]
                                        currentIndex: dialog.backend.settings["fps" + route.suffix] / 30 - 1
                                        onActivated: index => dialog.backend.settings["fps" + route.suffix] = (index + 1) * 30
                                        KeyNavigation.priority: popup.visible ? KeyNavigation.AfterItem : KeyNavigation.BeforeItem
                                        KeyNavigation.up: resolution
                                        KeyNavigation.down: bitrate
                                        KeyNavigation.left: route.index === 1 && route.peer ? route.peer.fpsControl : null
                                        KeyNavigation.right: route.index === 0 && route.peer ? route.peer.fpsControl : null
                                    }
                                    Label { text: qsTr("Bitrate"); font.pixelSize: 14 }
                                    ColumnLayout {
                                        Layout.fillWidth: true
                                        spacing: 8
                                        C.Slider {
                                            id: bitrate
                                            objectName: "bitrate" + route.suffix
                                            Accessible.name: qsTr("%1 bitrate in Mbps").arg(route.routeName)
                                            readonly property real automaticBitrate: {
                                                switch (dialog.backend.settings["resolution" + route.suffix]) {
                                                case 1: return 2;
                                                case 2: return 6;
                                                case 3: return 10;
                                                case 4: return 15;
                                                default: return 0;
                                                }
                                            }
                                            Layout.fillWidth: true
                                            Layout.minimumWidth: 0
                                            from: 2
                                            to: 100
                                            stepSize: 1
                                            value: dialog.backend.settings["bitrate" + route.suffix] / 1000 || automaticBitrate
                                            onMoved: dialog.backend.settings["bitrate" + route.suffix] = value * 1000
                                            KeyNavigation.priority: KeyNavigation.BeforeItem
                                            KeyNavigation.up: fps
                                            KeyNavigation.down: route.ps5 ? codec : bitrate
                                        }
                                        Label {
                                            objectName: "bitrateDescription" + route.suffix
                                            Layout.fillWidth: true
                                            font.pixelSize: 14
                                            wrapMode: Text.Wrap
                                            text: dialog.backend.settings["bitrate" + route.suffix] === 0
                                                ? qsTr("Automatic: %1 Mbps").arg(bitrate.value)
                                                : qsTr("%1 Mbps, default: %2 Mbps").arg(bitrate.value).arg(bitrate.automaticBitrate)
                                        }
                                    }
                                    Label {
                                        Layout.fillWidth: true
                                        font.pixelSize: 14
                                        wrapMode: Text.Wrap
                                        opacity: 0.65
                                        text: qsTr("Changing resolution restores automatic bitrate.")
                                    }
                                    Label { text: qsTr("Codec"); font.pixelSize: 14; visible: route.ps5 }
                                    C.ComboBox {
                                        id: codec
                                        objectName: "codec" + route.suffix
                                        Accessible.name: qsTr("%1 codec").arg(route.routeName)
                                        Layout.fillWidth: true
                                        Layout.minimumWidth: 0
                                        font.pixelSize: 14
                                        model: [qsTr("H264"), qsTr("H265 (Default)"), qsTr("H265 HDR")]
                                        visible: route.ps5
                                        currentIndex: route.ps5 ? dialog.backend.settings[route.index === 0 ? "codecLocalPS5" : "codecRemotePS5"] : -1
                                        onActivated: index => {
                                            if (route.ps5)
                                                dialog.backend.settings["codec" + route.suffix] = index;
                                        }
                                        KeyNavigation.priority: popup.visible ? KeyNavigation.AfterItem : KeyNavigation.BeforeItem
                                        KeyNavigation.up: bitrate
                                        KeyNavigation.down: codec
                                        KeyNavigation.left: route.index === 1 && route.peer ? route.peer.codecControl : null
                                        KeyNavigation.right: route.index === 0 && route.peer ? route.peer.codecControl : null
                                    }
                                }
                            }
                            StreamRoute { id: localStreamRoute; index: 0; peer: remoteStreamRoute }
                            StreamRoute { id: remoteStreamRoute; index: 1; peer: localStreamRoute }
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
                                columns: 2
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
                                columns: 2
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
                                columns: 2
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
                    objectName: "consolesPage"
                    anchors.fill: parent
                    contentWidth: width
                    contentHeight: consolesLayout.height
                    clip: true
                    flickableDirection: Flickable.VerticalFlick
                    ScrollBar.vertical: ScrollBar {}
                    ColumnLayout {
                        id: consolesLayout
                        width: consolesFlick.width - 12
                        spacing: 16
                        Label { text: qsTr("Consoles"); font.pixelSize: 24; font.bold: true }
                        C.Button {
                            id: registerNewButton
                            font.pixelSize: 14
                            objectName: "registerConsole"
                            text: qsTr("Register New")
                            Accessible.name: qsTr("Register a new console")
                            firstInFocusChain: true
                            onClicked: dialog.navigation.showRegistDialog("255.255.255.255", true)
                        }
                        Label { text: qsTr("Registered Consoles"); font.pixelSize: 14; font.bold: true }
                        Label {
                            font.pixelSize: 14
                            objectName: "registeredConsolesEmpty"
                            visible: dialog.backend.settings.registeredHosts.length === 0
                            text: qsTr("No registered consoles.")
                            Layout.fillWidth: true
                            wrapMode: Text.Wrap
                        }
                        Repeater {
                            model: dialog.backend.settings.registeredHosts
                            Frame {
                                font.pixelSize: 14
                                id: registeredCard
                                required property int index
                                required property var modelData
                                readonly property string displayName: dialog.backend.settings.streamerMode
                                    ? qsTr("Registered console %1").arg(index + 1) : modelData.name
                                objectName: "registeredConsole" + index
                                Layout.fillWidth: true
                                background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                                ColumnLayout {
                                    anchors.fill: parent
                                    spacing: 8
                                    Label {
                                        objectName: "registeredConsoleInfo" + registeredCard.index
                                        text: dialog.backend.settings.streamerMode
                                            ? "%1 (%2)".arg(registeredCard.displayName).arg(registeredCard.modelData.ps5 ? "PS5" : "PS4")
                                            : "%1 (%2, %3)".arg(registeredCard.modelData.mac).arg(registeredCard.modelData.ps5 ? "PS5" : "PS4").arg(registeredCard.displayName)
                                        textFormat: Text.PlainText
                                        wrapMode: Text.WrapAnywhere
                                        Layout.fillWidth: true
                                    }
                                    RowLayout {
                                        Layout.fillWidth: true
                                        C.CheckBox {
                                            id: autoConnectButton
                                            Layout.fillWidth: true
                                            Layout.minimumWidth: 0
                                            contentItem: Label { text: parent.text; textFormat: Text.PlainText; wrapMode: Text.Wrap; leftPadding: parent.indicator.width + parent.spacing; verticalAlignment: Text.AlignVCenter }
                                            KeyNavigation.right: deleteConsoleButton
                                            objectName: "autoConnectConsole" + registeredCard.index
                                            text: qsTr("Auto-Connect")
                                            Accessible.name: qsTr("Auto-connect to %1").arg(registeredCard.displayName)
                                            enabled: dialog.consoleIndex(dialog.backend.settings.registeredHosts, registeredCard.modelData.mac) >= 0
                                            checked: dialog.backend.settings.autoConnectMac === registeredCard.modelData.mac
                                            onToggled: {
                                                if (dialog.consoleIndex(dialog.backend.settings.registeredHosts, registeredCard.modelData.mac) >= 0)
                                                    dialog.backend.settings.autoConnectMac = checked ? registeredCard.modelData.mac : "";
                                            }
                                        }
                                        C.Button {
                                            id: deleteConsoleButton
                                            Layout.maximumWidth: registeredCard.width * 0.4
                                            contentItem: Label { text: parent.text; textFormat: Text.PlainText; wrapMode: Text.Wrap; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                                            KeyNavigation.left: autoConnectButton
                                            lastInFocusChain: registeredCard.index === dialog.backend.settings.registeredHosts.length - 1 && dialog.backend.hiddenHosts.length === 0
                                            objectName: "deleteRegisteredConsole" + registeredCard.index
                                            text: qsTr("Delete")
                                            Accessible.name: qsTr("Delete %1").arg(registeredCard.displayName)
                                            enabled: dialog.consoleIndex(dialog.backend.settings.registeredHosts, registeredCard.modelData.mac) >= 0
                                            onClicked: dialog.requestConsoleAction("delete", registeredCard.modelData.mac, this)
                                        }
                                    }
                                }
                            }
                        }
                        Label { text: qsTr("Hidden Consoles"); font.pixelSize: 14; font.bold: true }
                        Label {
                            font.pixelSize: 14
                            objectName: "hiddenConsolesEmpty"
                            visible: dialog.backend.hiddenHosts.length === 0
                            text: qsTr("No hidden consoles.")
                            Layout.fillWidth: true
                            wrapMode: Text.Wrap
                        }
                        Repeater {
                            model: dialog.backend.hiddenHosts
                            Frame {
                                font.pixelSize: 14
                                id: hiddenCard
                                required property int index
                                required property var modelData
                                readonly property string displayName: dialog.backend.settings.streamerMode
                                    ? qsTr("Hidden console %1").arg(index + 1) : modelData.name
                                objectName: "hiddenConsole" + index
                                Layout.fillWidth: true
                                background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                                ColumnLayout {
                                    anchors.fill: parent
                                    spacing: 8
                                    Label {
                                        objectName: "hiddenConsoleInfo" + hiddenCard.index
                                        text: dialog.backend.settings.streamerMode ? hiddenCard.displayName
                                            : "%1 (%2)".arg(hiddenCard.modelData.mac).arg(hiddenCard.displayName)
                                        textFormat: Text.PlainText
                                        wrapMode: Text.WrapAnywhere
                                        Layout.fillWidth: true
                                    }
                                    C.Button {
                                        objectName: "unhideConsole" + hiddenCard.index
                                        lastInFocusChain: hiddenCard.index === dialog.backend.hiddenHosts.length - 1
                                        text: qsTr("Unhide")
                                        Accessible.name: qsTr("Unhide %1").arg(hiddenCard.displayName)
                                        enabled: dialog.consoleIndex(dialog.backend.hiddenHosts, hiddenCard.modelData.mac) >= 0
                                        onClicked: dialog.requestConsoleAction("unhide", hiddenCard.modelData.mac, this)
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Item {
                // Keys
                Flickable {
                    id: keysFlick
                    objectName: "keysFlick"
                    anchors.fill: parent
                    clip: true
                    contentWidth: width
                    contentHeight: keysLayout.height
                    flickableDirection: Flickable.VerticalFlick
                    ScrollBar.vertical: ScrollBar {}
                    ColumnLayout {
                        id: keysLayout
                        width: Math.max(0, keysFlick.width - 14)
                        spacing: 16
                        Label { text: qsTr("Keyboard and mouse"); font.pixelSize: 24; font.bold: true }
                        C.CheckBox {
                            id: mouseCameraEnabled
                            objectName: "mouseCameraEnabled"
                            visible: dialog.backend.settings.mouseCameraSupported
                            text: qsTr("Enable mouse camera mode")
                            Accessible.name: text
                            Accessible.description: qsTr("During a stream, press F8 to capture or release your controls.")
                            checked: dialog.backend.settings.mouseCameraEnabled
                            onToggled: dialog.backend.settings.mouseCameraEnabled = checked
                        }
                        Label {
                            visible: dialog.backend.settings.mouseCameraSupported
                            Layout.fillWidth: true
                            text: qsTr("During a stream, press F8 to capture or release your controls.")
                            wrapMode: Text.WordWrap
                        }
                        Frame {
                            Layout.fillWidth: true
                            enabled: !dialog.backend.settings.mouseCameraEnabled
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            RowLayout {
                                anchors.fill: parent
                                C.CheckBox {
                                    objectName: "keyboardEnabled"
                                    text: qsTr("Keyboard as controller")
                                    Accessible.name: text
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    contentItem: Label { font.pixelSize: 14; text: parent.text; wrapMode: Text.Wrap; leftPadding: parent.indicator.width + parent.spacing; verticalAlignment: Text.AlignVCenter }
                                    checked: dialog.backend.settings.keyboardEnabled
                                    onToggled: dialog.backend.settings.keyboardEnabled = checked
                                }
                                C.CheckBox {
                                    objectName: "mouseTouchEnabled"
                                    text: qsTr("Enable Mouse Touchpad")
                                    Accessible.name: text
                                    Layout.fillWidth: true
                                    Layout.minimumWidth: 0
                                    contentItem: Label { font.pixelSize: 14; text: parent.text; wrapMode: Text.Wrap; leftPadding: parent.indicator.width + parent.spacing; verticalAlignment: Text.AlignVCenter }
                                    checked: dialog.backend.settings.mouseTouchEnabled
                                    onToggled: dialog.backend.settings.mouseTouchEnabled = checked
                                }
                            }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            enabled: !dialog.backend.settings.mouseCameraEnabled
                            Label { text: qsTr("Key assignments"); font.pixelSize: 14; font.bold: true; Layout.fillWidth: true }
                            C.Button {
                                font.pixelSize: 14
                                id: resetAllKeys
                                objectName: "resetAllKeys"
                                text: qsTr("Reset All Keys")
                                Accessible.name: text
                                onClicked: dialog.backend.settings.clearKeyMapping()
                            }
                        }
                        Frame {
                            Layout.fillWidth: true
                            enabled: !dialog.backend.settings.mouseCameraEnabled
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            GridLayout {
                                anchors.fill: parent
                                columns: keysFlick.width >= 900 ? 3 : 2
                                columnSpacing: 20
                                rowSpacing: 6
                                Repeater {
                                    id: chiakiKeys
                                    model: dialog.backend.settings.controllerMapping
                                    RowLayout {
                                        required property var modelData
                                        Layout.fillWidth: true
                                        spacing: 12
                                        Label {
                                            font.pixelSize: 14
                                            Layout.minimumWidth: 0
                                            Layout.fillWidth: true
                                            Layout.preferredWidth: 120
                                            text: modelData.buttonName
                                            textFormat: Text.PlainText
                                            wrapMode: Text.Wrap
                                        }
                                        C.Button {
                                            font.pixelSize: 14
                                            objectName: "keyMapping" + modelData.buttonValue
                                            property string capturedKeyName: ""
                                            Layout.preferredWidth: 120
                                            Layout.minimumWidth: 0
                                            text: capturedKeyName || modelData.keyName
                                            Accessible.name: qsTr("Key for %1").arg(modelData.buttonName)
                                            onClicked: keyDialog.show({
                                                value: modelData.buttonValue,
                                                opener: this,
                                                callback: name => capturedKeyName = name
                                            })
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
                    objectName: "controllersFlick"
                    anchors.fill: parent
                    anchors.margins: 16
                    clip: true
                    contentWidth: width
                    contentHeight: controllersLayout.height
                    flickableDirection: Flickable.VerticalFlick
                    ScrollBar.vertical: ScrollBar {}
                    ColumnLayout {
                        id: controllersLayout
                        width: Math.max(0, controllersFlick.width - 16)
                        spacing: 14
                        Label { text: qsTr("Controllers"); font.pixelSize: 24; font.bold: true }
                        Label { text: qsTr("Controller input"); font.pixelSize: 14; font.bold: true }
                        Frame {
                            Layout.fillWidth: true
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 12
                            RowLayout {
                                Layout.fillWidth: true
                                C.Button {
                                    id: controllerMappingChange
                                    objectName: "controllerMappingChange"
                                    firstInFocusChain: true
                                    text: qsTr("Change controller mapping")
                                    font.pixelSize: 14
                                    onClicked: controllerMappingDialog.show({ reset: false })
                                }
                                C.Button {
                                    id: controllerMappingReset
                                    objectName: "controllerMappingReset"
                                    text: qsTr("Reset controller mapping")
                                    font.pixelSize: 14
                                    onClicked: controllerMappingDialog.show({ reset: true })
                                }
                            }
                            C.CheckBox {
                                objectName: "allowJoystickBackgroundEvents"
                                text: qsTr("Process controller input in the background. Default: On")
                                font.pixelSize: 14
                                Layout.fillWidth: true
                                contentItem: Label { text: parent.text; font.pixelSize: 14; wrapMode: Text.Wrap; leftPadding: parent.indicator.width + parent.spacing; verticalAlignment: Text.AlignVCenter }
                                checked: dialog.backend.settings.allowJoystickBackgroundEvents
                                onToggled: dialog.backend.settings.allowJoystickBackgroundEvents = checked
                            }
                            C.CheckBox {
                                objectName: "buttonsByPosition"
                                text: qsTr("Interpret buttons by position. Default: Off")
                                font.pixelSize: 14
                                Layout.fillWidth: true
                                contentItem: Label { text: parent.text; font.pixelSize: 14; wrapMode: Text.Wrap; leftPadding: parent.indicator.width + parent.spacing; verticalAlignment: Text.AlignVCenter }
                                checked: dialog.backend.settings.buttonsByPosition
                                onToggled: dialog.backend.settings.buttonsByPosition = checked
                            }
                            }
                        }
                        Label { text: qsTr("Touchpad"); font.pixelSize: 14; font.bold: true }
                        Frame {
                            Layout.fillWidth: true
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 12
                            C.CheckBox {
                                objectName: "dpadTouchEnabled"
                                text: qsTr("D-pad touchpad emulation. Default: On")
                                font.pixelSize: 14
                                Layout.fillWidth: true
                                contentItem: Label { text: parent.text; font.pixelSize: 14; wrapMode: Text.Wrap; leftPadding: parent.indicator.width + parent.spacing; verticalAlignment: Text.AlignVCenter }
                                checked: dialog.backend.settings.dpadTouchEnabled
                                onToggled: dialog.backend.settings.dpadTouchEnabled = checked
                            }
                            ColumnLayout {
                                Layout.fillWidth: true
                                visible: dialog.backend.settings.dpadTouchEnabled
                                Label { text: qsTr("D-pad touch increment. Default: 0.3 mm"); font.pixelSize: 14 }
                                C.Slider {
                                    id: touchIncrement
                                    objectName: "dpadTouchIncrement"
                                    Accessible.name: qsTr("D-pad touch increment")
                                    Layout.fillWidth: true
                                    from: 1
                                    to: 1079
                                    stepSize: 1
                                    value: dialog.backend.settings.dpadTouchIncrement
                                    onMoved: dialog.backend.settings.dpadTouchIncrement = value
                                }
                                Label { objectName: "dpadTouchIncrementValue"; text: qsTr("%1 mm").arg(touchIncrement.value / 100); font.pixelSize: 14 }
                                Label { text: qsTr("D-pad regular/touch shortcut. Default: L1 + R1 + D-pad Up"); font.pixelSize: 14; wrapMode: Text.Wrap; Layout.fillWidth: true }
                                GridLayout {
                                    Layout.fillWidth: true
                                    columns: controllersFlick.width < 700 ? 2 : 4
                                    Repeater {
                                        model: 4
                                        C.ComboBox {
                                            required property int index
                                            objectName: "dpadTouchShortcut" + (index + 1)
                                            Accessible.name: qsTr("D-pad shortcut button %1").arg(index + 1)
                                            font.pixelSize: 14
                                            Layout.fillWidth: true
                                            Layout.minimumWidth: 0
                                            Layout.preferredWidth: 150
                                            model: [qsTr("Not Used"), qsTr("Cross"), qsTr("Moon"), qsTr("Box"), qsTr("Pyramid"), qsTr("Dpad Left"), qsTr("Dpad Right"), qsTr("Dpad Up"), qsTr("Dpad Down"), qsTr("L1"), qsTr("R1"), qsTr("L3"), qsTr("R3"), qsTr("Options"), qsTr("Share"), qsTr("Touchpad"), qsTr("PS")]
                                            currentIndex: dialog.backend.settings["dpadTouchShortcut" + (index + 1)]
                                            onActivated: value => dialog.backend.settings["dpadTouchShortcut" + (index + 1)] = value
                                        }
                                    }
                                }
                            }
                            }
                        }
                        Label { text: qsTr("Haptics"); font.pixelSize: 14; font.bold: true }
                        Frame {
                            Layout.fillWidth: true
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 12
                            Label { text: qsTr("Rumble haptics. Default: Normal"); font.pixelSize: 14 }
                            C.ComboBox {
                                objectName: "rumbleHapticsIntensity"
                                Accessible.name: qsTr("Rumble haptics intensity")
                                font.pixelSize: 14
                                Layout.fillWidth: true
                                Layout.minimumWidth: 0
                                model: [qsTr("Off"), qsTr("Very Weak"), qsTr("Weak"), qsTr("Normal"), qsTr("Strong"), qsTr("Very Strong")]
                                currentIndex: dialog.backend.settings.rumbleHapticsIntensity
                                onActivated: index => dialog.backend.settings.rumbleHapticsIntensity = index
                            }
                            Label { text: qsTr("True haptics intensity. Default: Console setting"); font.pixelSize: 14 }
                            C.Slider {
                                id: hapticOverride
                                objectName: "hapticOverride"
                                Accessible.name: qsTr("True haptics intensity")
                                Layout.fillWidth: true
                                from: 0
                                to: 2
                                stepSize: 0.1
                                value: dialog.backend.settings.hapticOverride
                                onMoved: dialog.backend.settings.hapticOverride = value
                                lastInFocusChain: true
                            }
                            Label {
                                objectName: "hapticOverrideValue"
                                font.pixelSize: 14
                                text: hapticOverride.value > 0.99 && hapticOverride.value < 1.01 ? qsTr("Console setting") : qsTr("%1% of console setting").arg((hapticOverride.value * 100).toFixed(0))
                            }
                            }
                        }
                    }
                }
            }

            Item {
                // Remote (PSN and Hole Punching)
                id: remotePage
                readonly property bool credentialsComplete: !!(dialog.backend.settings.psnRefreshToken
                    && dialog.backend.settings.psnAuthToken && dialog.backend.settings.psnAuthTokenExpiry
                    && dialog.backend.settings.psnAccountId)
                onCredentialsCompleteChanged: {
                    if (openPsnLogin.activeFocus || resetPsnTokens.activeFocus)
                        Qt.callLater(function() {
                            dialog.firstRemoteFocusableItem().forceActiveFocus(Qt.TabFocusReason);
                        });
                }
                Flickable {
                    id: remoteFlick
                    objectName: "remotePage"
                    anchors.fill: parent
                    clip: true
                    contentWidth: width
                    contentHeight: remoteLayout.height
                    flickableDirection: Flickable.VerticalFlick
                    ScrollBar.vertical: ScrollBar {}
                    ColumnLayout {
                        id: remoteLayout
                        width: Math.max(0, remoteFlick.width - 14)
                        spacing: 16
                        Label { text: qsTr("Remote"); font.pixelSize: 24; font.bold: true }
                        Label { text: qsTr("PSN access"); font.pixelSize: 14; font.bold: true }
                        Frame {
                            Layout.fillWidth: true
                            font.pixelSize: 14
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            ColumnLayout {
                                anchors.fill: parent
                                Label {
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                    text: remotePage.credentialsComplete ? qsTr("PSN account configured.") : qsTr("Log in to configure remote access through PSN.")
                                }
                                C.Button {
                                    id: openPsnLogin
                                    objectName: "openPsnLogin"
                                    firstInFocusChain: visible
                                    text: qsTr("Login to PSN")
                                    Accessible.name: text
                                    visible: !remotePage.credentialsComplete
                                    onClicked: dialog.navigation.showPSNTokenDialog(false)
                                }
                                C.Button {
                                    id: resetPsnTokens
                                    objectName: "resetPsnTokens"
                                    firstInFocusChain: visible
                                    text: qsTr("Clear PSN Token")
                                    Accessible.name: text
                                    visible: remotePage.credentialsComplete
                                    onClicked: {
                                        dialog.backend.settings.psnRefreshToken = "";
                                        dialog.backend.settings.psnAuthToken = "";
                                        dialog.backend.settings.psnAuthTokenExpiry = "";
                                        dialog.backend.settings.psnAccountId = "";
                                        openPsnLogin.forceActiveFocus(Qt.TabFocusReason);
                                    }
                                }
                            }
                        }
                        Label { text: qsTr("Port guessing"); font.pixelSize: 14; font.bold: true }
                        Frame {
                            Layout.fillWidth: true
                            font.pixelSize: 14
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            ColumnLayout {
                                anchors.fill: parent
                                spacing: 12
                                C.CheckBox {
                                    id: holePunchGuessingCheckbox
                                    objectName: "portGuessingEnabled"
                                    Layout.fillWidth: true
                                    contentItem: Label { text: parent.text; wrapMode: Text.Wrap; leftPadding: parent.indicator.width + parent.spacing; verticalAlignment: Text.AlignVCenter }
                                    text: qsTr("Force STUN port guessing")
                                    Accessible.name: text
                                    checked: dialog.backend.settings.portGuessingEnabled
                                    onToggled: dialog.backend.settings.portGuessingEnabled = checked
                                }
                                Label { text: qsTr("Default: Unchecked"); opacity: 0.65 }
                                Label { text: qsTr("Port Guess Count"); Layout.fillWidth: true; wrapMode: Text.Wrap }
                                C.Slider {
                                    id: portGuessCountSlider
                                    objectName: "portGuessCount"
                                    Accessible.name: qsTr("Port Guess Count")
                                    Accessible.description: qsTr("Guesses, default 75")
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 75
                                    stepSize: 1
                                    value: dialog.backend.settings.portGuessCount
                                    onMoved: dialog.backend.settings.portGuessCount = value
                                }
                                RowLayout {
                                    Layout.fillWidth: true
                                    Label {
                                        objectName: "portGuessCountValue"
                                        Layout.fillWidth: true
                                        wrapMode: Text.Wrap
                                        text: portGuessCountSlider.value + qsTr(" guesses")
                                    }
                                    Label { text: qsTr("Default: 75"); opacity: 0.65 }
                                }
                                Label { text: qsTr("Port Guess Socket Count"); Layout.fillWidth: true; wrapMode: Text.Wrap }
                                C.Slider {
                                    id: portGuessSocketSlider
                                    objectName: "portGuessSocketCount"
                                    Accessible.name: qsTr("Port Guess Socket Count")
                                    Accessible.description: qsTr("Sockets, default 250")
                                    Layout.fillWidth: true
                                    from: 0
                                    to: 500
                                    stepSize: 1
                                    value: dialog.backend.settings.portGuessSocketCount
                                    onMoved: dialog.backend.settings.portGuessSocketCount = value
                                    lastInFocusChain: true
                                }
                                RowLayout {
                                    Layout.fillWidth: true
                                    Label {
                                        objectName: "portGuessSocketCountValue"
                                        Layout.fillWidth: true
                                        wrapMode: Text.Wrap
                                        text: portGuessSocketSlider.value + qsTr(" sockets")
                                    }
                                    Label { text: qsTr("Default: 250"); opacity: 0.65 }
                                }
                            }
                        }
                    }
                }
            }

            Item {
                // Config (Options such as Import/Export and Logging)
                Flickable {
                    id: configFlick
                    objectName: "configFlick"
                    anchors.fill: parent
                    clip: true
                    contentWidth: width
                    contentHeight: configGrid.height + 32
                    flickableDirection: Flickable.VerticalFlick
                    ScrollBar.vertical: ScrollBar {}
                    ColumnLayout {
                        id: configGrid
                        x: 16; y: 16
                        width: Math.max(0, configFlick.width - 48)
                        spacing: 14
                        Label { text: qsTr("Config"); font.pixelSize: 24; font.bold: true }
                        Label { text: qsTr("Profiles"); font.pixelSize: 14; font.bold: true }
                        Frame {
                            Layout.fillWidth: true
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            RowLayout {
                                anchors.fill: parent
                                spacing: 24
                                Label {
                                    objectName: "configProfile"
                                    Layout.fillWidth: true
                                    wrapMode: Text.Wrap
                                    font.pixelSize: 14
                                    text: qsTr("Current profile: %1").arg(dialog.backend.settings.streamerMode ? qsTr("Hidden") : (dialog.backend.settings.currentProfile || qsTr("default")))
                                }
                                C.Button {
                                    id: profile
                                    objectName: "manageProfiles"
                                    text: qsTr("Manage profiles")
                                    Accessible.name: text
                                    firstInFocusChain: true
                                    font.pixelSize: 14
                                    onClicked: dialog.navigation.showProfileDialog()
                                    Material.roundedScale: Material.SmallScale
                                }
                            }
                        }
                        Label { text: qsTr("Settings files"); font.pixelSize: 14; font.bold: true }
                        Frame {
                            Layout.fillWidth: true
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            RowLayout {
                                anchors.fill: parent
                                spacing: 12
                                C.Button {
                                    id: exportButton
                                    objectName: "exportSettings"
                                    Layout.fillWidth: true
                                    text: qsTr("Export settings to file")
                                    Accessible.name: text
                                    font.pixelSize: 14
                                    onClicked: dialog.backend.settings.exportSettings()
                                    Material.roundedScale: Material.SmallScale
                                }
                                C.Button {
                                    id: importButton
                                    objectName: "importSettings"
                                    Layout.fillWidth: true
                                    text: qsTr("Import settings from file")
                                    Accessible.name: text
                                    font.pixelSize: 14
                                    onClicked: dialog.backend.settings.importSettings()
                                    Material.roundedScale: Material.SmallScale
                                }
                            }
                        }
                        Label { text: qsTr("Diagnostics"); font.pixelSize: 14; font.bold: true }
                        Frame {
                            Layout.fillWidth: true
                            background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                            ColumnLayout {
                                anchors.fill: parent
                                C.CheckBox {
                                    objectName: "logSanitize"
                                    text: qsTr("Sanitize logs · Default: On")
                                    Accessible.name: qsTr("Sanitize logs")
                                    font.pixelSize: 14
                                    checked: dialog.backend.settings.logSanitize
                                    onToggled: dialog.backend.settings.logSanitize = checked
                                }
                                C.CheckBox {
                                    objectName: "logVerbose"
                                    text: qsTr("Verbose logging · Default: Off")
                                    Accessible.name: qsTr("Verbose logging")
                                    font.pixelSize: 14
                                    checked: dialog.backend.settings.logVerbose
                                    onToggled: dialog.backend.settings.logVerbose = checked
                                }
                                C.Button {
                                    id: aboutButton
                                    objectName: "aboutSettings"
                                    text: qsTr("About Chiaki Recast")
                                    Accessible.name: text
                                    font.pixelSize: 14
                                    lastInFocusChain: true
                                    onClicked: { aboutDialog.restoreFocusItem = aboutButton; aboutDialog.open(); }
                                    Material.roundedScale: Material.SmallScale
                                }
                            }
                        }
                    }
                }
            }
        }

        ListView {
            id: resultList
            objectName: "settingsSearchResults"
            anchors { top: searchRow.bottom; left: sidebar.right; right: parent.right; bottom: parent.bottom; leftMargin: 24; topMargin: 20 }
            visible: dialog.searchQuery.length > 0
            clip: true
            spacing: 8
            model: dialog.searchResults
            property bool navigating: false
            currentIndex: -1
            onModelChanged: {
                currentIndex = -1;
                if (navigating && dialog.searchQuery.length)
                    Qt.callLater(() => searchField.forceActiveFocus(Qt.TabFocusReason));
            }
            ScrollBar.vertical: ScrollBar {}
            function navigate(event) {
                if (event.key === Qt.Key_Up) {
                    if (currentIndex <= 0) searchField.forceActiveFocus();
                    else decrementCurrentIndex();
                } else if (event.key === Qt.Key_Down) incrementCurrentIndex();
                else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) dialog.openSearchResult(currentIndex);
                else if (event.key === Qt.Key_Escape) { searchField.clear(); searchField.forceActiveFocus(); }
                else return;
                if ((event.key === Qt.Key_Down || event.key === Qt.Key_Up) && !searchField.activeFocus && currentItem)
                    currentItem.forceActiveFocus(Qt.TabFocusReason);
                event.accepted = true;
            }
            Keys.onPressed: event => navigate(event)
            delegate: C.Button {
                required property int index
                required property var modelData
                objectName: "settingsSearchResult" + index
                width: resultList.width - 14
                height: resultText.implicitHeight + 24
                highlighted: resultList.currentIndex === index
                firstInFocusChain: true
                lastInFocusChain: true
                background: Rectangle {
                    radius: 8
                    color: parent.highlighted ? "#24394c" : parent.hovered ? "#1b2633" : "#1a202b"
                    border.color: parent.activeFocus ? Material.accent : "#303a48"
                    border.width: parent.activeFocus ? 2 : 1
                }
                Accessible.name: modelData.title
                Accessible.description: dialog.searchContext(modelData) + ". " + modelData.description
                onActiveFocusChanged: if (activeFocus) { resultList.currentIndex = index; resultList.navigating = true; }
                Keys.onPressed: event => {
                    if (event.key === Qt.Key_Up || event.key === Qt.Key_Down || event.key === Qt.Key_Escape)
                        resultList.navigate(event);
                    else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                        dialog.openSearchResult(index);
                        event.accepted = true;
                    }
                }
                onClicked: dialog.openSearchResult(index)
                contentItem: Column {
                    id: resultText
                    spacing: 4
                    Label { text: modelData.title; textFormat: Text.PlainText; font.pixelSize: 16; font.bold: true; width: parent.width; wrapMode: Text.Wrap }
                    Label { text: dialog.searchContext(modelData); textFormat: Text.PlainText; font.pixelSize: 14; color: "#aeb9c8"; width: parent.width; wrapMode: Text.Wrap }
                    Label { text: modelData.description; textFormat: Text.PlainText; font.pixelSize: 14; width: parent.width; wrapMode: Text.Wrap }
                }
            }
            Label {
                objectName: "settingsSearchEmpty"
                visible: resultList.count === 0
                width: parent.width
                wrapMode: Text.Wrap
                text: qsTr("No settings found. Try a setting name or description.")
                Accessible.name: text
            }
        }

        Connections {
            target: dialog.Window.window
            function onActiveFocusItemChanged() {
                dialog.ensureActiveFocusVisible()
            }
        }
        Connections {
            target: dialog.activeSettingsFlick()
            function onContentHeightChanged() { Qt.callLater(dialog.ensureActiveFocusVisible); }
            function onHeightChanged() { Qt.callLater(dialog.ensureActiveFocusVisible); }
        }

        Item {
            Timer {
                id: openTimer
                interval: 100
                running: false
                onTriggered: {
                    if (!controllerMappingDialog.visible || !dialog.backend.controllerMappingInProgress) return;
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
                        const route = dialog.pendingMappingSearch;
                        dialog.pendingMappingSearch = null;
                        if (route) dialog.navigation.showControllerMappingDialog(route);
                        else dialog.navigation.showControllerMappingDialog();
                        quitControllerMapping = false;
                        controllerMappingDialog.resetFocus = false;
                        controllerMappingDialog.close();
                    }
                }
            }
        }

        ConfirmDialog {
            id: aboutDialog
            objectName: "aboutDialog"
            title: qsTr("About Chiaki Recast")
            cancelText: qsTr("Close")
            imageSource: "qrc:icons/chiaking-logo.svg"
            textFormat: Text.RichText
            navigation: dialog.navigation
            text: "<h1>Chiaki Recast</h1> version %1
                        <h2>Fork of chiaki-ng</h2> by Street Pea and contributors
                        <h2>Fork of Chiaki</h2> by Florian Markl at version 2.1.1

                        <p>This program is free software: you can redistribute it and/or modify
                        it under the terms of the GNU Affero General Public License version 3
                        as published by the Free Software Foundation.</p>

                        <p>This program is distributed in the hope that it will be useful,
                        but WITHOUT ANY WARRANTY; without even the implied warranty of
                        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
                        GNU General Public License for more details.</p>".arg(Qt.application.version)
        }

        Dialog {
            id: keyDialog
            objectName: "keyCaptureDialog"
            focus: false
            property int buttonValue
            property var buttonCallback
            property Item returnFocusItem
            property bool captureActive: false
            property int pressedKey: -1
            parent: Overlay.overlay
            x: Math.round((dialog.width - width) / 2)
            y: Math.round((dialog.height - height) / 2)
            title: qsTr("Key Capture")
            modal: true
            standardButtons: Dialog.Close
            closePolicy: Popup.CloseOnPressOutside
            onOpened: keyLabel.forceActiveFocus(Qt.TabFocusReason)
            onAboutToHide: captureActive = false
            onClosed: {
                keyLabel.focus = false;
                focus = false;
                Qt.callLater(() => {
                    const item = returnFocusItem && returnFocusItem.visible && returnFocusItem.enabled
                        ? returnFocusItem : resetAllKeys;
                    if (item) item.forceActiveFocus(Qt.TabFocusReason);
                });
            }
            Material.roundedScale: Material.MediumScale

            function show(opts) {
                buttonValue = opts.value;
                buttonCallback = opts.callback;
                returnFocusItem = opts.opener;
                pressedKey = -1;
                captureActive = true;
                open();
            }

            Connections {
                target: dialog.backend.settings
                function onCurrentProfileChanged() { keyDialog.close(); }
                function onControllerMappingChanged() { keyDialog.close(); }
            }
            Label {
                id: keyLabel
                objectName: "keyCapturePrompt"
                focus: true
                text: qsTr("Press a key to assign it. Press Escape or Close to cancel.")
                Accessible.name: text
                Keys.onPressed: (event) => {
                    event.accepted = true;
                    if (keyDialog.captureActive && !event.isAutoRepeat)
                        keyDialog.pressedKey = event.key;
                }
                Keys.onReleased: (event) => {
                    event.accepted = true;
                    if (!keyDialog.captureActive || event.isAutoRepeat || keyDialog.pressedKey !== event.key)
                        return;
                    keyDialog.pressedKey = -1;
                    if (event.key !== Qt.Key_Escape) {
                        const name = dialog.backend.settings.changeControllerKey(keyDialog.buttonValue, event.key);
                        keyDialog.buttonCallback(name);
                    }
                    keyDialog.close();
                }
            }
        }

        Dialog {
            id: controllerMappingDialog
            objectName: "controllerCapture"
            width: 600
            onAboutToHide: openTimer.stop()
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
                dialog.pendingMappingSearch = null;
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
            contentItem: Label {
                id: controllerLabel
                wrapMode: Text.Wrap
                text: qsTr("Choose the controller by pressing any button on the controller")
                Keys.onPressed: event => {
                    if (event.key === Qt.Key_Escape) controllerMappingDialog.close();
                    event.accepted = true;
                }
                Keys.onReleased: event => event.accepted = true
            }
        }

        Dialog {
            id: steamControllerMappingDialog
            objectName: "steamControllerNotice"
            width: 600
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
                focus = false;
                const opener = resetMapping ? controllerMappingReset : controllerMappingChange;
                if (dialog.visible && opener.visible && opener.enabled) {
                    opener.focus = false;
                    opener.forceActiveFocus(Qt.TabFocusReason);
                }
            }
            Material.roundedScale: Material.MediumScale

            contentItem: Label {
                id: steamLabel
                wrapMode: TextEdit.Wrap
                text: qsTr("This controller is managed by Steam.\nPlease use Steam to map controller or disable Steam Input for the controller before mapping here.")
                Keys.onReturnPressed: steamControllerMappingDialog.close();
                Keys.onEscapePressed: steamControllerMappingDialog.close();
            }
        }

        Connections {
            target: dialog.backend.settings
            function onCurrentProfileChanged() {
                dialog.pendingMappingSearch = null;
                if (controllerMappingDialog.visible) controllerMappingDialog.close();
                if (steamControllerMappingDialog.visible) steamControllerMappingDialog.close();
            }
        }

        Connections {
            target: dialog.backend

            function onControllerMappingInProgressChanged()
            {
                if (dialog.backend.controllerMappingInProgress && controllerMappingDialog.visible)
                    openTimer.start();
                else
                {
                    openTimer.stop();
                    dialog.pendingMappingSearch = null;
                }
            }

            function onControllerMappingSteamControllerSelected()
            {
                if (!controllerMappingDialog.visible) return;
                dialog.pendingMappingSearch = null;
                controllerMappingDialog.resetFocus = false;
                quitControllerMapping = false;
                steamControllerMappingDialog.resetMapping = controllerMappingDialog.resetMapping;
                controllerMappingDialog.close();
                steamControllerMappingDialog.open();
            }
        }
    }
}
