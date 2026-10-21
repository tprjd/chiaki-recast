import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Window

import org.streetpea.chiaking

FocusScope {
    id: content
    required property var backend
    required property var settings
    required property var streamWindow
    required property var navigation
    required property var menuController
    property bool separate: false
    focus: true
    Keys.onMenuPressed: menuController.close()
    Keys.onEscapePressed: menuController.close()
    property Item initialFocusItem: closeButton
    anchors.fill: parent

    Rectangle { anchors.fill: parent; color: "#2b2b2b"; visible: content.separate }

    Canvas {
        visible: !content.separate
        anchors.fill: parent
        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()
        onPaint: {
            let ctx = getContext("2d");
            let gradient = ctx.createLinearGradient(0, 0, 0, height);
            gradient.addColorStop(0.0, Qt.rgba(0.0, 0.0, 0.0, 0.0));
            gradient.addColorStop(0.7, Qt.rgba(0.5, 0.5, 0.5, 0.7));
            gradient.addColorStop(1.0, Qt.rgba(0.5, 0.5, 0.5, 0.9));
            ctx.fillStyle = gradient;
            ctx.fillRect(0, 0, width, height);
        }
    }

    RowLayout {
        anchors {
            left: parent.left
            bottom: parent.bottom
            leftMargin: 30
            bottomMargin: 40
        }
        spacing: 0

        ToolButton {
            id: closeButton
            objectName: "streamDisconnect"
            Layout.rightMargin: 20
            text: "×"
            padding: 10
            font.pixelSize: 50
            down: activeFocus
            onClicked: {
                menuController.close();
                if (backend.session)
                    streamWindow.close();
                else
                    navigation.showMainView();
            }
            KeyNavigation.right: volumeSlider
            Keys.onReturnPressed: clicked()
            Keys.onEscapePressed: menuController.close()
        }

        ToolSeparator {
            Layout.leftMargin: -10
            Layout.rightMargin: 10
        }

        Slider {
            id: volumeSlider
            objectName: "streamVolume"
            Layout.rightMargin: 20
            orientation: Qt.Vertical
            from: 0
            to: 128
            Layout.preferredHeight: 100
            padding: 10
            stepSize: 1
            value: settings.audioVolume
            onMoved: settings.audioVolume = value
            KeyNavigation.left: closeButton
            KeyNavigation.right: muteButton
            Keys.onEscapePressed: menuController.close()
            Label {
                anchors {
                    top: parent.bottom
                    horizontalCenter: parent.horizontalCenter
                    leftMargin: 10
                }
                text: {
                    ((parent.value / 128.0) * 100).toFixed(0) + qsTr("% Volume")
                }
            }
        }

        ToolSeparator {
            Layout.leftMargin: -10
            Layout.rightMargin: -10
        }

        ToolButton {
            id: muteButton
            objectName: "streamMicrophone"
            Layout.rightMargin: 20
            text: qsTr("Mic")
            padding: 10
            checkable: true
            enabled: backend.session && backend.session.connected
            checked: backend.session && !backend.session.muted
            onToggled: { if (backend.session && backend.session.connected) backend.session.muted = !backend.session.muted; }
            KeyNavigation.left: volumeSlider
            KeyNavigation.right: zoomButton
            Keys.onReturnPressed: toggled()
            Keys.onEscapePressed: menuController.close()
        }

        ToolButton {
            id: zoomButton
            objectName: "streamZoom"
            text: qsTr("Zoom")
            padding: 10
            checkable: true
            checked: streamWindow.videoMode == ChiakiWindow.VideoMode.Zoom
            onToggled: streamWindow.videoMode = streamWindow.videoMode == ChiakiWindow.VideoMode.Zoom ? ChiakiWindow.VideoMode.Normal : ChiakiWindow.VideoMode.Zoom
            KeyNavigation.left: muteButton
            KeyNavigation.right: {
                if(streamWindow.videoMode == ChiakiWindow.VideoMode.Zoom)
                    zoomFactor
                else
                    stretchButton
            }
            Keys.onReturnPressed: toggled()
            Keys.onEscapePressed: menuController.close()
        }

        Slider {
            id: zoomFactor
            objectName: "streamZoomFactor"
            orientation: Qt.Vertical
            from: -1
            to: 4
            Layout.preferredHeight: 100
            stepSize: 0.01
            visible: streamWindow.videoMode == ChiakiWindow.VideoMode.Zoom
            value: streamWindow.ZoomFactor
            onMoved: {
                streamWindow.ZoomFactor = value
                settings.sZoomFactor = value
            }
            Label {
                anchors {
                    top: parent.bottom
                    horizontalCenter: parent.horizontalCenter
                    leftMargin: 10
                }
                text: {
                    if(parent.value === -1)
                        qsTr("No Black Bars")
                    else if(parent.value >= 0)
                        qsTr((parent.value + 1).toFixed(2)) + qsTr(" x")
                    else
                        qsTr(parent.value.toFixed(2)) + qsTr(" x")
                }

            }
        }

        ToolSeparator {
            Layout.leftMargin: -10
            Layout.rightMargin: -10
        }

        ToolButton {
            id: stretchButton
            objectName: "streamStretch"
            Layout.rightMargin: 50
            text: qsTr("Stretch")
            padding: 10
            checkable: true
            checked: streamWindow.videoMode == ChiakiWindow.VideoMode.Stretch
            onToggled: streamWindow.videoMode = streamWindow.videoMode == ChiakiWindow.VideoMode.Stretch ? ChiakiWindow.VideoMode.Normal : ChiakiWindow.VideoMode.Stretch
            KeyNavigation.left: {
                if(streamWindow.videoMode == ChiakiWindow.VideoMode.Zoom)
                    zoomFactor
                else
                    zoomButton
            }
            KeyNavigation.right: defaultButton
            Keys.onReturnPressed: toggled()
            Keys.onEscapePressed: menuController.close()
        }

        ToolButton {
            id: defaultButton
            objectName: "streamDefault"
            text: qsTr("Default")
            padding: 10
            checkable: true
            checked: streamWindow.videoPreset == ChiakiWindow.VideoPreset.Default
            onToggled: {
                streamWindow.videoPreset = ChiakiWindow.VideoPreset.Default
                settings.videoPreset = ChiakiWindow.VideoPreset.Default
            }
            KeyNavigation.left: stretchButton
            KeyNavigation.right: highQualityButton
            Keys.onReturnPressed: toggled()
            Keys.onEscapePressed: menuController.close()
        }

        ToolSeparator {
            Layout.leftMargin: -10
            Layout.rightMargin: -10
        }

        ToolButton {
            id: highQualityButton
            objectName: "streamHighQuality"
            text: qsTr("High Quality")
            padding: 10
            checkable: true
            checked: streamWindow.videoPreset == ChiakiWindow.VideoPreset.HighQuality
            onToggled: {
                streamWindow.videoPreset = ChiakiWindow.VideoPreset.HighQuality
                settings.videoPreset = ChiakiWindow.VideoPreset.HighQuality
            }
            KeyNavigation.left: defaultButton
            KeyNavigation.right: highQualitySpatialButton
            Keys.onReturnPressed: toggled()
            Keys.onEscapePressed: menuController.close()
        }

        ToolButton {
            id: highQualitySpatialButton
            objectName: "streamSpatial"
            text: qsTr("HQ + Spatial")
            padding: 10
            checkable: true
            checked: streamWindow.videoPreset == ChiakiWindow.VideoPreset.HighQualitySpatial
            onToggled: {
                streamWindow.videoPreset = ChiakiWindow.VideoPreset.HighQualitySpatial
                settings.videoPreset = ChiakiWindow.VideoPreset.HighQualitySpatial
            }
            KeyNavigation.left: highQualityButton
            KeyNavigation.right: highQualityAdvancedSpatialButton
            Keys.onReturnPressed: toggled()
            Keys.onEscapePressed: menuController.close()
        }

        ToolButton {
            id: highQualityAdvancedSpatialButton
            objectName: "streamAdvancedSpatial"
            text: qsTr("HQ + Adv Spatial")
            padding: 10
            checkable: true
            checked: streamWindow.videoPreset == ChiakiWindow.VideoPreset.HighQualityAdvancedSpatial
            onToggled: {
                streamWindow.videoPreset = ChiakiWindow.VideoPreset.HighQualityAdvancedSpatial
                settings.videoPreset = ChiakiWindow.VideoPreset.HighQualityAdvancedSpatial
            }
            KeyNavigation.left: highQualitySpatialButton
            KeyNavigation.right: customButton
            Keys.onReturnPressed: toggled()
            Keys.onEscapePressed: menuController.close()
        }

        ToolSeparator {
            Layout.leftMargin: -10
            Layout.rightMargin: -10
        }

        ToolButton {
            id: customButton
            objectName: "streamCustom"
            text: qsTr("Custom")
            Layout.rightMargin: 40
            padding: 10
            checkable: true
            checked: streamWindow.videoPreset == ChiakiWindow.VideoPreset.Custom
            onToggled: {
                streamWindow.videoPreset = ChiakiWindow.VideoPreset.Custom
                settings.videoPreset = ChiakiWindow.VideoPreset.Custom
            }
            KeyNavigation.left: highQualityAdvancedSpatialButton
            KeyNavigation.right: displaySettingsButton
            Keys.onReturnPressed: toggled()
            Keys.onEscapePressed: menuController.close()
        }

        ToolButton {
            id: displaySettingsButton
            objectName: "streamDisplaySettings"
            text: qsTr("Display")
            padding: 10
            checkable: false
            icon.source: "qrc:/icons/settings-20px.svg";
            onClicked: { menuController.close(); navigation.openDisplaySettings(); }
            KeyNavigation.left: customButton
            KeyNavigation.right: {
                if(streamWindow.videoPreset == ChiakiWindow.VideoPreset.Custom)
                    placeboSettingsButton;
                else
                    displaySettingsButton;
            }
            Keys.onReturnPressed: clicked()
            Keys.onEscapePressed: menuController.close()
        }

        ToolButton {
            id: placeboSettingsButton
            objectName: "streamRendererSettings"
            text: qsTr("Placebo")
            icon.source: "qrc:/icons/settings-20px.svg";
            padding: 10
            checkable: false
            onClicked: { menuController.close(); navigation.openPlaceboSettings(); }
            KeyNavigation.left: displaySettingsButton
            visible: streamWindow.videoPreset == ChiakiWindow.VideoPreset.Custom
            Keys.onReturnPressed: clicked()
            Keys.onEscapePressed: menuController.close()
        }
    }

    Label {
        anchors {
            right: consoleNameLabel.right
            bottom: consoleNameLabel.top
            bottomMargin: 5

        }
        text: "Mbps"
        font.pixelSize: 18
        visible: backend.session

        Label {
            anchors {
                right: parent.left
                baseline: parent.baseline
                rightMargin: 5
            }
            text: visible && backend.session ? backend.session.measuredBitrate.toFixed(1) : ""
            color: Material.accent
            font.bold: true
            font.pixelSize: 28
        }
    }

    Label {
        id: consoleNameLabel
        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
            rightMargin: 30
        }
        text: {
            if (!backend.session)
                return "";
            if (backend.session.connected)
                return qsTr("Connected to <b>%1</b>").arg(settings.streamerMode ? "hidden" : backend.session.host);
            return qsTr("Connecting to <b>%1</b>").arg(settings.streamerMode ? "hidden" : backend.session.host);
        }

        RowLayout {
            anchors {
                right: parent.right
                top: parent.bottom
                topMargin: 12
            }

            Label {
                text: qsTr("packet loss")
                font.pixelSize: 15
                opacity: parent.visible && backend.session ? 1.0 : 0.0
                visible: opacity

                Behavior on opacity { NumberAnimation { duration: 250 } }

                Label {
                    anchors {
                        right: parent.left
                        baseline: parent.baseline
                        rightMargin: 5
                    }
                    text: visible ? "%1<font size=\"1\">%</font>".arg((((backend.session && isFinite(backend.session.averagePacketLoss)) ? backend.session.averagePacketLoss : 0) * 100).toFixed(1)) : ""
                    color: "#ef9a9a" // Material.Red
                    font.bold: true
                    font.pixelSize: 18
                }
            }

            Label {
                Layout.leftMargin: droppedFramesLabel.width + 6
                text: qsTr("dropped frames")
                font.pixelSize: 15
                opacity: parent.visible && streamWindow.droppedFrames ? 1.0 : 0.0
                visible: opacity

                Behavior on opacity { NumberAnimation { duration: 250 } }

                Label {
                    id: droppedFramesLabel
                    anchors {
                        right: parent.left
                        baseline: parent.baseline
                        rightMargin: 5
                    }
                    text: visible ? streamWindow.droppedFrames : ""
                    color: "#ef9a9a" // Material.Red
                    font.bold: true
                    font.pixelSize: 18
                }
            }
        }
    }
}
