import QtQuick
import QtQuick.Controls
import org.streetpea.chiaking

Item {
    id: view
    objectName: "psnConnectionView"
    property string presentation: ""
    StackView.onDeactivating: progress.retiring = true
    StackView.onActivating: progress.retiring = false
    Component.onCompleted: presentation = Chiaki.window.streamInput.present(view, "psn", null)
    ConnectionView { id: progress; anchors.fill: parent; managesAttempt: phase !== "video" && phase !== "ready" }
}
