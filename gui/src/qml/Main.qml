import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material

import org.streetpea.chiaking

Item {
    id: root
    property string settingsPresentation: ""
    property bool initialAsk: false
    property var librarySelection: null
    readonly property bool preferSeparateStreamSettingsWindows: Chiaki.window.runtimeRendererBackend === 1 && Chiaki.session
    property bool useSeparateStreamSettingsWindows: false
    readonly property bool streamSettingsOpen: displaySettingsRect.opacity > 0 || placeboSettingsRect.opacity > 0 || colorMappingSettingsRect.opacity > 0
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
        settingsPresentation = Chiaki.window.streamInput.present(loader, role, loader);
    }
    function dismissSettings() {
        const id = settingsPresentation;
        Chiaki.window.streamInput.dismiss(id);
        Chiaki.window.streamInput.settled(id);
        settingsPresentation = "";
    }

    function openDisplaySettings() {
        if (livePinHost.blocking) return;
        useSeparateStreamSettingsWindows = preferSeparateStreamSettingsWindows;
        const loader = useSeparateStreamSettingsWindows ? separateDisplaySettingsLoader : displaySettingsLoader;
        if(loader.status == Loader.Ready)
        {
            if(placeboSettingsRect.opacity || displaySettingsRect.opacity || colorMappingSettingsRect.opacity)
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
        if (!displaySettingsRect.opacity && !placeboSettingsRect.opacity && !colorMappingSettingsRect.opacity)
            useSeparateStreamSettingsWindows = preferSeparateStreamSettingsWindows;
        const loader = useSeparateStreamSettingsWindows ? separatePlaceboSettingsLoader : placeboSettingsLoader;
        if(loader.status == Loader.Ready)
        {
            if(placeboSettingsRect.opacity || displaySettingsRect.opacity || colorMappingSettingsRect.opacity)
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

    function closeDialog() {
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
        dismissSettings();
        colorMappingSettingsRect.opacity = 0.0;
        const loader = useSeparateStreamSettingsWindows ? separateColorMappingSettingsLoader : colorMappingSettingsLoader;
        loader.item.restoreFocusItem = Window.window.activeFocusItem;
        root.openPlaceboSettings();
       }
       else
        stack.pop();
    }

    function showMainView() {
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
        if (stack.depth > 1)
            stack.pop(stack.get(0));
        else
            stack.replace(stack.get(0), mainViewComponent);
    }

    function showStreamView() {
        if (stack.currentItem instanceof StreamView) return;
        stack.replace(stack.get(0), streamViewComponent);
    }

    function showPsnView() {
        if (stack.currentItem instanceof PsnView || stack.currentItem instanceof StreamView || stack.currentItem instanceof PSNTokenDialog) return;
        stack.replace(stack.get(0), psnViewComponent, {}, StackView.Immediate)
    }

    function showManualHostDialog() {
        stack.push(manualHostDialogComponent);
    }

    function rememberMainFocus() {
        if (stack.currentItem instanceof MainView) stack.currentItem.rememberFocus();
    }

    function restoreMainFocus() {
        const view = stack.currentItem;
        if (view instanceof MainView) { view.restoreFocus(); return; }
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

    function showConfirmDialog(title, text, actionText, callback, rejectCallback = null) {
        rememberMainFocus();
        confirmDialog.title = title;
        confirmDialog.text = text;
        confirmDialog.actionText = actionText;
        confirmDialog.callback = callback;
        confirmDialog.rejectCallback = rejectCallback;
        confirmDialog.restoreFocusItem = Window.window.activeFocusItem;
        confirmDialog.open();
    }

    function showRendererFallbackDialog(reason) {
        rememberMainFocus();
        infoDialog.reason = reason;
        infoDialog.restoreFocusItem = Window.window.activeFocusItem;
        infoDialog.open();
    }

    function showPsnReminder() { remindDialog.offerPsnReminder(); }

    function showRemindDialog(title, text, remotePlay, callback) {
        if (remindDialog.visible) return false;
        rememberMainFocus();
        return remindDialog.showReminder(title, text, remotePlay, callback, Window.window.activeFocusItem);
    }


    function showRegistDialog(host, ps5, context) {
        stack.push(registDialogComponent, {host: host, ps5: ps5,
            registrationContext: context || Chiaki.createRegistrationContext()});
    }

    function showSettingsDialog() {
        if (livePinHost.blocking) return;
        stack.push(settingsDialogComponent);
    }

    function showDisplaySettingsDialog(route) {
        if (route) stack.push(displaySettingsDialogComponent, {searchRoute: route});
        else stack.push(displaySettingsDialogComponent);
    }

    function showPlaceboSettingsDialog(route) {
        if (route) stack.push(placeboSettingsDialogComponent, {searchRoute: route});
        else stack.push(placeboSettingsDialogComponent);
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
        else if (route)
            stack.push(placeboColorMappingDialogComponent, {searchRoute: route});
        else
            stack.push(placeboColorMappingDialogComponent);
    }

    function showProfileDialog(route) {
        if (route) stack.push(profileDialogComponent, {searchRoute: route});
        else stack.push(profileDialogComponent)
    }

    function showConsolePinDialog(submitPin, cancelPin, consoleName) {
        stack.push(consolePinDialogComponent, {submitPin, cancelPin, consoleName: Qt.binding(consoleName)});
    }

    function showSteamShortcutDialog(fromReminder) {
        stack.push(steamShortcutDialogComponent, {fromReminder: fromReminder});
    }

    function showPSNTokenDialog(psnurl, expired) {
        stack.push(psnTokenDialogComponent, {psnurl: psnurl || "", expired: !!expired});
    }

    function showControllerMappingDialog(route) {
        if (route) stack.push(controllerMappingDialogComponent, {searchRoute: route});
        else stack.push(controllerMappingDialogComponent)
    }

    Component.onCompleted: {
        if (Chiaki.connectionAttempt.presented && Chiaki.connectionAttempt.waitingForWake)
            stack.replace(stack.get(0), autoConnectViewComponent, {}, StackView.Immediate);
        else if (Chiaki.connectionAttempt.presented && Chiaki.connectionAttempt.route === "psn"
                && Chiaki.connectionAttempt.phase !== "video" && Chiaki.connectionAttempt.phase !== "ready")
            stack.replace(stack.get(0), psnViewComponent, {}, StackView.Immediate);
        else if (Chiaki.session || Chiaki.connectionAttempt.presented)
            stack.replace(stack.get(0), streamViewComponent, {}, StackView.Immediate);
        else if (Chiaki.autoConnect)
            stack.replace(stack.get(0), autoConnectViewComponent, {}, StackView.Immediate);
    }

    Pane {
        anchors.fill: parent
        visible: !Chiaki.window.hasVideo && !Chiaki.window.keepVideo
    }

    StackView {
        id: stack
        enabled: !livePinHost.blocking
        objectName: "mainNavigation"
        anchors.fill: parent
        hoverEnabled: false
        initialItem: mainViewComponent
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

    Window {
        id: separateDisplaySettingsWindow
        visible: useSeparateStreamSettingsWindows && displaySettingsRect.opacity > 0.0
        transientParent: Chiaki.window
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
        transientParent: Chiaki.window
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
        transientParent: Chiaki.window
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
        navigation: root
    }

    ConfirmDialog {
        id: confirmDialog
        navigation: root
    }

    RegistrationMethodDialog {
        id: registrationMethodDialog
        navigation: root
    }

    RemindDialog {
        id: remindDialog
        navigation: root
    }

    Connections {
        target: Chiaki

        function onConnectionAttemptChanged() {
            const attempt = Chiaki.connectionAttempt;
            if (attempt.presented && attempt.waitingForWake) {
                if (!stack.currentItem || stack.currentItem.objectName !== "autoConnectView")
                    stack.replace(stack.get(0), autoConnectViewComponent, {}, StackView.Immediate);
                return;
            }
            if (!attempt.presented || attempt.waitingForWake || Chiaki.autoConnect) return;
            if (attempt.route === "psn" && attempt.phase !== "video" && attempt.phase !== "ready")
                root.showPsnView();
            else if (attempt.phase !== "ready") root.showStreamView();
        }

        function onSessionChanged() {
            if (Chiaki.session)
                root.showStreamView();
        }

        function onShowPsnView() {
            root.showPsnView();
        }

        function onPsnCredsExpired() {
            Chiaki.settings.psnRefreshToken = ""
            Chiaki.settings.psnAuthToken = ""
            Chiaki.settings.psnAuthTokenExpiry = ""
            Chiaki.settings.psnAccountId = ""
            root.showPSNTokenDialog("", true);
        }

        function onError(title, text) {
            const attempt = Chiaki.connectionAttempt;
            if (attempt.presented && attempt.errorTitle === title && attempt.errorText === text) return;
            errorTitleLabel.text = title;
            errorTextLabel.text = text;
            errorHideTimer.start();
        }

        function onRegistDialogRequested(host, ps5, duid, context) {
            if(!duid)
                showRegistDialog(host, ps5, context);
            else {
                rememberMainFocus();
                registrationMethodDialog.chooseRegistration(host, ps5, context, Window.window.activeFocusItem);
            }
        }

        function onWakeupStartInitiated() {
            if (!stack.currentItem || stack.currentItem.objectName !== "autoConnectView")
                stack.replace(stack.get(0), autoConnectViewComponent, {}, StackView.Immediate);
        }
    }

    Component {
        id: mainViewComponent
        MainView { }
    }

    Component {
        id: streamViewComponent
        StreamView { }
    }

    Component {
        id: autoConnectViewComponent
        AutoConnectView { }
    }

    Component {
        id: psnViewComponent
        PsnView {}
    }

    Component {
        id: manualHostDialogComponent
        ManualHostDialog { }
    }

    Component {
        id: settingsDialogComponent
        SettingsDialog { }
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

    Component {
        id: profileDialogComponent
        ProfileDialog { }
    }

    Component {
        id: consolePinDialogComponent
        ConsolePinDialog { }
    }

    Component {
        id: steamShortcutDialogComponent
        SteamShortcutDialog { }
    }

    Component {
        id: psnTokenDialogComponent
        PSNTokenDialog { }
    }

    Component {
        id: registDialogComponent
        RegistDialog { }
    }

    Component {
        id: controllerMappingDialogComponent
        ControllerMappingDialog { }
    }
}
