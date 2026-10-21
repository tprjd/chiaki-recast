import QtQuick
import QtQuick.Controls
import org.streetpea.chiaking

ConsolePinContent {
    id: dialog
    required property var submitPin
    required property var cancelPin
    property var navigation: root
    property bool released: false
    showSidebar: true
    privateMode: Chiaki.settings.streamerMode
    heading: phase === "saved" ? qsTr("Login PIN saved") : phase === "failure" ? qsTr("PIN was not saved") : qsTr("Sign in automatically")
    message: phase === "saved" ? qsTr("The app will use this PIN when the console asks you to sign in.") : qsTr("Save the four-digit login PIN for this console user.")
    function save() {
        if (phase !== "form" || !validPin || submitting) return;
        submitting = true;
        let accepted = false;
        try { accepted = submitPin(pin) === true; } catch (error) { accepted = false; }
        phase = accepted ? "saved" : "failure";
        if (!accepted) failureText = qsTr("The console is no longer available or the PIN could not be saved. Return to the library and select the console again.");
        submitting = false;
    }
    function release() {
        clear();
        if (released) return;
        released = true;
        cancelPin();
    }
    function close() {
        if (released) return;
        phase = "closed";
        release();
        navigation.closeDialog();
    }
    onSubmitted: save()
    onCancelled: close()
    onDone: close()
    onSettingsRequested: { close(); navigation.showSettingsDialog(); }
    StackView.onActivated: if (phase === "form") focusPin()
    StackView.onRemoved: release()
    Component.onDestruction: release()
}
