# Minimal native HighQuality mix control

This control changes only the earlier pure native control's render call:
pl_render_image becomes pl_render_image_mix. Each of the two inputs has a
new signature, timestamp zero, one source frame, and normalized vsync one.
The uploaded YUV420P image is constant red. No Qt Quick, QRhi, WebEngine,
WebChannel, screen capture, focus request, or production source is involved.

- Compute enabled: both readbacks are 0/0/0/0. The known-red self-check fails,
  exit 3.
- Compute disabled: both readbacks are 254/0/0/255. The self-check passes,
  exit 0.

Both processes identify NVIDIA RTX 5090, OpenGL 4.6.0, driver 617.42. They
use the same Qt 6.9.3, libplacebo 7.360.1 and FFmpeg 7.1 staging dependencies.
Executable SHA256:
E49C54DEC3581C6A50A5CD75428A593D08194B8AB4B098CB7D20D74BD2AA1B75

This is a reproducible native control failure independent of the web UI
and the imported Qt Quick device. It is not a full pre-migration application
baseline or a live console test. It does not establish which library,
driver, shader or cache operation is defective. Keep the user's preset and
production defaults unchanged. Full HighQuality acceptance remains open.

Exact source, build command/log, run script, raw output and exit results are
in this directory. The direct-render comparison remains in the adjacent
pure-libplacebo-control directory. Both binaries are separate from the
passing ticket03 graphics harness.
