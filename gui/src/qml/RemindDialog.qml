import QtQuick
import org.streetpea.chiaking

ConfirmDialog {
    id: dialog
    property var backend: Chiaki
    property bool remotePlay: false
    objectName: "startupReminder"
    actionText: qsTr("Set up now")
    secondaryActionText: qsTr("Don't ask again")
    cancelText: qsTr("Later")

    function showReminder(heading, message, remote, setup, opener) {
        if (visible) return false;
        title = heading;
        text = message;
        remotePlay = remote;
        callback = setup;
        rejectCallback = () => { if (!remote) offerPsnReminder(); };
        secondaryCallback = () => {
            if (remote) backend.settings.remotePlayAsk = false;
            else backend.settings.addSteamShortcutAsk = false;
            if (!remote) offerPsnReminder();
        };
        restoreFocusItem = opener;
        open();
        return true;
    }

    function offerPsnReminder() {
        if (!backend.settings.remotePlayAsk) return;
        if (backend.settings.psnRefreshToken && backend.settings.psnAuthToken
                && backend.settings.psnAuthTokenExpiry && backend.settings.psnAccountId) {
            backend.settings.remotePlayAsk = false;
            return;
        }
        navigation.showRemindDialog(qsTr("PSN sign-in"),
            qsTr("Sign in to PSN to register consoles automatically and use Remote Play outside your home network. Sign-in saves your account credentials in Chiaki Recast. You can also set this up later in Settings, under Config."),
            true, () => navigation.showPSNTokenDialog(false));
    }
}
