import QtQuick
import org.streetpea.chiaking

ConnectionView {
    objectName: "autoConnectView"
    managesAttempt: attempt.waitingForWake === true
}
