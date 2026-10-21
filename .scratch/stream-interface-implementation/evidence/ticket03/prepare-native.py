"""Use the established real backend, isolated INI files, and stopped transport."""
from pathlib import Path
import subprocess
import sys

repo = Path(__file__).resolve().parents[4]
cache = Path("C:/Users/david/.cache/chiaki-ui-native")
name = sys.argv[1]
subprocess.run([sys.executable, str(repo / ".scratch/psn-dialog-refresh/evidence/ticket06/prepare-native.py"), name], cwd=repo, check=True)
header = cache / "source/gui/include/qmlmainwindow.h"
text = header.read_text()
old = "int nativeQaGrabCount() const { return grab_input; }"
assert old in text
text = text.replace(old, old + """
    void nativeQaVideoReady() { has_video = true; emit hasVideoChanged(); }
    void nativeQaSeedStatistics() { dropped_frames = 27; dropped_frames_current.storeRelaxed(9); queue_depth_average = 3.2; updatePendingFrameAge(0.015); emit droppedFramesChanged(); emit queueDepthAverageChanged(); }
""")
header.write_text(text)
target = cache / name / "native-disconnect.cpp"
text = target.read_text()
start = text.index("void contracts() {")
end = text.index("\n}\n}\nvoid initializeDisconnectQa", start)
text = text[:start] + Path(__file__).with_name("native-statistics.cpp.inc").read_text() + text[end:]
text = text.replace("settings.SetAudioVideoDisabled(CHIAKI_AUDIO_VIDEO_DISABLED);", "settings.SetAudioVideoDisabled(CHIAKI_AUDIO_DISABLED); settings.SetShowStreamStats(true);")
target.write_text(text)
print("Prepared native statistics assertions through real StreamView and real session replacement")
