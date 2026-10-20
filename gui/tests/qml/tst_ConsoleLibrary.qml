import QtQuick
import QtQuick.Controls
import QtTest
import "../../src/qml"

TestCase {
    id: testCase
    name: "ConsoleLibrary"
    width: 640
    height: 480
    visible: true
    when: windowShown

    Component {
        id: libraryComponent
        ConsoleLibrary { width: 320; height: 360 }
    }

    SignalSpy { id: connectSpy; signalName: "connectRequested" }

    function readyHost(name) {
        return { display: true, registered: true, discovered: true,
            state: "ready", name: name, address: "192.0.2.7",
            mac: "00:11:22:33:44:55", ps5: true };
    }

    function cleanup() {
        connectSpy.target = null;
        connectSpy.clear();
    }

    function test_readyConsoleConnectsToSourceOnce() {
        const library = createTemporaryObject(libraryComponent, testCase, {
            hosts: [readyHost("Living room")], sourceIndices: [7]
        });
        verify(library);
        connectSpy.target = library;
        compare(findChild(library, "consoleName").text, "Living room");
        compare(findChild(library, "consoleStatus").text, "Online");
        const connectButton = findChild(library, "connectButton");
        verify(connectButton.enabled);
        mouseClick(connectButton);
        compare(connectSpy.count, 1);
        compare(connectSpy.signalArguments[0][0], 7);
        compare(connectSpy.signalArguments[0][1], "Living room");
    }

    function test_enterConnectsOnceWithVisibleFocus() {
        const library = createTemporaryObject(libraryComponent, testCase, {
            hosts: [readyHost("Living room")], sourceIndices: [7]
        });
        connectSpy.target = library;
        const connectButton = findChild(library, "connectButton");
        connectButton.forceActiveFocus(Qt.TabFocusReason);
        verify(connectButton.visualFocus);
        keyClick(Qt.Key_Return);
        compare(connectSpy.count, 1);
        compare(connectSpy.signalArguments[0][0], 7);
    }

    function test_streamerModeMasksAddressName() {
        const library = createTemporaryObject(libraryComponent, testCase, {
            hosts: [readyHost("192.0.2.7")], streamerMode: true
        });
        const nameLabel = findChild(library, "consoleName");
        compare(nameLabel.text, "Manual console");
        compare(nameLabel.Accessible.name, "Manual console");
        compare(nameLabel.ToolTip.text, "Manual console");
        compare(findChild(library, "connectButton").Accessible.name,
            "Connect to Manual console");
    }
}
