import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import org.streetpea.chiaking

Item {
    id: root
    required property var nativeWindow
    property string settingsPresentation: ""
    readonly property bool preferSeparateStreamSettingsWindows: root.nativeWindow.runtimeRendererBackend === 1 && Chiaki.session
    property bool useSeparateStreamSettingsWindows: false
    readonly property bool streamSettingsOpen: displaySettingsRect.opacity > 0 || placeboSettingsRect.opacity > 0 || colorMappingSettingsRect.opacity > 0 || keyboardMouseSettingsRect.opacity > 0
    Material.theme: Material.Dark
    Material.accent: "#00a7ff"

    // Render control keeps its Quick window hidden, so its overlay needs the viewport size.
    Binding { target: root.Overlay.overlay; property: "width"; value: root.width }
    Binding { target: root.Overlay.overlay; property: "height"; value: root.height }

    function controllerButton(name) {
        let type = "deck";
        for (let i = 0; i < Chiaki.controllers.length; ++i) {
            if (Chiaki.controllers[i].playStation) {
                type = "ps";
                break;
            }
        }
        return "image://svg/button-%1#%2".arg(type).arg(name);
    }
    function presentSettings(loader, role) {
        settingsPresentation = root.nativeWindow.streamInput.present(loader, role, loader);
    }
    function dismissSettings() {
        const id = settingsPresentation;
        root.nativeWindow.streamInput.dismiss(id);
        root.nativeWindow.streamInput.settled(id);
        settingsPresentation = "";
    }

    function openDisplaySettings() {
        if (livePinHost.blocking) return;
        useSeparateStreamSettingsWindows = preferSeparateStreamSettingsWindows;
        const loader = useSeparateStreamSettingsWindows ? separateDisplaySettingsLoader : displaySettingsLoader;
        if(loader.status == Loader.Ready)
        {
            if(root.streamSettingsOpen)
                closeDialog();
            displaySettingsRect.opacity = 0.8;
            presentSettings(loader, "display");
            if (!loader.item.restoreFocusItem) {
                let item = loader.item.mainItem.nextItemInFocusChain();
                if (item)
                    item.forceActiveFocus(Qt.TabFocusReason);
            } else {
                loader.item.restoreFocusItem.forceActiveFocus(Qt.TabFocusReason);
                loader.item.restoreFocusItem = null;
            }
        }
    }
    function openPlaceboSettings() {
        if (livePinHost.blocking) return;
        if (!root.streamSettingsOpen)
            useSeparateStreamSettingsWindows = preferSeparateStreamSettingsWindows;
        const loader = useSeparateStreamSettingsWindows ? separatePlaceboSettingsLoader : placeboSettingsLoader;
        if(loader.status == Loader.Ready)
        {
            if(root.streamSettingsOpen)
                closeDialog();
            placeboSettingsRect.opacity = 0.8;
            presentSettings(loader, "renderer");
            if (!loader.item.restoreFocusItem) {
                let item = loader.item.mainItem.nextItemInFocusChain();
                if (item)
                    item.forceActiveFocus(Qt.TabFocusReason);
            } else {
                loader.item.restoreFocusItem.forceActiveFocus(Qt.TabFocusReason);
                loader.item.restoreFocusItem = null;
            }
        }
    }

    function openKeyboardMouseSettings() {
        if (livePinHost.blocking || !Chiaki.settings.mouseCameraSupported) return;
        if (root.streamSettingsOpen) closeDialog();
        useSeparateStreamSettingsWindows = preferSeparateStreamSettingsWindows;
        const loader = useSeparateStreamSettingsWindows ? separateKeyboardMouseSettingsLoader : keyboardMouseSettingsLoader;
        if (loader.status !== Loader.Ready) return;
        keyboardMouseSettingsRect.opacity = 0.8;
        presentSettings(loader, "keyboard-mouse");
        const item = loader.item.restoreFocusItem || loader.item.mainItem.nextItemInFocusChain();
        if (item) item.forceActiveFocus(Qt.TabFocusReason);
        loader.item.restoreFocusItem = null;
    }

    function closeDialog() {
       if(keyboardMouseSettingsRect.opacity) {
        const loader = useSeparateStreamSettingsWindows ? separateKeyboardMouseSettingsLoader : keyboardMouseSettingsLoader;
        loader.item.restoreFocusItem = useSeparateStreamSettingsWindows ? separateKeyboardMouseSettingsWindow.activeFocusItem : Window.window.activeFocusItem;
        keyboardMouseSettingsRect.opacity = 0.0;
        dismissSettings();
        useSeparateStreamSettingsWindows = false;
       }
       else if(displaySettingsRect.opacity) {
        displaySettingsRect.opacity = 0.0;
        const loader = useSeparateStreamSettingsWindows ? separateDisplaySettingsLoader : displaySettingsLoader;
        loader.item.restoreFocusItem = Window.window.activeFocusItem;
        dismissSettings();
        useSeparateStreamSettingsWindows = false;
       }
       else if(placeboSettingsRect.opacity) {
        placeboSettingsRect.opacity = 0.0;
        const loader = useSeparateStreamSettingsWindows ? separatePlaceboSettingsLoader : placeboSettingsLoader;
        loader.item.restoreFocusItem = Window.window.activeFocusItem;
        dismissSettings();
        useSeparateStreamSettingsWindows = false;
       }
       else if(colorMappingSettingsRect.opacity) {
        dismissSettings();
        colorMappingSettingsRect.opacity = 0.0;
        const loader = useSeparateStreamSettingsWindows ? separateColorMappingSettingsLoader : colorMappingSettingsLoader;
        loader.item.restoreFocusItem = Window.window.activeFocusItem;
        root.openPlaceboSettings();
       }
    }

    function showMainView() {
        if (keyboardMouseSettingsRect.opacity) closeDialog();
        if(displaySettingsRect.opacity) {
            displaySettingsRect.opacity = 0.0;
            const loader = useSeparateStreamSettingsWindows ? separateDisplaySettingsLoader : displaySettingsLoader;
            loader.item.restoreFocusItem = Window.window.activeFocusItem;
            dismissSettings();
            useSeparateStreamSettingsWindows = false;
        }
        else if(placeboSettingsRect.opacity) {
            placeboSettingsRect.opacity = 0.0;
            const loader = useSeparateStreamSettingsWindows ? separatePlaceboSettingsLoader : placeboSettingsLoader;
            loader.item.restoreFocusItem = Window.window.activeFocusItem;
            dismissSettings();
            useSeparateStreamSettingsWindows = false;
        }
        else if(colorMappingSettingsRect.opacity) {
            colorMappingSettingsRect.opacity = 0.0;
            const loader = useSeparateStreamSettingsWindows ? separateColorMappingSettingsLoader : colorMappingSettingsLoader;
            loader.item.restoreFocusItem = Window.window.activeFocusItem;
            dismissSettings();
            useSeparateStreamSettingsWindows = false;
        }
        root.nativeWindow.dismissStreamView();
    }

    function restoreMainFocus() {
        const view = stack.currentItem;
        if (!view) return;
        let item = view.nextItemInFocusChain();
        for (let count = 0; item && item !== view && count < 256; ++count) {
            let owner = item;
            while (owner && owner !== view) owner = owner.parent;
            if (owner && item.visible && item.enabled && item.activeFocusOnTab) {
                item.forceActiveFocus(Qt.TabFocusReason);
                return;
            }
            item = item.nextItemInFocusChain();
        }
    }

    function showRendererFallbackDialog(reason) {
        infoDialog.reason = reason;
        infoDialog.restoreFocusItem = Window.window.activeFocusItem;
        infoDialog.open();
    }

    function showPlaceboColorMappingDialog(route) {
        if (livePinHost.blocking) return;
        if(placeboSettingsRect.opacity)
        {
            root.closeDialog();
            useSeparateStreamSettingsWindows = preferSeparateStreamSettingsWindows;
            const loader = useSeparateStreamSettingsWindows ? separateColorMappingSettingsLoader : colorMappingSettingsLoader;
            if(loader.status == Loader.Ready)
            {
                presentSettings(loader, "color-mapping");
                colorMappingSettingsRect.opacity = 0.8;
                if (!loader.item.restoreFocusItem) {
                    let item = loader.item.mainItem.nextItemInFocusChain();
                    if (item)
                        item.forceActiveFocus(Qt.TabFocusReason);
                } else {
                    loader.item.restoreFocusItem.forceActiveFocus(Qt.TabFocusReason);
                    loader.item.restoreFocusItem = null;
                }
                if (route) loader.item.revealSetting(route);
            }
        }
    }

    Pane {
        anchors.fill: parent
        visible: !root.nativeWindow.hasVideo && !root.nativeWindow.keepVideo
    }

    StackView {
        id: stack
        enabled: !livePinHost.blocking
        objectName: "streamNavigation"
        anchors.fill: parent
        hoverEnabled: false
        initialItem: streamViewComponent
        font.pixelSize: 20

        replaceEnter: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0.01
                to: 1.0
                duration: 200
            }
        }

        replaceExit: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 1.0
                to: 0.0
                duration: 200
            }
        }
    }

    LiveConsolePinHost {
        id: livePinHost
        hostWindow: root.nativeWindow
        navigation: root
        anchors.fill: parent
    }

    Rectangle {
        id: placeboSettingsRect
        opacity: 0.0
        visible: opacity && !useSeparateStreamSettingsWindows
        height: 650
        width: 1200
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        color: Material.background
        Loader {
            anchors.fill: parent
            id: placeboSettingsLoader
            sourceComponent: placeboSettingsDialogComponent
        }
    }
    Rectangle {
        id: displaySettingsRect
        opacity: 0.0
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        height: 500
        width: 1200
        visible: opacity && !useSeparateStreamSettingsWindows
        color: Material.background
        Loader {
            anchors.fill: parent
            id: displaySettingsLoader
            sourceComponent: displaySettingsDialogComponent
        }
    }
    Rectangle {
        id: colorMappingSettingsRect
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        opacity: 0.0
        height: 600
        width: 1200
        visible: opacity && !useSeparateStreamSettingsWindows
        color: Material.background
        Loader {
            anchors.fill: parent
            id: colorMappingSettingsLoader
            sourceComponent: placeboColorMappingDialogComponent
        }
    }

    Rectangle {
        id: keyboardMouseSettingsRect
        opacity: 0.0
        visible: opacity > 0 && !useSeparateStreamSettingsWindows
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(900, root.width)
        height: Math.min(700, root.height)
        color: Material.background
        Loader {
            id: keyboardMouseSettingsLoader
            anchors.fill: parent
            sourceComponent: keyboardMouseSettingsDialogComponent
        }
    }
    Window {
        id: separateKeyboardMouseSettingsWindow
        objectName: "keyboardMouseSettingsWindow"
        visible: useSeparateStreamSettingsWindows && keyboardMouseSettingsRect.opacity > 0
        transientParent: root.nativeWindow
        flags: Qt.Dialog | Qt.FramelessWindowHint
        color: Material.background
        modality: Qt.NonModal
        x: Math.round(root.mapToGlobal(0, 0).x + (root.width - width) / 2)
        y: Math.round(root.mapToGlobal(0, 0).y)
        width: Math.min(900, Math.round(root.width))
        height: Math.min(700, Math.round(root.height))
        onVisibleChanged: if (visible) requestActivate()
        onClosing: event => { event.accepted = false; root.closeDialog(); }
        Shortcut { sequence: "Ctrl+O"; onActivated: root.closeDialog() }
        Shortcut { sequences: [StandardKey.Cancel]; onActivated: root.closeDialog() }
        Loader {
            id: separateKeyboardMouseSettingsLoader
            anchors.fill: parent
            sourceComponent: keyboardMouseSettingsDialogComponent
        }
    }

    Window {
        id: separateDisplaySettingsWindow
        visible: useSeparateStreamSettingsWindows && displaySettingsRect.opacity > 0.0
        transientParent: root.nativeWindow
        flags: Qt.Dialog | Qt.FramelessWindowHint
        color: Material.background
        modality: Qt.NonModal
        x: Math.round(root.mapToGlobal(0, 0).x)
        y: Math.round(root.mapToGlobal(0, 0).y)
        width: Math.round(root.width)
        height: separateDisplaySettingsLoader.item ? Math.min(Math.round(root.height), Math.round(separateDisplaySettingsLoader.item.gridHeight + 140)) : 500
        onVisibleChanged: if (visible) requestActivate()

        Shortcut {
            sequence: "Ctrl+O"
            onActivated: root.closeDialog()
        }

        Shortcut {
            sequence: StandardKey.Cancel
            onActivated: root.closeDialog()
        }

        Loader {
            anchors.fill: parent
            id: separateDisplaySettingsLoader
            sourceComponent: displaySettingsDialogComponent
        }
    }

    Window {
        id: separatePlaceboSettingsWindow
        visible: useSeparateStreamSettingsWindows && placeboSettingsRect.opacity > 0.0
        transientParent: root.nativeWindow
        flags: Qt.Dialog | Qt.FramelessWindowHint
        color: Material.background
        modality: Qt.NonModal
        x: Math.round(root.mapToGlobal(0, 0).x)
        y: Math.round(root.mapToGlobal(0, 0).y)
        width: Math.round(root.width)
        height: Math.min(Math.round(root.height), 650)
        onVisibleChanged: if (visible) requestActivate()

        Shortcut {
            sequence: "Ctrl+O"
            onActivated: root.closeDialog()
        }

        Shortcut {
            sequence: StandardKey.Cancel
            onActivated: root.closeDialog()
        }

        Loader {
            anchors.fill: parent
            id: separatePlaceboSettingsLoader
            sourceComponent: placeboSettingsDialogComponent
        }
    }

    Window {
        id: separateColorMappingSettingsWindow
        visible: useSeparateStreamSettingsWindows && colorMappingSettingsRect.opacity > 0.0
        transientParent: root.nativeWindow
        flags: Qt.Dialog | Qt.FramelessWindowHint
        color: Material.background
        modality: Qt.NonModal
        x: Math.round(root.mapToGlobal(0, 0).x)
        y: Math.round(root.mapToGlobal(0, 0).y)
        width: Math.round(root.width)
        height: Math.min(Math.round(root.height), 600)
        onVisibleChanged: if (visible) requestActivate()

        Shortcut {
            sequence: "Ctrl+O"
            onActivated: root.closeDialog()
        }

        Shortcut {
            sequence: StandardKey.Cancel
            onActivated: root.closeDialog()
        }

        Loader {
            anchors.fill: parent
            id: separateColorMappingSettingsLoader
            sourceComponent: placeboColorMappingDialogComponent
        }
    }
    Rectangle {
        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
            bottomMargin: 30
        }
        color: Material.accent
        width: errorLayout.width + 40
        height: errorLayout.height + 20
        radius: 8
        opacity: errorHideTimer.running ? 0.8 : 0.0

        Behavior on opacity { NumberAnimation { duration: 500 } }

        ColumnLayout {
            id: errorLayout
            anchors.centerIn: parent

            Label {
                id: errorTitleLabel
                Layout.alignment: Qt.AlignCenter
                font.bold: true
                font.pixelSize: 24
            }

            Label {
                id: errorTextLabel
                Layout.alignment: Qt.AlignCenter
                horizontalAlignment: Text.AlignHCenter
                font.pixelSize: 20
            }
        }

        Timer {
            id: errorHideTimer
            interval: 2000
        }
    }

    RendererFallbackDialog {
        id: infoDialog
        property string presentation: ""
        navigation: root
        onAboutToShow: presentation = root.nativeWindow.streamInput.present(infoDialog, "renderer-fallback", null)
        onAboutToHide: {
            const id = presentation;
            root.nativeWindow.streamInput.dismiss(id);
            const complete = function() {
                infoDialog.closed.disconnect(complete);
                root.nativeWindow.streamInput.settled(id);
                if (infoDialog.presentation === id) infoDialog.presentation = "";
            };
            infoDialog.closed.connect(complete);
        }
    }

    Connections {
        target: Chiaki
        function onError(title, text) {
            const attempt = Chiaki.connectionAttempt;
            if (attempt.presented && attempt.errorTitle === title && attempt.errorText === text) return;
            errorTitleLabel.text = title;
            errorTextLabel.text = text;
            errorHideTimer.start();
        }
    }

    Component {
        id: streamViewComponent
        StreamView { streamWindow: root.nativeWindow; navigation: root }
    }

    Component {
        id: keyboardMouseSettingsDialogComponent
        KeyboardMouseSettingsDialog { }
    }

    Component {
        id: displaySettingsDialogComponent
        DisplaySettingsDialog { }
    }

    Component {
        id: placeboSettingsDialogComponent
        PlaceboSettingsDialog { }
    }

    Component {
        id: placeboColorMappingDialogComponent
        PlaceboColorMappingDialog { }
    }

}
