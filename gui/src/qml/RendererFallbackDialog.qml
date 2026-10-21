import QtQuick

ConfirmDialog {
    property string reason: ""
    objectName: "rendererInformationDialog"
    title: qsTr("Renderer fallback")
    text: qsTr("Vulkan renderer initialization failed and chiaki-ng switched to OpenGL.\n\nReason: %1").arg(reason)
    cancelText: qsTr("Close")
}
