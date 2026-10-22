# Continuous native HighQuality diagnostic

The controlled native input submitted 72 red YUV420P frames over 1250 ms.
An event timer supplies current elapsed timestamps. The production frame
queue advanced its observed signature from 0 to 47 by 766 ms.

At 766 ms the native center was ff/00/ff/ff instead of red. The mapped
source texture values were still 81/90/240, and the Quick texture center
was transparent 0/0/0/0. The blue notice and readback context/surface were
correct; read/restore GL errors were zero.

This excludes a held-single-frame-only explanation. It does not locate
the compute, state, or cached-intermediate defect. This is a native-only
window using the imported QRhi; no WebEngine page was created. It does
not request focus, inspect the screen, or change production presets.

The process deliberately ends with QFAIL, exit 1, after collecting output.
See the exact source patch, build log, run script, result hash, and raw
native.txt. Tracked source was restored afterward; the cache executable
remains diagnostic-only. The passing staged graphics harness is separate.
