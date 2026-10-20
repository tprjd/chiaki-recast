import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import org.streetpea.chiaking
import "controls" as C
import "SettingsSearch.js" as Search

DialogView {
    id: dialog
    objectName: "displaySettingsDialog"
    property var backend: Chiaki
    property var navigation: root
    property var searchRoute: null
    function revealSetting(route) { Search.reveal(dialog, route, "display", "displayTargetPrim"); }
    StackView.onActivated: Qt.callLater(function() {
        if (!searchRoute) return;
        const route = searchRoute;
        searchRoute = null;
        revealSetting(route);
    })
    property alias gridHeight: displayGrid.height
    title: qsTr("Display Settings")
    header: ""
    buttonVisible: false
    Material.theme: Material.Dark
    Material.primary: "#11151c"
    Material.background: "#11151c"
    function close() { navigation.closeDialog(); }
    function ensureFocusVisible() {
        const item = Window.window ? Window.window.activeFocusItem : null;
        if (!item) return;
        let ancestor = item;
        while (ancestor && ancestor !== displayFlick.contentItem) ancestor = ancestor.parent;
        if (!ancestor) return;
        const point = item.mapToItem(displayFlick.contentItem, 0, 0);
        if (point.y < displayFlick.contentY) displayFlick.contentY = point.y;
        else if (point.y + item.height > displayFlick.contentY + displayFlick.height)
            displayFlick.contentY = point.y + item.height - displayFlick.height;
    }
    Flickable {
        Rectangle { parent: dialog; anchors.fill: parent; color: "#11151c"; z: -1 }
        id: displayFlick
        objectName: "displayFlick"
        clip: true
        contentWidth: width
        contentHeight: displayGrid.height + 32
        flickableDirection: Flickable.VerticalFlick
        ScrollBar.vertical: ScrollBar {}
        ColumnLayout {
            id: displayGrid
            x: 16
            y: 16
            width: Math.max(0, displayFlick.width - 48)
            spacing: 14
            Label { text: qsTr("Display target"); font.pixelSize: 14; font.bold: true }
            Frame {
                Layout.fillWidth: true
                background: Rectangle { color: "#1a202b"; radius: 8; border.color: "#303a48" }
                GridLayout {
                    anchors.fill: parent
                    columns: displayFlick.width < 760 ? 1 : 2
                    columnSpacing: 24
                    rowSpacing: 12
                    Label { text: qsTr("Target primaries\nDefault: Auto"); font.pixelSize: 14; wrapMode: Text.Wrap; Layout.fillWidth: true }
                    C.ComboBox {
                        id: displayTargetPrim
                        objectName: "displayTargetPrim"
                        Accessible.name: qsTr("Target primaries")
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        Layout.preferredWidth: 500
                        font.pixelSize: 14
                        popup.width: width
                        firstInFocusChain: true
                        model: [qsTr("Auto"), qsTr("ITU-R Rec. BT.601 NTSC (Standard Gamut)"), qsTr("ITU-R Rec. BT.601 PAL (Standard Gamut)"), qsTr("ITU-R Rec. BT.709 (Standard Gamut)"), qsTr("ITU-R Rec. BT.470 M (Standard Gamut)"), qsTr("EBU Tech. 3213-E (Standard Gamut)"), qsTr("ITU-R Rec. BT.2020 (Wide Gamut)"), qsTr("Apple RGB (Wide Gamut)"), qsTr("Adobe RGB (Wide Gamut)"), qsTr("ProPhoto RGB (Wide Gamut)"), qsTr("CIE 1931 RGB primaries (Wide Gamut)"), qsTr("DCI-P3 (Wide Gamut)"), qsTr("DCI-P3 with D65 white point (Wide Gamut)"), qsTr("Panasonic V-Gamut (Wide Gamut)"), qsTr("Sony S-Gamut (Wide Gamut)"), qsTr("Traditional film primaries with Illuminant C (Wide Gamut)"), qsTr("ACES Primaries #0 (Wide Gamut)"), qsTr("ACES Primaries #1 (Wide Gamut)")]
                        currentIndex: dialog.backend.settings.displayTargetPrim
                        onActivated: index => { dialog.backend.settings.displayTargetPrim = index; }
                    }
                    Label { text: qsTr("Target transfer characteristics\nDefault: Auto"); font.pixelSize: 14; wrapMode: Text.Wrap; Layout.fillWidth: true }
                    C.ComboBox {
                        id: displayTargetTrc
                        objectName: "displayTargetTrc"
                        Accessible.name: qsTr("Target transfer characteristics")
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        Layout.preferredWidth: 500
                        font.pixelSize: 14
                        popup.width: width
                        firstInFocusChain: false
                        model: [qsTr("Auto"), qsTr("ITU-R Rec. BT.1886 (SDR)"), qsTr("IEC 61966-2-4 sRGB (SDR)"), qsTr("Linear light content (SDR)"), qsTr("IPure power gamma 1.8 (SDR)"), qsTr("Pure power gamma 2.0 (SDR)"), qsTr("Pure power gamma 2.2 (SDR)"), qsTr("Pure power gamma 2.4 (SDR)"), qsTr("Pure power gamma 2.6 (SDR)"), qsTr("Pure power gamma 2.8 (SDR)"), qsTr("ProPhoto RGB (SDR)"), qsTr("Digital Cinema Distribution Master (SDR)"), qsTr("ITU-R BT.2100 PQ / SMPTE ST2048 (HDR)"), qsTr("ITU-R BT.2100 HLG / ARIB STD-B67 (HDR)"), qsTr("Panasonic V-Log (HDR)"), qsTr("Sony S-Log1 (HDR)"), qsTr("Sony S-Log2 (HDR)")]
                        currentIndex: dialog.backend.settings.displayTargetTrc
                        onActivated: index => { dialog.backend.settings.displayTargetTrc = index; }
                    }
                    Label { text: qsTr("Target peak\nDefault: Auto"); font.pixelSize: 14; wrapMode: Text.Wrap; Layout.fillWidth: true }
                    C.ComboBox {
                        id: targetPeakChoice
                        objectName: "targetPeakChoice"
                        Accessible.name: qsTr("Target peak")
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        Layout.preferredWidth: 500
                        font.pixelSize: 14
                        popup.width: width
                        firstInFocusChain: false
                        model: [qsTr("Auto"), qsTr("Numeric Value")]
                        currentIndex: dialog.backend.settings.displayTargetPeak === 0 ? 0 : 1
                        onActivated: index => { dialog.backend.settings.displayTargetPeak = index === 0 ? 0 : 1000; }
                    }
                    Label { text: qsTr("Target peak value\nDefault: 1000 nits"); font.pixelSize: 14; wrapMode: Text.Wrap; Layout.fillWidth: true; visible: targetPeakChoice.currentIndex === 1 }
                    ColumnLayout {
                        Layout.fillWidth: true
                        visible: targetPeakChoice.currentIndex === 1
                        C.Slider {
                            id: displayTargetPeak
                            objectName: "displayTargetPeak"
                            Accessible.name: qsTr("Target peak value in nits")
                            Layout.fillWidth: true
                            from: 10
                            to: 10000
                            stepSize: 10
                            value: dialog.backend.settings.displayTargetPeak
                            onMoved: dialog.backend.settings.displayTargetPeak = value
                        }
                        Label { text: qsTr("%1 nits").arg(displayTargetPeak.value); font.pixelSize: 14 }
                    }
                    Label { text: qsTr("Target contrast\nDefault: Auto"); font.pixelSize: 14; wrapMode: Text.Wrap; Layout.fillWidth: true }
                    C.ComboBox {
                        id: targetContrastChoice
                        objectName: "targetContrastChoice"
                        Accessible.name: qsTr("Target contrast")
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                        Layout.preferredWidth: 500
                        font.pixelSize: 14
                        popup.width: width
                        firstInFocusChain: false
                        model: [qsTr("Auto"), qsTr("Infinity"), qsTr("Numeric Value")]
                        currentIndex: dialog.backend.settings.displayTargetContrast === -1 ? 1 : dialog.backend.settings.displayTargetContrast === 0 ? 0 : 2
                        onActivated: index => { dialog.backend.settings.displayTargetContrast = index === 0 ? 0 : index === 1 ? -1 : 1000; }
                    }
                    Label { text: qsTr("Target contrast value\nDefault: 1000"); font.pixelSize: 14; wrapMode: Text.Wrap; Layout.fillWidth: true; visible: targetContrastChoice.currentIndex === 2 }
                    ColumnLayout {
                        Layout.fillWidth: true
                        visible: targetContrastChoice.currentIndex === 2
                        C.Slider {
                            id: displayTargetContrast
                            objectName: "displayTargetContrast"
                            Accessible.name: qsTr("Target contrast value")
                            Layout.fillWidth: true
                            from: 10
                            to: 1000000
                            stepSize: 1000
                            value: dialog.backend.settings.displayTargetContrast
                            onMoved: dialog.backend.settings.displayTargetContrast = value
                        }
                        Label { text: displayTargetContrast.value; font.pixelSize: 14 }
                    }
                }
            }
        }
        Connections {
            target: dialog.Window.window
            function onActiveFocusItemChanged() { dialog.ensureFocusVisible(); }
        }
    }
}
