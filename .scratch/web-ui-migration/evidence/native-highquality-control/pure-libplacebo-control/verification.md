# HighQuality native control

This diagnostic uses QGuiApplication, a native QWindow and QOpenGLContext,
libplacebo, and the existing FFmpeg texture mapping helper. It has no Qt Quick,
QRhi, WebEngine, or WebChannel. It does not activate the window or inspect the
screen. The production sources and presets are unchanged.

The same executable rendered a known red 320 by 180 YUV420P frame twice with
HighQuality parameters. A second process repeated this with no_compute set.
The log identifies NVIDIA RTX 5090, OpenGL 4.6.0, driver 617.42.

- Compute enabled: first center RGBA 0/0/0/0; second 253/0/0/255.
- Compute disabled: first 254/0/0/255; second 253/0/0/255.
- Both final diagnostic processes exit 0 after asserting that the second
  render is opaque red. See compute-control.txt and no-compute.txt.

The first program version did not capture its Qt log output on Windows.
Its exit codes are retained in initial-api-only-results.json. They do not
establish pixel output. The final program uses explicit output and a color
self-check; results.json records those processes.

Executable SHA256:
80AF32CEA1FD3DF9C68BAACFBEEB77F6CC00F975F624F13B374A0FA9D580BBDF

This proves that the installed native compute path can render HighQuality
after its first render. It does not prove streaming performance, Qt Quick
coexistence, or production first-frame behavior. The next check must separate
a single-frame fixture/cache effect from a production rendering fault.
