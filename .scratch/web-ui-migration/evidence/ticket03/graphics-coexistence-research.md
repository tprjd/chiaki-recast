# Qt WebEngine and native playback graphics

Date: 2026-10-22. Source review only. No builds, tests, or repository edits.

The inspected Qt version is **6.9.3**, matched to the installed Windows MSVC SDK. The source supports a small same-process integration. It does not prove that a specific GPU and driver will pass. There is no evidence here that requires a separate application process or a streaming backend change.

## Confirmed constraints

1. **QWebEngineView uses Qt Quick internally, but it does not require OpenGL.** Its internal `WebEngineQuickWidget` derives from `QQuickWidget`. Qt 6.9.3 WebEngine accepts OpenGL, Vulkan, Metal, and D3D11. The Windows Vulkan path imports a Chromium D3D11 texture into a Vulkan image. Thus, Chromium and Qt Quick do not have to use the same native GPU API. The installed SDK has `QT_FEATURE_webengine_vulkan 1` in `C:/Users/david/.cache/chiaki-native-qt/6.9.3/msvc2022_64/include/QtWebEngineCore/6.9.3/QtWebEngineCore/private/qtwebenginecore-config_p.h:27`.

   Primary source: [internal widget, lines 83–103](https://github.com/qt/qtwebengine/blob/v6.9.3/src/webenginewidgets/api/qwebengineview.cpp#L83), [accepted APIs, lines 356–362](https://github.com/qt/qtwebengine/blob/v6.9.3/src/core/web_engine_context.cpp#L356), [Windows Vulkan import, lines 186–207 and 258–264](https://github.com/qt/qtwebengine/blob/v6.9.3/src/core/compositor/native_skia_output_device_vulkan.cpp#L186).

2. **The native constructor must stop changing the process-wide Qt Quick API after setup starts.** `QQuickWindow::setGraphicsApi()` must precede the first Quick window. Its RenderControl exception requires destruction of all existing Quick windows and render controls. Hiding setup does not meet this condition. More strongly, WebEngine caches the selected API in a function-static value. Recreating only its view does not reset that cache.

   Current integration source: `C:/Users/david/.codex/worktrees/web-ui-integration/chiaki-ng-ui/gui/src/qmlmainwindow.cpp:1783`. Ticket 01 still has the call at `C:/Users/david/.codex/worktrees/web-ui-ticket-01/chiaki-ng-ui/gui/src/qmlmainwindow.cpp:1799`. Its following `fromDeviceObjects` or `fromOpenGLContext` call imports a device into the globally chosen backend; it does not select an independent API.

   Primary source: [selection rule, lines 4130–4137](https://github.com/qt/qtdeclarative/blob/v6.9.3/src/quick/items/qquickwindow.cpp#L4130), [WebEngine cache, line 40](https://github.com/qt/qtwebengine/blob/v6.9.3/src/core/compositor/display_overrides.cpp#L40), [global backend and device import, lines 1154–1207](https://github.com/qt/qtdeclarative/blob/v6.9.3/src/quick/scenegraph/qsgrhisupport.cpp#L1154).

3. **A common Vulkan API is not a cross-platform answer.** The Qt 6.9.3 WebEngine compositor explicitly rejects Vulkan on macOS. Locking the global API to native Vulkan also prevents the existing later Vulkan-to-OpenGL fallback while setup remains alive. A common API chosen before the first window is useful as a Windows/Linux probe, but it does not meet all migration requirements by itself.

   Primary source: [macOS Vulkan rejection, lines 54–63](https://github.com/qt/qtwebengine/blob/v6.9.3/src/core/compositor/display_overrides.cpp#L54). Existing fallback: ticket 01 `gui/src/qmlmainwindow.cpp:1615–1627`; the test trigger is at 1636–1639.

## Smallest per-window path to check

Keep a fixed supported Qt Quick API for WebEngine and ordinary Quick windows. Use the platform default or an explicit startup choice; do not change it in each playback constructor. Create an application-owned `QRhi` for each native render-control window, using that window's actual libplacebo backend. Supply it through `QQuickGraphicsDevice::fromRhi()` before `QQuickRenderControl::initialize()`.

The exact import APIs exist in the installed SDK:

- Vulkan: `QRhiVulkanInitParams::inst = qt_vk_inst`. Fill `QRhiVulkanNativeHandles::physDev`, `dev`, `gfxQueueFamilyIdx`, and `gfxQueueIdx` from the existing libplacebo device and graphics queue. Call `QRhi::create(QRhi::Vulkan, &params, flags, &handles)`. Keep the imported `QVulkanInstance` on the native Quick window. Do not create a second independent Vulkan device for the overlay texture.
- OpenGL: set `QRhiGles2InitParams::format` and `fallbackSurface` to the existing compatible format and offscreen surface. Set `QRhiGles2NativeHandles::context = qt_gl_context`. Call `QRhi::create(QRhi::OpenGLES2, &params, flags, &handles)`. `OpenGLES2` is the QRhi backend name used for this OpenGL path; it does not require replacement of the existing desktop OpenGL renderer.
- Pass the resulting pointer with `quick_window->setGraphicsDevice(QQuickGraphicsDevice::fromRhi(native_rhi))`. Check both the QRhi creation result and the boolean result of `quick_render->initialize()`.

Headers: `C:/Users/david/.cache/chiaki-native-qt/6.9.3/msvc2022_64/include/QtGui/6.9.3/QtGui/rhi/qrhi_platform.h:50–95`; sibling `qrhi.h:1947–1950`; `include/QtQuick/qquickgraphicsdevice.h:52`. The project currently finds and links `Qt::GuiPrivate` only for Qt >= 6.10. The Qt 6.9.3 import path must require the matching GuiPrivate module as well.

This is a concrete source path: [createRhi, lines 1111–1121](https://github.com/qt/qtdeclarative/blob/v6.9.3/src/quick/scenegraph/qsgrhisupport.cpp#L1111) returns the supplied QRhi with `own=false`, before global backend selection. [Render-control initialization, lines 303–316](https://github.com/qt/qtdeclarative/blob/v6.9.3/src/quick/items/qquickrendercontrol.cpp#L303) gives that QRhi to the window and scene graph. The existing `fromOpenGLTexture` and `fromVulkanImage` render targets use API-specific format conversion, then [resolve against the supplied QRhi](https://github.com/qt/qtdeclarative/blob/v6.9.3/src/quick/items/qquickrendertarget.cpp#L1659). Current native targets are at ticket 01 `gui/src/qmlmainwindow.cpp:2245` and `:2266`.

This permits a native Vulkan or OpenGL QRhi to be selected without changing WebEngine's global choice. It is the smallest candidate that preserves native fallback and the macOS requirement. It still needs a real combined probe because Qt retains the following global assumptions:

- `rendererInterface()->graphicsApi()` returns the global API, not the imported QRhi backend. Native-resource queries in `getResource()` also dispatch by the global API and can interpret imported handles incorrectly. `RhiResource` returns the actual QRhi before that dispatch. For the probe, read `QQuickRenderControl::rhi()->backend()` on its owning thread. See [graphicsApi](https://github.com/qt/qtdeclarative/blob/v6.9.3/src/quick/scenegraph/qsgdefaultcontext.cpp#L251) and [resource dispatch, lines 804–831](https://github.com/qt/qtdeclarative/blob/v6.9.3/src/quick/scenegraph/qsgrhisupport.cpp#L804).
- Native QSG texture import uses a format conversion selected from the global API. See [QSGPlainTexture, line 85](https://github.com/qt/qtdeclarative/blob/v6.9.3/src/quick/scenegraph/util/qsgplaintexture.cpp#L85). This differs from the existing native render-target path described above. No `rendererInterface` calls or native QSG texture imports were found in the current `gui/` sources. Exercise the real QML overlays to cover Qt-internal consumers.
- RenderControl checks for a window Vulkan instance when the *global* API is Vulkan, even with an imported QRhi. Therefore a global Vulkan setup choice is a poor default for a design that must import an OpenGL fallback. A fixed D3D11/Metal/OpenGL setup choice avoids that particular check. See [initRhi, lines 766–780](https://github.com/qt/qtdeclarative/blob/v6.9.3/src/quick/items/qquickrendercontrol.cpp#L766).

## Lifetime and ordering

Set `AA_ShareOpenGLContexts` before `QApplication`, as the existing startup does. If `QtWebEngineQuick::initialize()` remains in use, retain its pre-application call. Remove the unconditional `--disable-gpu` assignment at integration `gui/src/main.cpp:91`. Make the fixed setup API choice before any Quick/WebEngine window or WebEngine context use. Qt's initialization does not force Vulkan, Metal, or D3D11 back to OpenGL: [QtWebEngineQuick initialization, lines 38–55](https://github.com/qt/qtwebengine/blob/v6.9.3/src/webenginequick/api/qtwebenginequickglobal.cpp#L38).

Create, use, and destroy the native QRhi on its one owning render thread. In the existing OpenGL path this is the GUI thread; the Vulkan path uses a dedicated thread. Prepare the render control for that thread before initialization. `QQuickWidget` changes Qt's automatic render-loop behavior; it does not remove the explicit `prepareThread` mechanism. Preserve native queue synchronization and context-current rules.

The application owns the new QRhi. It does not own the imported Vulkan device or OpenGL context through that QRhi. Stop frame work, finish render-control invalidation and release all scene-graph/render-target resources while the QRhi and imported device remain valid. Delete the QRhi on its owning thread before that thread exits and before libplacebo/device/context destruction. Do not leave a live window or render control with a dangling imported QRhi pointer. The existing ticket 01 destructor stops its Vulkan thread at lines 447–449; a new QRhi therefore needs cleanup within the render-thread shutdown step, not a later GUI-thread deletion.

Primary source: [QRhi single-thread rule](https://github.com/qt/qtbase/blob/v6.9.3/src/gui/rhi/qrhi.cpp#L295), [Vulkan import and no ownership transfer](https://github.com/qt/qtbase/blob/v6.9.3/src/gui/rhi/qrhivulkan.cpp#L144), [OpenGL import and ownership](https://github.com/qt/qtbase/blob/v6.9.3/src/gui/rhi/qrhigles2.cpp#L96), [RenderControl destruction](https://github.com/qt/qtdeclarative/blob/v6.9.3/src/quick/items/qquickrendercontrol.cpp#L158). QRhi itself provides no fallback; handle a null result before exposing the stream window.

## Bounded combined runtime probe

Use the full native MSVC Qt 6.9.3 application build once available. Keep a real accelerated QWebEngineView alive through this sequence:

1. Render and interact with setup. Record Qt and Chromium graphics diagnostics, actual adapters, and hardware acceleration state. A GPU process or a non-software Qt API alone is not proof that Chromium composites on hardware. The Qt 6.9 [hardware acceleration documentation](https://doc.qt.io/archives/qt-6.9/qtwebengine-features.html#hardware-acceleration) identifies `chrome://gpu` as a diagnostic.
2. Create native Vulkan playback with the imported QRhi, render actual decoded video and real overlays, resize, stop, clean up, and restore setup. Check the actual native QRhi backend and imported device identity. Check HDR on a supported display as part of native acceptance.
3. Recreate playback once to check cleanup and retained WebEngine state. Exercise native OpenGL and forced Vulkan fallback with the same fixed setup API. The fallback check has a specific purpose: detect a late global API change or stale Vulkan resources. It is not a reason for a blanket 20-cycle run.
4. Confirm there is no blank web view, failed texture import, invalid native-handle cast, device loss, render-thread violation, or loss of setup input after return. Use diagnostics to identify a failure before broadening tests.

Reuse the result until graphics selection, device ownership, cleanup, Qt version, or driver changes. The Windows result does not certify macOS or Linux. Repeat the combined check on a real GPU for each supported platform and its actual Qt build. WSL llvmpipe can check code paths but cannot pass GPU acceptance. The GNU Qt 6.11.2 environment without WebEngine cannot check coexistence. Linux Qt 6.10.2 and macOS binaries were not source-audited in this review; do not treat the exact 6.9.3 implementation details as a guarantee for them.
