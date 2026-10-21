import QtQuick
import QtQuick.Controls

import org.streetpea.chiaking

Pane {
    padding: 0
    id: consolePane
    function rememberFocus() { library.rememberFocus(); }
    function restoreFocus() { library.restoreFocus(); }
    StackView.onDeactivating: {
        library.rememberFocus();
        root.librarySelection = { profile: library.profileKey, host: library.selectedTarget ? library.selectedTarget.host : null };
    }
    StackView.onActivated: {
        const saved = root.librarySelection;
        if (saved && saved.profile === library.profileKey && saved.host) {
            library.selectedIdentity = library.identityOf(saved.host);
            library.updateSelection();
        }
        library.restoreFocus();
        if(!Chiaki.autoConnect && !root.initialAsk && !Chiaki.window.directStream)
        {
            root.initialAsk = true;
            if(Chiaki.settings.addSteamShortcutAsk && (typeof Chiaki.createSteamShortcut === "function"))
                root.showRemindDialog(qsTr("Official Steam artwork + controller layout"), qsTr("Would you like to either create a new non-Steam game for chiaki-ng\nor update an existing non-Steam game with the official artwork and controller layout?") + "\n\n" + qsTr("(Note: If you select no now and want to do this later, click the button or press R3 from the main menu.)"), false, () => root.showSteamShortcutDialog(true));
            else if(Chiaki.settings.remotePlayAsk)
            {
                if(!Chiaki.settings.psnRefreshToken || !Chiaki.settings.psnAuthToken || !Chiaki.settings.psnAuthTokenExpiry || !Chiaki.settings.psnAccountId)
                    root.showRemindDialog(qsTr("Remote Play via PSN"), qsTr("Would you like to connect to PSN?\nThis enables:\n- Automatic registration\n- Playing outside of your home network without port forwarding?") + "\n\n" + qsTr("(Note: If you select no now and want to do this later, go to the Config section of the settings.)"), true, () => root.showPSNTokenDialog(false));
                else
                    Chiaki.settings.remotePlayAsk = false;
            }
        }
    }
    function manageConsole(action, accept, cancel, consoleName) {
        if (action === "pin") {
            root.showConsolePinDialog(accept, cancel, consoleName);
        } else if (action === "delete") {
            root.showConfirmDialog(qsTr("Delete Console"), qsTr("Are you sure you want to delete this console?"), accept, cancel);
        } else if (action === "hide") {
            root.showConfirmDialog(qsTr("Hide Console"), qsTr("Are you sure you want to hide this console?") + "\n\n" + qsTr("Note: You can unhide from the Consoles section of the Settings under Hidden Consoles"), accept, cancel);
        }
    }

    ConsoleLibrary {
        id: library
        anchors.fill: parent
        focus: true
        hosts: Chiaki.hosts
        streamerMode: Chiaki.settings.streamerMode
        profileKey: Chiaki.settings.currentProfile
        discoveryEnabled: Chiaki.discoveryEnabled
        psnAvailable: !!Chiaki.settings.psnAuthToken
        steamAvailable: typeof Chiaki.createSteamShortcut === "function"
        applicationVersion: Qt.application.version
        playStationController: Chiaki.controllers.some(controller => controller.playStation)
        onConnectRequested: (sourceIndex, nickname) => Chiaki.connectToHost(sourceIndex, nickname)
        onSettingsRequested: root.showSettingsDialog()
        onAddConsoleRequested: root.showManualHostDialog()
        onRefreshPsnRequested: Chiaki.refreshPsnToken()
        onSteamShortcutRequested: root.showSteamShortcutDialog(false)
        onDiscoveryToggled: enabled => Chiaki.discoveryEnabled = enabled
        onQuitRequested: root.showConfirmDialog(qsTr("Quit"), qsTr("Are you sure you want to quit?"), () => Qt.quit())
        onManagementRequested: (action, accept, cancel, consoleName) => consolePane.manageConsole(action, accept, cancel, consoleName)
        onWakeRequested: sourceIndex => Chiaki.wakeUpHost(sourceIndex)
        onDeleteRequested: sourceIndex => Chiaki.deleteHost(sourceIndex)
        onHideRequested: (mac, nickname) => Chiaki.hideHost(mac, nickname)
        savePin: (sourceIndex, pin) => Chiaki.setConsolePin(sourceIndex, pin)
    }
}
