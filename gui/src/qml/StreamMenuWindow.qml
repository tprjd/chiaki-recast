import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Window

import org.streetpea.chiaking

Window {
    id: streamMenuWindow

    property Component menuContent
    property string presentation: ""
    property bool open: false
    property bool closing: false
    property bool flyoutOpen: false
    readonly property real menuHeight: contentLoader.item ? contentLoader.item.implicitHeight : 80

    signal closeRequested()
    signal menuCloseRequested()
    signal closeAnimationFinished(string presentation)

    onClosing: event => { event.accepted = false; closeRequested(); }

    visible: open || closing
    flags: Qt.Tool | Qt.FramelessWindowHint
    color: "transparent"
    modality: Qt.NonModal

    function focusInitialItem() {
        requestActivate();
        if (contentLoader.item)
            contentLoader.item.initialFocusItem.forceActiveFocus(Qt.TabFocusReason);
    }

    onVisibleChanged: {
        if (visible)
            focusTimer.restart();
    }

    Shortcut {
        enabled: streamMenuWindow.open && !streamMenuWindow.flyoutOpen
        sequences: [StandardKey.Cancel]
        onActivated: streamMenuWindow.closeRequested()
    }

    Shortcut {
        enabled: streamMenuWindow.open && !streamMenuWindow.flyoutOpen
        sequence: "Ctrl+O"
        onActivated: {
            if (streamMenuWindow.open)
                streamMenuWindow.menuCloseRequested();
        }
    }

    onOpenChanged: {
        if (!open && presentation) closeTimerComponent.createObject(streamMenuWindow, {presentation: presentation}).start();
    }

    Component {
        id: closeTimerComponent
        Timer {
            property string presentation: ""
            interval: 250
            onTriggered: {
                if (!streamMenuWindow.open && presentation === streamMenuWindow.presentation)
                    streamMenuWindow.closeAnimationFinished(presentation);
                destroy();
            }
        }
    }

    Timer {
        id: focusTimer
        interval: 0
        repeat: false
        onTriggered: {
            if (streamMenuWindow.open)
                streamMenuWindow.focusInitialItem();
        }
    }

    Loader {
        id: contentLoader
        anchors.fill: parent
        sourceComponent: streamMenuWindow.menuContent
        enabled: streamMenuWindow.open
    }
}
