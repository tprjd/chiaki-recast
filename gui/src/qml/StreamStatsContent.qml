import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Item {
    id: stats
    required property var backend
    required property var streamWindow
    anchors.fill: parent
    function fixed(value, precision) { return (isFinite(value) ? value : 0).toFixed(precision); }

    Rectangle {
        objectName: "streamStatsReadout"
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 18
        width: Math.min(parent.width - 36, metrics.implicitWidth + 20)
        height: metrics.implicitHeight + 14
        radius: 5
        color: "#a6090e15"
        Accessible.name: qsTr("Stream statistics")

        RowLayout {
            id: metrics
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            anchors.topMargin: 7
            anchors.bottomMargin: 7
            spacing: 16
            Repeater {
                model: [
                    {key: "Bitrate", label: qsTr("Bitrate"), name: qsTr("Bitrate")},
                    {key: "Loss", label: qsTr("Loss"), name: qsTr("Packet loss")},
                    {key: "Dropped", label: qsTr("Dropped"), name: qsTr("Dropped frames")},
                    {key: "Lost", label: qsTr("Lost"), name: qsTr("Lost frames")},
                    {key: "Queue", label: qsTr("Queue"), name: qsTr("Average queue depth")},
                    {key: "Age", label: qsTr("Age"), name: qsTr("Pending frame age")}
                ]
                RowLayout {
                    required property var modelData
                    Layout.fillWidth: true
                    spacing: 5
                    Label {
                        objectName: "streamStats" + modelData.key + "Label"
                        Layout.fillWidth: true
                        text: modelData.label
                        elide: Text.ElideRight
                        color: "#e0e5ed"
                        font.pixelSize: 11
                        Accessible.name: modelData.name
                    }
                    Label {
                        objectName: "streamStats" + modelData.key
                        text: {
                            if (!stats.backend.session) return "";
                            switch (modelData.key) {
                            case "Bitrate": return qsTr("%1 Mbps").arg(stats.fixed(stats.backend.session.measuredBitrate, 1));
                            case "Loss": return qsTr("%1 %").arg(stats.fixed(stats.backend.session.averagePacketLoss * 100, 1));
                            case "Dropped": return stats.fixed(stats.streamWindow.droppedFrames, 0);
                            case "Lost": return stats.fixed(stats.backend.session.framesLost, 0);
                            case "Queue": return stats.fixed(stats.streamWindow.queueDepthAverage, 1);
                            case "Age": return qsTr("%1 ms").arg(stats.fixed(stats.streamWindow.pendingFrameAge * 1000, 0));
                            }
                        }
                        color: "#ffffff"
                        font.pixelSize: 12
                        font.weight: Font.DemiBold
                        Accessible.name: modelData.name + " " + text
                    }
                }
            }
        }
    }
}
