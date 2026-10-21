import QtQuick
import org.streetpea.chiaking

ConfirmDialog {
    id: dialog
    property var backend: Chiaki
    title: qsTr("Registration method")
    actionText: qsTr("Automatic registration")
    secondaryActionText: qsTr("Manual registration")
    function chooseRegistration(host, ps5, context, opener) {
        text = ps5 ? qsTr("Choose how to register this console for Remote Play.")
            : qsTr("Choose how to register this console for Remote Play. Automatic registration requires your main PS4 console registered to your PSN account.");
        callback = () => backend.autoRegister(context);
        secondaryCallback = () => navigation.showRegistDialog(host, ps5, context);
        restoreFocusItem = opener;
        open();
    }
}
