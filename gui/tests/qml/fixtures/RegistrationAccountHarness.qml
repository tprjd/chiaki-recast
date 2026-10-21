import QtQuick
import QtQuick.Controls
import "../../../src/qml"

Item {
    id: harness
    width: 1280; height: 720
    readonly property var screen: stack.depth ? stack.get(0) : null
    readonly property var current: stack.currentItem
    readonly property bool navigating: stack.busy
    property alias backend: fakeBackend
    property var lookupRequests: []
    property var lookupCallbacks: []
    property int closes: 0
    function closeDialog() { closes++; stack.pop(); }
    function replaceRegistration() { stack.clear(); stack.push(registration); }
    function replaceAccount() { stack.push(replacement); }
    function lookup(username, callback) {
        lookupRequests = lookupRequests.concat([username]);
        lookupCallbacks = lookupCallbacks.concat([callback]);
    }
    QtObject {
        id: settings
        property string currentProfile: "profile-a"
        property string psnAccountId: ""
        property bool streamerMode: false
        property bool remotePlayAsk: true
    }
    QtObject {
        id: fakeBackend
        property alias settings: settings
        property var psnAuthAttempt: ({})
        property int signIns: 0
        function createRegistrationContext() { return {profile: settings.currentProfile}; }
        function psnLoginUrl() { return "https://example.invalid/sign-in"; }
        function openPsnLink() { return psnLoginUrl(); }
        function startPsnAuth(url) {
            if (psnAuthAttempt.phase === "exchanging" || psnAuthAttempt.phase === "account") return "";
            signIns++;
            psnAuthAttempt = {id: String(signIns), phase: "exchanging"};
            return psnAuthAttempt.id;
        }
        function finish(accountId) { psnAuthAttempt = {id: String(signIns), phase: "success", accountId: accountId}; }
        signal error(string title, string text)
    }
    StackView {
        id: stack
        anchors.fill: parent
        initialItem: RegistDialog { backend: fakeBackend; navigation: harness; accountLookup: harness.lookup }
    }
    Component { id: registration; RegistDialog { backend: fakeBackend; navigation: harness; accountLookup: harness.lookup } }
    Component { id: replacement; Item { objectName: "replacement" } }
}
