import QtQuick
import QtQuick.Controls
import QtTest
import "file:///C:/Users/david/Projects/chiaki-ng-ui/gui/src/qml"
TestCase {
 id: testCase; name: "ReviewScroll"; width: 900; height: 600; visible: true; when: windowShown
 Component { id: component; ConfirmDialog {} }
 function test_keyboardCanReadLongActionMessage() {
  const dialog = createTemporaryObject(component, testCase, {title: "Reminder", actionText: "Set up now", secondaryActionText: "Don't ask again", cancelText: "Later", text: "Long translated instructions. ".repeat(120)});
  dialog.open(); tryCompare(dialog, "opened", true);
  const scroll = dialog.contentItem;
  verify(scroll.contentHeight > scroll.height);
  for (let i = 0; i < 12; i++) { keyClick(Qt.Key_Tab); keyClick(Qt.Key_PageDown); }
  for (let i = 0; i < 12; i++) { keyClick(Qt.Key_Up); keyClick(Qt.Key_PageDown); }
  verify(scroll.contentItem.contentY > 0, "Text must scroll by keyboard");
 }
}
