import QtQuick

Item {
    id: notices
    property real networkOpacity: 0
    property bool audioDisabled: false
    property real packetLoss: 0
    implicitHeight: (networkOpacity > 0 ? networkNotice.implicitHeight : 0)
        + (audioDisabled ? audioNotice.implicitHeight : 0) + (networkOpacity > 0 && audioDisabled ? 8 : 0)
    Column {
        width: parent.width
        spacing: 8
        StreamNotice {
            id: networkNotice
            objectName: "streamNetworkNotice"
            width: Math.min(parent.width, implicitWidth)
            x: parent.width - width
            height: implicitHeight
            visible: opacity > 0
            opacity: notices.networkOpacity
            title: qsTr("Unstable connection")
            message: qsTr("Packet loss %1%").arg((notices.packetLoss * 100).toFixed(1))
            symbol: "!"
            warning: true
        }
        StreamNotice {
            id: audioNotice
            objectName: "streamAudioNotice"
            width: Math.min(parent.width, implicitWidth)
            x: parent.width - width
            height: implicitHeight
            visible: notices.audioDisabled
            title: qsTr("Audio disabled")
            message: qsTr("Settings → General → Audio/Video")
            symbol: "♪"
        }
    }
}
