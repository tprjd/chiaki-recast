import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Window

import org.streetpea.chiaking

Item {
    id: streamStatsContentRoot
    required property var backend
    required property var streamWindow
    anchors.fill: parent
    ColumnLayout {
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
            rightMargin: 5
        }

        Label {
            Layout.alignment: Qt.AlignRight
            text: "Mbps"
            font.pixelSize: 18
            visible: backend.session ? true : false

            Label {
                anchors {
                    right: parent.left
                    baseline: parent.baseline
                    rightMargin: 5
                }
                text: parent.visible && backend.session ? backend.session.measuredBitrate.toFixed(1) : ""
                color: Material.accent
                font.bold: true
                font.pixelSize: 28
            }
        }

        Label {
            Layout.alignment: Qt.AlignRight
            text: qsTr("queue depth avg")
            font.pixelSize: 15
            opacity: backend.session ? 1 : 0
            visible: opacity > 0

            Behavior on opacity { NumberAnimation { duration: 250 } }

            Label {
                anchors {
                    right: parent.left
                    baseline: parent.baseline
                    rightMargin: 5
                }
                text: parent.visible && backend.session ? streamWindow.queueDepthAverage.toFixed(1) : ""
                font.bold: true
                color: "#90caf9"
                font.pixelSize: 18
            }
        }

        Label {
            Layout.alignment: Qt.AlignRight
            text: qsTr("pending frame age")
            font.pixelSize: 15
            opacity: backend.session ? 1 : 0
            visible: opacity > 0

            Behavior on opacity { NumberAnimation { duration: 250 } }

            Label {
                anchors {
                    right: parent.left
                    baseline: parent.baseline
                    rightMargin: 5
                }
                text: parent.visible && backend.session ? qsTr("%1 ms").arg((streamWindow.pendingFrameAge * 1000.0).toFixed(0)) : ""
                font.bold: true
                color: "#90caf9"
                font.pixelSize: 18
            }
        }

        Label {
            Layout.alignment: Qt.AlignRight
            id: statsPacketLossLabel
            text: qsTr("packet loss")
            font.pixelSize: 15
            opacity: backend.session ? 1 : 0
            visible: opacity > 0

            Behavior on opacity { NumberAnimation { duration: 250 } }

            Label {
                anchors {
                    right: parent.left
                    baseline: parent.baseline
                    rightMargin: 5
                }
                text: parent.visible && backend.session ? "%1<font size=\"1\">%</font>".arg((((backend.session && isFinite(backend.session.averagePacketLoss)) ? backend.session.averagePacketLoss : 0) * 100).toFixed(1)) : ""
                font.bold: true
                color: "#ef9a9a"
                font.pixelSize: 18
            }
        }

        Label {
            Layout.alignment: Qt.AlignRight
            text: qsTr("dropped frames")
            font.pixelSize: 15
            opacity: backend.session ? 1 : 0
            visible: opacity > 0

            Behavior on opacity { NumberAnimation { duration: 250 } }

            Label {
                id: statsDroppedFramesLabel
                anchors {
                    right: parent.left
                    baseline: parent.baseline
                    rightMargin: 5
                }
                text: parent.visible && backend.session ? streamWindow.droppedFrames : ""
                color: "#ef9a9a"
                font.bold: true
                font.pixelSize: 18
            }
        }

        Label {
            Layout.alignment: Qt.AlignRight
            text: qsTr("lost frames")
            font.pixelSize: 15
            opacity: backend.session ? 1 : 0
            visible: opacity > 0

            Behavior on opacity { NumberAnimation { duration: 250 } }

            Label {
                anchors {
                    right: parent.left
                    baseline: parent.baseline
                    rightMargin: 5
                }
                text: parent.visible && backend.session ? ((backend.session && isFinite(backend.session.framesLost)) ? backend.session.framesLost : 0) : ""
                color: "#ef9a9a"
                font.bold: true
                font.pixelSize: 18
            }
        }
    }
}
