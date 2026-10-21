import QtQuick
import QtQuick.Controls
import QtTest
import "../../src/qml"

TestCase {
    id: tests
    name: "RegistrationMethod"
    width: 1280; height: 720
    visible: true
    when: windowShown
    property var automatic: []
    property var manual: []
    Button { id: opener; text: "Register" }
    Component { id: dialogComponent; RegistrationMethodDialog {} }
    function test_choice_data() {
        return ["automatic", "manual", "cancel", "escape"].map(choice => ({tag: choice, choice: choice}));
    }
    function test_choice(data) {
        automatic = []; manual = [];
        const dialog = createTemporaryObject(dialogComponent, tests, {
            backend: {autoRegister: context => automatic = automatic.concat([context])},
            navigation: {showRegistDialog: (host, ps5, context) => manual = manual.concat([{host: host, ps5: ps5, context: context}])}
        });
        const context = {profileGeneration: 7, automaticRequest: 4};
        dialog.chooseRegistration("192.0.2.1", false, context, opener);
        tryCompare(dialog, "opened", true);
        compare(findChild(dialog.footer, "confirmationAction").text, "Automatic registration");
        compare(findChild(dialog.footer, "confirmationSecondaryAction").text, "Manual registration");
        verify(dialog.text.includes("main PS4"));
        tryCompare(findChild(dialog.footer, "confirmationCancel"), "activeFocus", true);
        verify(findChild(dialog.footer, "confirmationCancel").visualFocus);
        if (data.choice === "automatic") keyClick(Qt.Key_Down);
        else if (data.choice === "manual") keyClick(Qt.Key_Up);
        keyClick(data.choice === "escape" ? Qt.Key_Escape : Qt.Key_Return);
        dialog.accept();
        tryCompare(dialog, "visible", false);
        compare(automatic.length, data.choice === "automatic" ? 1 : 0);
        compare(manual.length, data.choice === "manual" ? 1 : 0);
        if (automatic.length) compare(automatic[0], context);
        if (manual.length) {
            compare(manual[0].host, "192.0.2.1");
            compare(manual[0].ps5, false);
            compare(manual[0].context, context);
        }
        tryCompare(opener, "activeFocus", true);
    }
    function test_longTextAtSupportedSizes_data() {
        return [{tag: "900", width: 900, height: 600}, {tag: "1280", width: 1280, height: 720}];
    }
    function test_longTextAtSupportedSizes(data) {
        tests.width = data.width; tests.height = data.height;
        const dialog = createTemporaryObject(dialogComponent, tests, {backend: ({}), navigation: ({})});
        dialog.chooseRegistration("192.0.2.1", true, {}, opener);
        tryCompare(dialog, "opened", true);
        verify(!dialog.text.includes("PS4"));
        dialog.text = "Long registration instructions. ".repeat(90);
        dialog.actionText = "Automatically register the selected console for Remote Play";
        dialog.secondaryActionText = "Manually register the selected console for Remote Play";
        waitForRendering(tests);
        for (const name of ["confirmationCancel", "confirmationAction", "confirmationSecondaryAction"]) {
            const button = findChild(dialog.footer, name);
            button.forceActiveFocus(Qt.TabFocusReason);
            const position = button.mapToItem(tests, 0, 0);
            verify(button.visualFocus);
            verify(position.x >= 0 && position.x + button.width <= tests.width);
            verify(position.y >= 0 && position.y + button.height <= tests.height);
        }
        keyClick(Qt.Key_Escape);
        tryCompare(dialog, "visible", false);
    }
}
