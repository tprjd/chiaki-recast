import QtQuick
import org.streetpea.chiaking

ConsolePinContent {
    id: content
    required property var backend
    required property string requestId
    property var navigation: root
    readonly property var request: backend.sessionPinRequest || ({})
    readonly property bool ownsRequest: request.id === requestId
    readonly property string livePhase: ownsRequest ? request.phase : "closed"
    property bool commandPending: false
    settingsEnabled: false
    privateMode: backend.settings.streamerMode
    consoleName: ownsRequest ? request.name || "" : ""
    title: qsTr("Console login PIN")
    contextLabel: qsTr("Remote Play sign-in")
    heading: livePhase === "submitting" ? qsTr("Signing in")
        : livePhase === "cancelling" ? qsTr("Cancelling connection")
        : livePhase === "cancelled" ? qsTr("Connection cancelled")
        : request.incorrect ? qsTr("Incorrect console login PIN") : qsTr("Sign in to your console")
    message: livePhase === "submitting" ? qsTr("Waiting for your console to accept the PIN.")
        : livePhase === "cancelling" ? qsTr("Waiting for the current session to stop.")
        : livePhase === "cancelled" ? qsTr("Your console is still available in the library.")
        : request.incorrect ? qsTr("The console did not accept the PIN. Enter the four-digit PIN for this console user again.")
        : qsTr("Enter the four-digit login PIN for this console user to continue Remote Play.")
    submitText: qsTr("Sign in")
    guidanceDetailTitle: qsTr("For this session only")
    guidanceDetail: qsTr("Signing in here does not save or replace your saved console login PIN.")
    actionHelp: qsTr("Used for this session only.")
    phase: livePhase === "form" || livePhase === "submitting" ? "form" : livePhase
    submitting: livePhase !== "form" || commandPending
    busy: livePhase === "submitting" || livePhase === "cancelling"
    incorrectPin: livePhase === "form" && !!request.incorrect
    canCancel: ownsRequest && (livePhase === "form" || livePhase === "submitting")
    showCancel: livePhase === "cancelling"
    showBack: livePhase === "cancelled"
    function send(value) {
        if (!ownsRequest || livePhase !== "form" || commandPending) return;
        commandPending = true;
        const accepted = backend.enterPin(requestId, value);
        clear();
        commandPending = false;
        if (!accepted && ownsRequest && livePhase === "form") focusPin();
    }
    function cancel() {
        if (livePhase === "cancelled") {
            if (backend.dismissSessionPin(requestId)) navigation.showMainView();
        } else if (canCancel && !commandPending) {
            commandPending = true;
            backend.cancelSessionPin(requestId);
            clear();
            commandPending = false;
        }
    }
    onLivePhaseChanged: {
        if (livePhase !== "form") clear();
    }
    onSubmitted: value => send(value)
    onCancelled: cancel()
    Component.onDestruction: clear()
}
