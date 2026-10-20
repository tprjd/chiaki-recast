import QtQuick
import QtQuick.Controls

import org.streetpea.chiaking

Pane {
    padding: 0
    id: consolePane
    StackView.onActivated: {
        library.forceActiveFocus(Qt.TabFocusReason);
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
    function manageConsole(action, identity) {
        const target = library.resolveTarget(identity);
        if (!target || !library.eligible(action, target.host))
            return;
        if (action === "pin") {
            root.showConsolePinDialog(target.sourceIndex);
        } else if (action === "wake") {
            Chiaki.wakeUpHost(target.sourceIndex);
        } else if (action === "delete") {
            root.showConfirmDialog(qsTr("Delete Console"), qsTr("Are you sure you want to delete this console?"), () => {
                const current = library.resolveTarget(identity);
                if (current && library.eligible("delete", current.host))
                    Chiaki.deleteHost(current.sourceIndex);
            });
        } else if (action === "hide") {
            root.showConfirmDialog(qsTr("Hide Console"), qsTr("Are you sure you want to hide this console?") + "\n\n" + qsTr("Note: You can unhide from the Consoles section of the Settings under Hidden Consoles"), () => {
                const current = library.resolveTarget(identity);
                if (current && library.eligible("hide", current.host))
                    Chiaki.hideHost(current.host.mac, current.host.name);
            });
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
        onManagementRequested: (action, identity) => consolePane.manageConsole(action, identity)
    }
}
