import QtQuick
import QtQuick.Controls
import "../../../src/qml"

Item {
    id: harness
    width: 1280
    height: 720
    readonly property var screen: screenLoader.item ? screenLoader.item.get(0) : null
    property alias settings: fakeSettings
    property alias backend: fakeBackend
    property string initialHost: "console.local"
    property bool initialPs5: true
    property var initialContext: null
    property bool screenActive: true
    property var actions: []
    property var accountCallback: null
    function closeDialog() { actions = actions.concat([{name: "close"}]); }
    function showMainView() { actions = actions.concat([{name: "consoles"}]); }
    function returnFromAccount(id) {
        if (id) accountCallback(id);
        screenLoader.item.pop();
    }
    QtObject {
        id: fakeSettings
        property string psnAccountId: ""
        property bool streamerMode: false
        property string currentProfile: "profile-a"
    }
    QtObject {
        id: fakeBackend
        property alias settings: fakeSettings
        property bool acceptRegistration: true
        property var requests: []
        property var callback: null
        signal error(string title, string text)
        function createRegistrationContext() { return {profile: fakeSettings.currentProfile}; }
        function registerHost(host, identity, pin, consolePin, broadcast, target, cb, context) {
            requests = requests.concat([{host: host, identity: identity, pin: pin,
                consolePin: consolePin, broadcast: broadcast, target: target, context: context}]);
            callback = cb;
            if (!acceptRegistration) error("Invalid Account-ID", "Use an Account ID with eight bytes encoded as Base64.");
            return acceptRegistration;
        }
    }
    Loader {
        id: screenLoader
        anchors.fill: parent
        active: harness.screenActive
        sourceComponent: StackView {
            initialItem: RegistDialog {
                host: harness.initialHost
                ps5: harness.initialPs5
                registrationContext: harness.initialContext || fakeBackend.createRegistrationContext()
                backend: fakeBackend
                navigation: harness
                openAccount: function(login, callback) {
                    harness.accountCallback = callback;
                    harness.actions = harness.actions.concat([{name: "account", login: login}]);
                    screenLoader.item.push(accountPage);
                }
            }
        }
    }
    Component { id: accountPage; Item {} }
}
