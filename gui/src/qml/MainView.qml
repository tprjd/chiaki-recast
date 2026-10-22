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

    }
    function manageConsole(action, accept, cancel, consoleName) {
        if (action === "delete") {
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
        applicationVersion: Qt.application.version
        playStationController: backend.controllers.some(controller => controller.playStation)
        onConnectRequested: (sourceIndex, nickname) => backend.connectToHost(sourceIndex, nickname)
        onSettingsRequested: navigation.showSettingsDialog()
        onRefreshPsnRequested: backend.refreshPsnToken()
        onDiscoveryToggled: enabled => backend.discoveryEnabled = enabled
        onQuitRequested: navigation.showConfirmDialog(qsTr("Quit"), qsTr("Are you sure you want to quit?"), qsTr("Quit"), () => Qt.quit())
        onManagementRequested: (action, accept, cancel, consoleName) => consolePane.manageConsole(action, accept, cancel, consoleName)
        onWakeRequested: sourceIndex => backend.wakeUpHost(sourceIndex)
        onDeleteRequested: sourceIndex => backend.deleteHost(sourceIndex)
        onHideRequested: (mac, nickname) => backend.hideHost(mac, nickname)
    }
}
