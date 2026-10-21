import QtQuick
import org.streetpea.chiaking

Item {
    id: view
    objectName: "psnConnectionView"
    property bool connectionInputGrabbed: false
    property string attemptId: Chiaki.connectionAttempt.id || ""
    Component.onCompleted: {
        Chiaki.window.grabInput();
        connectionInputGrabbed = true;
    }
    Component.onDestruction: {
        if (connectionInputGrabbed) Chiaki.window.releaseInput();
        const attempt = Chiaki.connectionAttempt;
        if (attempt.id === attemptId && attempt.phase !== "video" && attempt.phase !== "ready")
            Chiaki.dismissConnection(attemptId);
    }
    ConnectionView { anchors.fill: parent; managesAttempt: false }
}
