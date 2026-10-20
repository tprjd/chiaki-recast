import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import org.streetpea.chiaking
import "controls" as C

DialogView {
    id: dialog
    objectName: "profileDialog"
    property var backend: Chiaki
    property var navigation: root
    property string selectedProfile: backend.settings.currentProfile || "default"
    readonly property bool creating: selectedProfile === "create new profile"
    readonly property bool canDelete: backend.settings.profiles.indexOf(selectedProfile) >= 0
        && selectedProfile !== "default" && !creating && selectedProfile !== backend.settings.currentProfile
    title: qsTr("Profiles")
    Material.theme: Material.Dark
    Material.primary: "#11151c"
    Material.background: "#11151c"
    function close() { navigation.closeDialog(); }
    buttonText: deleteBox.visible && deleteBox.checked ? qsTr("Delete Profile") : creating ? qsTr("Create Profile") : qsTr("Switch Profile")
    buttonEnabled: backend.settings.profiles.indexOf(selectedProfile) >= 0
        && (creating ? profileName.text.trim().length > 0
                     : selectedProfile !== (backend.settings.currentProfile || "default"))
    onAccepted: {
        if (!buttonEnabled) return;
        if (canDelete && deleteBox.checked)
            backend.settings.deleteProfile(selectedProfile);
        else if (creating)
            backend.settings.currentProfile = profileName.text.trim();
        else
            backend.settings.currentProfile = selectedProfile === "default" ? "" : selectedProfile;
        close();
    }
    function refreshSelection() {
        profileComboBox.popup.close();
        selectedProfile = backend.settings.currentProfile || "default";
        deleteBox.checked = false;
        profileName.clear();
    }
    Item {
        Connections {
            target: dialog.backend.settings
            function onProfilesChanged() { dialog.refreshSelection(); }
            function onCurrentProfileChanged() { dialog.refreshSelection(); }
        }
        Rectangle { anchors.fill: parent; color: "#11151c" }
        ColumnLayout {
            anchors { top: parent.top; left: parent.left; right: parent.right; margins: 24 }
            spacing: 14
            Label {
                objectName: "activeProfile"
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                font.pixelSize: 14
                text: qsTr("Current profile: %1").arg(dialog.backend.settings.streamerMode ? qsTr("Hidden") : (dialog.backend.settings.currentProfile || qsTr("default")))
            }
            Frame {
                Layout.fillWidth: true
                background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                GridLayout {
                    anchors.fill: parent
                    columns: 2
                    columnSpacing: 24
                    rowSpacing: 12
                    Label { text: qsTr("User profile"); font.pixelSize: 14; Layout.fillWidth: true }
                    C.ComboBox {
                        id: profileComboBox
                        objectName: "profileSelection"
                        Accessible.name: qsTr("User profile")
                        Layout.preferredWidth: 450
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        firstInFocusChain: true
                        font.pixelSize: 14
                        popup.width: width
                        model: dialog.backend.settings.profiles.map(function(name, index) {
                            if (name === "default") return qsTr("default");
                            if (name === "create new profile") return qsTr("Create new profile");
                            return dialog.backend.settings.streamerMode ? qsTr("Profile %1").arg(index) : name;
                        })
                        currentIndex: dialog.backend.settings.profiles.indexOf(dialog.selectedProfile)
                        onActivated: index => {
                            dialog.selectedProfile = dialog.backend.settings.profiles[index];
                            deleteBox.checked = false;
                        }
                        lastInFocusChain: !dialog.creating && !dialog.canDelete
                    }
                    Label { text: qsTr("New profile name"); font.pixelSize: 14; visible: dialog.creating }
                    C.TextField {
                        id: profileName
                        objectName: "newProfileName"
                        Accessible.name: qsTr("New profile name")
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        font.pixelSize: 14
                        visible: dialog.creating
                        echoMode: dialog.backend.settings.streamerMode ? TextInput.Password : TextInput.Normal
                        lastInFocusChain: true
                    }
                    Label { text: qsTr("Delete selected profile"); font.pixelSize: 14; visible: dialog.canDelete }
                    C.CheckBox {
                        id: deleteBox
                        objectName: "deleteProfile"
                        Accessible.name: qsTr("Delete selected profile")
                        visible: dialog.canDelete
                        lastInFocusChain: true
                    }
                }
            }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                font.pixelSize: 14
                color: "#aeb9c8"
                text: qsTr("Select a profile, then use the action above. The default and current profiles cannot be deleted.")
            }
        }
    }
}
