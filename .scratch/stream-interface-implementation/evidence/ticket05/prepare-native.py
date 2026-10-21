"""Extend ticket 02's real native runner; all changes stay in its saved QA copy."""
from pathlib import Path
import subprocess, sys

here = Path(__file__).resolve().parent
repo = here.parents[3]
cache = Path('C:/Users/david/.cache/chiaki-ui-native')
name = sys.argv[1]
subprocess.run([sys.executable, str(here.parent / 'ticket02/prepare-native.py'), name], cwd=repo, check=True)
def edit(path, old, new):
    target = cache / 'source' / path
    value = target.read_text()
    assert old in value, path + ': ' + old
    target.write_text(value.replace(old, new))
edit('gui/include/qmlmainwindow.h', 'int nativeQaGrabCount() const { return grab_input; }', '''int nativeQaGrabCount() const { return grab_input; }
    void nativeQaVideoReady() { has_video = true; emit hasVideoChanged(); }
    void nativeQaSeedStatistics() { dropped_frames = 2147483647; queue_depth_average = 123456.7; updatePendingFrameAge(12.345); emit droppedFramesChanged(); emit queueDepthAverageChanged(); }
''')
edit('gui/include/streamsession.h', 'bool nativeQaInputBlocked() const { return input_block == 1; }', '''bool nativeQaInputBlocked() const { return input_block == 1; }
        void nativeQaMetrics() { measured_bitrate=123456789; session.stream_connection.congestion_control.packet_loss=0.057; packet_loss_history.clear(); average_packet_loss=0.057; frames_lost=2147483647; emit MeasuredBitrateChanged(); emit AveragePacketLossChanged(); emit FramesLostChanged(); }
        void nativeQaBlocked(bool value) { CantDisplayMessage(value); }
''')
for method, body in [('GoHome', 'qApp->setProperty("qaHome",qApp->property("qaHome").toInt()+1); return;'), ('ToggleMute', 'qApp->setProperty("qaMute",qApp->property("qaMute").toInt()+1); muted=!muted; emit MutedChanged(); return;'), ('SetLoginPIN', 'qApp->setProperty("qaPin",qApp->property("qaPin").toInt()+1); return;')]:
    signature = 'void StreamSession::' + method + ('(const QString &pin)' if method == 'SetLoginPIN' else '()') + '\n{'
    edit('gui/src/streamsession.cpp', signature, signature + '\n    ' + body)
edit('gui/include/qmlbackend.h', 'Settings *nativeQaSettings() { return settings; }', '''Settings *nativeQaSettings() { return settings; }
    StreamSession *nativeQaReplace(bool blocked) {
        auto *previous=session; session=new StreamSession(session_info,this);
        ChiakiEvent connected{}; connected.type=CHIAKI_EVENT_CONNECTED; session->nativeQaEvent(&connected);
        session->nativeQaBlocked(blocked); emit sessionChanged(session); previous->deleteLater(); return session;
    }
    void nativeQaEndReplacement() { auto *previous=session; session=nullptr; emit sessionChanged(nullptr); previous->deleteLater(); }
''')
# A changing known background proves that the desktop capture contains this live app.
edit('gui/src/qml/StreamView.qml', 'id: view\n', '''id: view
    property int qaBackground: 0
    Rectangle { anchors.fill: parent; z: -10; color: view.qaBackground === 1 ? "#f0e4c8" : "#101824"
        Repeater { model: view.qaBackground === 2 ? 36 : 0
            Rectangle { required property int index; x: (index % 6)*parent.width/6; y: Math.floor(index/6)*parent.height/6; width: parent.width/6; height: parent.height/6; color: index % 2 ? "#c3e2b8" : "#305178" }
        }
    }
    Rectangle { x: 10; y: 10; width: 32; height: 32; color: view.qaBackground === 1 ? "#00cc77" : "#ff00ff" }
''')
target = cache / name / 'native-disconnect.cpp'
text = target.read_text()
text = text.replace('candidate->objectName()!="streamMenuWindow" || candidate->isVisible()', '!candidate->objectName().startsWith("stream") || candidate->isVisible()')
text = text.replace('if (item->window()->objectName()=="streamMenuWindow" || item->window()->flags().testFlag(Qt::Popup)) return item->window();', 'if (item->window()->isVisible()) return item->window();')
text = text.replace('GetCursorPos(&previous); SetCursorPos(point.x,point.y); INPUT', 'GetCursorPos(&previous); check(SetCursorPos(point.x,point.y),"Native pointer movement is available"); INPUT')
text = text.replace('event(session,CHIAKI_EVENT_CONNECTED);\n', 'event(session,CHIAKI_EVENT_CONNECTED); window->nativeQaVideoReady(); waitUi();\n')
text = text.replace('#include <QTimer>', '#include <QTimer>\n#include <QProcess>\n#include <QTranslator>\n#include <QQmlEngine>\n#include <qpa/qwindowsysteminterface.h>')
text = text.replace('void contracts() {', Path(__file__).with_name('native-acceptance.cpp.inc').read_text() + '\nvoid toolbarContracts() {')
text = text.replace('QTimer::singleShot(500,qApp,contracts);', 'QTimer::singleShot(500,qApp,qEnvironmentVariableIsSet("CHIAKI_QA_BASELINE")?toolbarContracts:acceptanceContracts);')
text = text.replace('settings.SetAudioVideoDisabled(CHIAKI_AUDIO_VIDEO_DISABLED);', 'settings.SetAudioVideoDisabled(CHIAKI_AUDIO_DISABLED); settings.SetShowStreamStats(true);')
target.write_text(text)
print('Prepared integrated native acceptance, transport isolation, and desktop capture checks')
