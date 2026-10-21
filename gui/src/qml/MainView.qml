import QtQuick
import QtQuick.Controls

import org.streetpea.chiaking

Pane {
    padding: 0
    id: consolePane
    property var backend: Chiaki
    property var navigation: root
    function rememberFocus() { library.rememberFocus(); }
    function restoreFocus() { library.restoreFocus(); }
    StackView.onDeactivating: {
        library.rememberFocus();
        navigation.librarySelection = { profile: library.profileKey, host: library.selectedTarget ? library.selectedTarget.host : null };
    }
    StackView.onActivated: {
        const saved = navigation.librarySelection;
        if (saved && saved.profile === library.profileKey && saved.host) {
            library.selectedIdentity = library.identityOf(saved.host);
            library.updateSelection();
        }
        library.restoreFocus();
        if(!backend.autoConnect && !navigation.initialAsk && !backend.window.directStream)
        {
            navigation.initialAsk = true;
            if(backend.settings.addSteamShortcutAsk && (typeof backend.createSteamShortcut === "function"))
                navigation.showRemindDialog(qsTr("Official Steam artwork + controller layout"), qsTr("Would you like to either create a new non-Steam game for chiaki-ng\nor update an existing non-Steam game with the official artwork and controller layout?") + "\n\n" + qsTr("(Note: If you select no now and want to do this later, click the button or press R3 from the main menu.)"), false, () => navigation.showSteamShortcutDialog(true));
            else if(backend.settings.remotePlayAsk)
            {
                if(!backend.settings.psnRefreshToken || !backend.settings.psnAuthToken || !backend.settings.psnAuthTokenExpiry || !backend.settings.psnAccountId)
                    navigation.showRemindDialog(qsTr("Remote Play via PSN"), qsTr("Would you like to connect to PSN?\nThis enables:\n- Automatic registration\n- Playing outside of your home network without port forwarding?") + "\n\n" + qsTr("(Note: If you select no now and want to do this later, go to the Config section of the settings.)"), true, () => navigation.showPSNTokenDialog(false));
                else
                    backend.settings.remotePlayAsk = false;
            }
        }
    }
    function manageConsole(action, accept, cancel, consoleName) {
        if (action === "pin") {
            navigation.showConsolePinDialog(accept, cancel, consoleName);
        } else if (action === "delete") {
            navigation.showConfirmDialog(qsTr("Delete Console"), qsTr("Are you sure you want to delete this console?"), qsTr("Delete console"), accept, cancel);
        } else if (action === "hide") {
            navigation.showConfirmDialog(qsTr("Hide Console"), qsTr("Are you sure you want to hide this console?") + "\n\n" + qsTr("Note: You can unhide from the Consoles section of the Settings under Hidden Consoles"), qsTr("Hide console"), accept, cancel);
        }
    }

    ConsoleLibrary {
        id: library
        anchors.fill: parent
        focus: true
        hosts: backend.hosts
        streamerMode: backend.settings.streamerMode
        profileKey: backend.settings.currentProfile
        discoveryEnabled: backend.discoveryEnabled
        psnAvailable: !!backend.settings.psnAuthToken
        steamAvailable: typeof backend.createSteamShortcut === "function"
        applicationVersion: Qt.application.version
        playStationController: backend.controllers.some(controller => controller.playStation)
        onConnectRequested: (sourceIndex, nickname) => backend.connectToHost(sourceIndex, nickname)
        onSettingsRequested: navigation.showSettingsDialog()
        onAddConsoleRequested: navigation.showManualHostDialog()
        onRefreshPsnRequested: backend.refreshPsnToken()
        onSteamShortcutRequested: navigation.showSteamShortcutDialog(false)
        onDiscoveryToggled: enabled => backend.discoveryEnabled = enabled
        onQuitRequested: navigation.showConfirmDialog(qsTr("Quit"), qsTr("Are you sure you want to quit?"), qsTr("Quit"), () => Qt.quit())
        onManagementRequested: (action, accept, cancel, consoleName) => consolePane.manageConsole(action, accept, cancel, consoleName)
        onWakeRequested: sourceIndex => backend.wakeUpHost(sourceIndex)
        onDeleteRequested: sourceIndex => backend.deleteHost(sourceIndex)
        onHideRequested: (mac, nickname) => backend.hideHost(mac, nickname)
        savePin: (sourceIndex, pin) => backend.setConsolePin(sourceIndex, pin)
    }
}
