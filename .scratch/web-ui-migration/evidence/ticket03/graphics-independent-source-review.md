# Independent native graphics review

2026-10-22. Read-only source review. No builds, test runs, GUI actions, or repository changes.

The current evidence localizes black video before presentation. The native GL_BACK read is black at the video center, while the production blue notice renders. Existing mapped Y/U/V textures contain 81/90/240, and the overlay center is transparent. These facts do not identify QRhi import or swapBuffers as the cause.

The sampler-object hypothesis is unsupported by exact Qt 6.9.3. QRhi GLES bindCombinedSampler binds a texture, assigns the sampler uniform, and sets glTexParameteri. It does not bind OpenGL sampler objects. The full source contains no glBindSampler or glSamplerParameter calls. See https://github.com/qt/qtbase/blob/v6.9.3/src/gui/rhi/qrhigles2.cpp#L3894 . Libplacebo src/opengl/gpu_pass.c:450 likewise binds textures and sets their parameters.

Qt releases its VAO at frame end. Libplacebo gl_pass_run explicitly binds its own program, descriptors, target, viewport and VAO. Its make_current callback runs at each outer acquisition. Therefore neither the current-source review nor the supplied observations establish a stale target, context, or VAO cache as the cause. The public Quick reset omits some GL state, but the negative reset comparison and working blue overlay do not support another blanket state-reset experiment.

The cheapest next comparison is one diagnostic direct pl_render_image call in place of pl_render_image_mix, using the same sole mapped frame, target_frame and params. Keep the crop, preset, overlay and target unchanged. Reuse the existing valid GL_BACK read. This is the same fallback call that libplacebo already makes at src/renderer.c:4059-4061.

The distinction is useful because image_mix creates and then reuses a converted intermediate texture by frame signature, src/renderer.c:3800-3950. It also adds a shader to sample that texture, lines 3995-4026. Direct pl_render_image runs conversion and scaling without that cache and extra mix shader. If red appears, inspect the mix/cache branch. If black remains, inspect the common conversion/scaling/output branch. This is a diagnostic comparison, not a production fix. Setting skip_caching_single_frame after an entry already exists is a weaker comparison because the existing matching cache can still be reused.

This agrees with the independent fixture/color reviewer. Only ticket03 is authorized to run the comparison. No further rendering change is justified by this source review alone.
