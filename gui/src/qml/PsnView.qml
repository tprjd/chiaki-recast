import QtQuick
import QtQuick.Controls
import org.streetpea.chiaking

Item {
    id: view
    objectName: "psnConnectionView"
    property bool connectionInputGrabbed: false
    StackView.onDeactivating: progress.retiring = true
    StackView.onActivating: progress.retiring = false
    Component.onCompleted: {
        Chiaki.window.grabInput();
        connectionInputGrabbed = true;
    }
    Component.onDestruction: {
        if (connectionInputGrabbed) Chiaki.window.releaseInput();
    }
    ConnectionView { id: progress; anchors.fill: parent; managesAttempt: phase !== "video" && phase !== "ready" }
}
