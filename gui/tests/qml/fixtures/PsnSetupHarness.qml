import QtQuick
import "../../../src/qml"

Item {
    id: harness
    width: 1280; height: 720
    property alias backend: backend
    property alias screen: screen
    property var actions: []
    property bool browserAvailable: false
    property bool browserHintsFail: false
    property bool embedded: false
    property bool screenActive: true
    property var browserCommands: []
    function closeDialog() { actions = actions.concat(["close"]); }
    QtObject {
        id: settings
        property bool streamerMode: false
        property bool remotePlayAsk: true
    }
    QtObject {
        id: backend
        property alias settings: settings
        property var psnAuthAttempt: ({})
        property var commands: []
        property bool rejectRedirect: false
        function psnLoginUrl() { return "https://example.invalid/sign-in"; }
        function openPsnLink() { commands = commands.concat(["browser"]); return psnLoginUrl(); }
        function checkPsnRedirectURL(url) { return String(url).indexOf("https://remoteplay.dl.playstation.net/remoteplay/redirect") === 0; }
        function startPsnAuth(url) {
            if (psnAuthAttempt.phase === "exchanging" || psnAuthAttempt.phase === "account") return "";
            commands = commands.concat(["submit"]);
            psnAuthAttempt = {id: String(commands.length), phase: rejectRedirect ? "failure" : "exchanging",
                inputRejected: rejectRedirect, message: rejectRedirect ? "Paste the complete Sign-in redirect URL." : ""};
            return psnAuthAttempt.id;
        }
        function change(values) { psnAuthAttempt = Object.assign({}, psnAuthAttempt, values); }
        function setWebEngineHints(profile) { if (harness.browserHintsFail) throw new Error("Controlled browser initialization failure"); }
        function clearCookies(profile) { harness.browserCommands = harness.browserCommands.concat(["cookies"]); }
    }
    PSNTokenDialog {
        id: screen
        anchors.fill: parent
        backend: backend
        navigation: harness
        preferEmbedded: harness.embedded
        createBrowser: function(parent) {
            if (!harness.browserAvailable) throw new Error("Controlled browser unavailability");
            return browserComponent.createObject(parent);
        }
    }
    Component {
        id: browserComponent
        Item {
            property url url
            property alias profile: profile
            signal navigationRequested(var request)
            function reload() { harness.browserCommands = harness.browserCommands.concat(["reload"]); }
            function stop() { harness.browserCommands = harness.browserCommands.concat(["stop"]); }
            QtObject {
                id: profile
                signal clearHttpCacheCompleted()
                function clearHttpCache() { harness.browserCommands = harness.browserCommands.concat(["cache"]); clearHttpCacheCompleted(); }
            }
        }
    }
}
