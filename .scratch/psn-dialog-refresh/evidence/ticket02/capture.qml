import QtQuick
import QtTest
import "../../../../gui/tests/qml/fixtures"

TestCase {
    id: testCase
    name: "RegistrationAccountCapture"
    width: 1280; height: 720
    visible: true; when: windowShown
    Component { id: fixture; RegistrationAccountHarness {} }
    function test_capture_data() {
        return [{tag: "900x600", width: 900, height: 600}, {tag: "1280x720", width: 1280, height: 720}];
    }
    function capture(h, name) {
        tryCompare(h, "navigating", false);
        let saved = false;
        const path = Qt.resolvedUrl(name + ".png").toString().replace("file:///", "");
        waitForRendering(h);
        verify(h.grabToImage(result => { saved = result.saveToFile(path); }));
        tryVerify(() => saved);
    }
    function test_capture(data) {
        const h = createTemporaryObject(fixture, testCase, {width: data.width, height: data.height});
        findChild(h.screen, "registrationLookup").clicked();
        tryVerify(() => findChild(h.current, "lookupUsername").activeFocus);
        capture(h, "lookup-" + data.tag);
        findChild(h.current, "lookupUsername").text = "public-user";
        findChild(h.current, "lookupSubmit").clicked();
        h.lookupCallbacks[0](200, "not-json");
        capture(h, "lookup-error-" + data.tag);
        findChild(h.current, "lookupCancel").clicked();
        tryVerify(() => findChild(h.screen, "registrationAccountId").activeFocus);
        findChild(h.screen, "registrationLogin").clicked();
        tryVerify(() => findChild(h.current, "psnRedirect").activeFocus);
        capture(h, "sign-in-" + data.tag);
    }
}
