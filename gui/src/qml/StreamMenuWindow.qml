import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Window

import org.streetpea.chiaking

Window {
    id: streamMenuWindow

    property Component menuContent
    property bool open: false
    property bool closing: false

    signal closeRequested()
    signal closeAnimationFinished()

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

    onActiveChanged: {
        if (active && open)
            focusTimer.restart();
    }

    Shortcut {
        enabled: streamMenuWindow.open
        sequences: [StandardKey.Cancel]
        onActivated: streamMenuWindow.closeRequested()
    }

    Shortcut {
        enabled: streamMenuWindow.open
        sequence: "Ctrl+O"
        onActivated: {
            if (streamMenuWindow.open)
                streamMenuWindow.closeRequested();
        }
    }

    Behavior on y {
        NumberAnimation {
            id: menuAnimation
            duration: 250
            onRunningChanged: {
                if (!running && !streamMenuWindow.open)
                    streamMenuWindow.closeAnimationFinished();
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
