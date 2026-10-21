import QtQuick
import QtQuick.Controls
import QtTest
import "../../src/qml"
import "fixtures"

TestCase {
    id: tests
    name: "ConsolePin"
    width: 1280
    height: 720
    visible: true
    when: windowShown
    property int saves: 0
    property int closes: 0
    property int cancels: 0
    property string received: ""
    Component { id: dialogComponent; ConsolePinDialog {} }
    Component { id: harnessComponent; ConsolePinHarness { width: 1280; height: 720 } }
    function host(changes) {
        return Object.assign({display: true, discovered: false, manual: true,
            registered: true, name: "Living room", mac: "00:11:22:33:44:55",
            address: "192.0.2.7", ps5: true}, changes || {});
    }
    function openLibraryForm(harness) {
        mouseClick(findChild(harness.library, "consoleActionsButton"));
        const details = findChild(harness.library, "consoleDetails");
        tryCompare(details, "opened", true);
        mouseClick(findChild(details.contentItem, "pinAction"));
        tryVerify(() => !!harness.dialog);
        findChild(harness.dialog, "consoleLoginPin").text = "0123";
    }
    function open(outcome = true) {
        saves = 0; closes = 0; cancels = 0; received = "";
        return createTemporaryObject(dialogComponent, tests, {
            width: tests.width, height: tests.height, consoleName: "Living room",
            privateMode: false, navigation: {closeDialog: () => closes++},
            submitPin: value => { saves++; received = value; return outcome; },
            cancelPin: () => cancels++
        });
    }
    function test_saveIsConfirmedOnceAndClearsInput() {
        const dialog = open();
        verify(dialog);
        const field = findChild(dialog, "consoleLoginPin");
        tryCompare(field, "activeFocus", true);
        const save = findChild(dialog, "pinSubmit");
        verify(!save.enabled);
        field.text = "0123";
        verify(save.enabled);
        keyClick(Qt.Key_Return);
        compare(saves, 1);
        verify(received === "0123", "Leading zeros reach the save callback unchanged");
        verify(field.text === "", "Completion clears the transient PIN");
        verify(findChild(dialog, "pinDone").visible);
        dialog.save();
        compare(saves, 1);
        mouseClick(findChild(dialog, "pinDone"));
        compare(closes, 1);
    }
    function test_invalidInputCannotSubmit_data() {
        return ["", "123", "abcd", "١٢٣٤"].map((value, index) => ({tag: String(index), value}));
    }
    function test_invalidInputCannotSubmit(data) {
        const dialog = open();
        const field = findChild(dialog, "consoleLoginPin");
        field.text = data.value;
        verify(!findChild(dialog, "pinSubmit").enabled);
        keyClick(Qt.Key_Return);
        compare(saves, 0);
    }
    function test_revealAndPrivacyAlwaysHidePrivateValues() {
        const dialog = open();
        const field = findChild(dialog, "consoleLoginPin");
        field.text = "0123";
        compare(field.echoMode, TextInput.Password);
        const reveal = findChild(dialog, "pinReveal");
        mouseClick(reveal);
        compare(field.echoMode, TextInput.Normal);
        mouseClick(reveal);
        compare(field.echoMode, TextInput.Password);
        mouseClick(reveal);
        dialog.privateMode = true;
        compare(field.echoMode, TextInput.Password);
        verify(!reveal.enabled);
        verify(findChild(dialog, "pinConsoleName").text.indexOf("Living room") < 0);
        dialog.privateMode = false;
        compare(field.echoMode, TextInput.Password);
        compare(findChild(dialog, "pinConsoleName").text, "Living room");
    }
    function test_rejectedSaveCannotShowSuccessOrSaveTwice() {
        const dialog = open(false);
        findChild(dialog, "consoleLoginPin").text = "0123";
        mouseClick(findChild(dialog, "pinSubmit"));
        compare(saves, 1);
        verify(!findChild(dialog, "pinDone").visible);
        verify(findChild(dialog, "pinFailure").visible);
        verify(findChild(dialog, "consoleLoginPin").text === "", "Rejection clears the transient PIN");
        dialog.save();
        compare(saves, 1);
        mouseClick(findChild(dialog, "pinBack"));
        compare(closes, 1);
    }
    function test_cancelDiscardsInput_data() {
        return [{tag: "button", escape: false}, {tag: "escape", escape: true}];
    }
    function test_cancelDiscardsInput(data) {
        const dialog = open();
        const field = findChild(dialog, "consoleLoginPin");
        field.text = "0123";
        if (data.escape) keyClick(Qt.Key_Escape);
        else mouseClick(findChild(dialog, "pinCancel"));
        compare(saves, 0);
        compare(closes, 1);
        verify(field.text === "", "Cancellation clears the transient PIN");
        dialog.save();
        compare(saves, 0);
        dialog.close();
        compare(closes, 1);
    }
    function test_removedFormInvalidatesTheManagementCallback() {
        const dialog = open();
        findChild(dialog, "consoleLoginPin").text = "0123";
        dialog.destroy();
        wait(0);
        compare(cancels, 1);
        compare(saves, 0);
    }
    function test_librarySaveFollowsTargetAfterReorderAndRestoresOpener() {
        const target = host();
        const peer = host({address: "192.0.2.8", mac: "00:11:22:33:44:66", name: "Bedroom"});
        const harness = createTemporaryObject(harnessComponent, tests);
        harness.library.hosts = [target, peer];
        openLibraryForm(harness);
        harness.library.hosts = [peer, target];
        mouseClick(findChild(harness.dialog, "pinSubmit"));
        compare(harness.saveCount, 1);
        compare(harness.savedIndex, 1);
        verify(harness.savedPin === "0123", "Save preserves leading zeros");
        compare(harness.connectionCount, 0);
        mouseClick(findChild(harness.dialog, "pinDone"));
        tryVerify(() => !harness.dialog);
        tryVerify(() => harness.library.activeFocus && tests.Window.window.activeFocusItem.objectName === "consoleActionsButton");
    }
    function test_invalidLibraryTargetNeverShowsSuccess_data() {
        return ["removed", "remove-return", "hidden", "ambiguous", "profile", "profile-return", "obsolete", "rejected"].map(change => ({tag: change, change}));
    }
    function test_invalidLibraryTargetNeverShowsSuccess(data) {
        const target = host();
        const harness = createTemporaryObject(harnessComponent, tests);
        harness.library.hosts = [target];
        openLibraryForm(harness);
        switch (data.change) {
        case "removed": harness.library.hosts = []; break;
        case "remove-return": harness.library.hosts = []; harness.library.hosts = [target]; break;
        case "hidden": harness.library.hosts = [host({display: false})]; break;
        case "ambiguous": harness.library.hosts = [target, target]; break;
        case "profile": harness.library.profileKey = "other"; break;
        case "profile-return": harness.library.profileKey = "other"; harness.library.profileKey = ""; break;
        case "obsolete": harness.library.requestManagement("pin", harness.library.selectedIdentity); break;
        case "rejected": harness.saveOutcome = false; break;
        }
        mouseClick(findChild(harness.dialog, "pinSubmit"));
        compare(harness.saveCount, data.change === "rejected" ? 1 : 0);
        compare(harness.connectionCount, 0);
        verify(!findChild(harness.dialog, "pinDone").visible);
        verify(findChild(harness.dialog, "pinBack").visible);
        mouseClick(findChild(harness.dialog, "pinBack"));
        tryVerify(() => !harness.dialog);
    }
    function test_expandedTextRemainsReachable_data() {
        return [{tag: "900", w: 900, h: 600}, {tag: "1280", w: 1280, h: 720}];
    }
    function test_expandedTextRemainsReachable(data) {
        const dialog = open();
        dialog.width = data.w; dialog.height = data.h;
        dialog.consoleName = "Long console name for the living room with extended text ".repeat(3);
        dialog.title = "Save console login PIN with expanded text";
        dialog.heading = "Sign in automatically with the console user login PIN";
        dialog.guidance = dialog.guidance + " " + dialog.guidance;
        for (const name of ["consoleLoginPin", "pinReveal", "pinCancel"]) {
            const item = findChild(dialog, name);
            item.forceActiveFocus(Qt.TabFocusReason);
            tryVerify(() => {
                const p = item.mapToItem(dialog, 0, 0);
                return p.x >= 0 && p.y >= 0 && p.x + item.width <= dialog.width && p.y + item.height <= dialog.height;
            }, 5000, name + " remains reachable");
        }
        waitForRendering(dialog);
        const image = grabImage(dialog);
        compare(image.width, data.w);
        compare(image.height, data.h);
    }
}
