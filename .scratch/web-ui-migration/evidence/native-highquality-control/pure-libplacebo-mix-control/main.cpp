#include <QGuiApplication>
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QWindow>
#include <QElapsedTimer>
#include <QThread>
#include <QDebug>
#include <cstring>
#include <cstdlib>
#include <cstdio>
extern "C" {
#include <libavutil/frame.h>
#include <libplacebo/opengl.h>
#include <libplacebo/renderer.h>
#include <libplacebo/utils/libav.h>
}

struct Context { QOpenGLContext *gl; QWindow *window; unsigned char center[4]{}; };
static void require(bool condition, const char *message)
{
    if (!condition) { qCritical() << message; std::exit(2); }
}

int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QGuiApplication app(argc, argv);
    const bool noCompute = argc > 1 && QByteArray(argv[1]) == "no-compute";
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setRenderableType(QSurfaceFormat::OpenGL);
    format.setAlphaBufferSize(8);
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setSwapInterval(0);
    QOpenGLContext context;
    context.setFormat(format);
    if (auto *shared = QOpenGLContext::globalShareContext()) context.setShareContext(shared);
    require(context.create(), "create context");
    QWindow window;
    window.setFlag(Qt::WindowDoesNotAcceptFocus);
    window.setSurfaceType(QSurface::OpenGLSurface);
    window.setFormat(context.format());
    window.setGeometry(80, 80, 960, 640);
    window.show();
    QElapsedTimer exposed;
    exposed.start();
    while (!window.isExposed() && exposed.elapsed() < 3000) { app.processEvents(); QThread::msleep(10); }
    require(window.isExposed(), "exposed native surface");
    require(context.makeCurrent(&window), "make current");
    auto *functions = context.extraFunctions();
    qInfo() << "PURE device"
        << reinterpret_cast<const char *>(functions->glGetString(GL_VENDOR))
        << reinterpret_cast<const char *>(functions->glGetString(GL_RENDERER))
        << reinterpret_cast<const char *>(functions->glGetString(GL_VERSION));
    std::printf("PURE device %s | %s | %s\n", functions->glGetString(GL_VENDOR),
        functions->glGetString(GL_RENDERER), functions->glGetString(GL_VERSION));
    Context state{&context, &window};
    pl_opengl_params gpuParams{};
    gpuParams.get_proc_addr_ex = [](void *data, const char *name) -> pl_voidfunc_t {
        return reinterpret_cast<pl_voidfunc_t>(static_cast<Context *>(data)->gl->getProcAddress(name));
    };
    gpuParams.proc_ctx = &state;
    gpuParams.allow_software = true;
    gpuParams.no_compute = noCompute;
    gpuParams.make_current = [](void *data) -> bool {
        auto *value = static_cast<Context *>(data);
        return value->gl->makeCurrent(value->window);
    };
    gpuParams.release_current = [](void *data) { static_cast<Context *>(data)->gl->doneCurrent(); };
    gpuParams.priv = &state;
    auto gpu = pl_opengl_create(nullptr, &gpuParams);
    require(gpu, "create libplacebo GPU");
    pl_opengl_swapchain_params swapParams{};
    swapParams.swap_buffers = [](void *data) {
        auto *value = static_cast<Context *>(data);
        auto *gl = value->gl->extraFunctions();
        gl->glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        gl->glReadBuffer(GL_BACK);
        unsigned char rgba[4]{};
        gl->glReadPixels(qRound(value->window->width() * value->window->devicePixelRatio()) / 2,
            qRound(value->window->height() * value->window->devicePixelRatio()) / 2,
            1, 1, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
        qInfo() << "PURE native center" << QByteArray(reinterpret_cast<char *>(rgba), 4).toHex()
            << "GL error" << gl->glGetError();
        std::memcpy(value->center, rgba, 4);
        std::printf("PURE native center %u %u %u %u\n", rgba[0], rgba[1], rgba[2], rgba[3]);
        value->gl->swapBuffers(value->window);
    };
    swapParams.priv = &state;
    auto swap = pl_opengl_create_swapchain(gpu, &swapParams);
    require(swap, "create swapchain");
    int width = qRound(window.width() * window.devicePixelRatio());
    int height = qRound(window.height() * window.devicePixelRatio());
    require(pl_swapchain_resize(swap, &width, &height), "resize swapchain");
    auto renderer = pl_renderer_create(nullptr, gpu->gpu);
    require(renderer, "create renderer");
    AVFrame *frame = av_frame_alloc();
    require(frame, "allocate frame");
    frame->format = AV_PIX_FMT_YUV420P;
    frame->width = 320;
    frame->height = 180;
    frame->colorspace = AVCOL_SPC_SMPTE170M;
    frame->color_range = AVCOL_RANGE_MPEG;
    require(av_frame_get_buffer(frame, 32) >= 0, "allocate frame data");
    for (int plane = 0; plane < 3; ++plane)
        for (int y = 0; y < (plane ? 90 : 180); ++y)
            std::memset(frame->data[plane] + y * frame->linesize[plane], plane == 0 ? 81 : plane == 1 ? 90 : 240, plane ? 160 : 320);
    pl_tex textures[4]{};
    pl_frame source{};
    pl_avframe_params sourceParams{};
    sourceParams.frame = frame;
    sourceParams.tex = textures;
    require(pl_map_avframe_ex(gpu->gpu, &source, &sourceParams), "map frame");
    qInfo() << "PURE no_compute" << noCompute << "size" << width << height
        << "format" << context.format();
    for (int i = 0; i < 2; ++i) {
        pl_swapchain_frame output{};
        require(pl_swapchain_start_frame(swap, &output), "start frame");
        pl_frame target{};
        pl_frame_from_swapchain(&target, &output);
        pl_rect2df_aspect_copy(&target.crop, &source.crop, 0.0);
        const pl_frame *frames[] = {&source};
        const uint64_t signatures[] = {static_cast<uint64_t>(i)};
        const float timestamps[] = {0.0f};
        pl_frame_mix mix{};
        mix.num_frames = 1;
        mix.frames = frames;
        mix.signatures = signatures;
        mix.timestamps = timestamps;
        mix.vsync_duration = 1.0f;
        require(pl_render_image_mix(renderer, &mix, &target, &pl_render_high_quality_params), "render HighQuality mix");
        require(pl_swapchain_submit_frame(swap), "submit frame");
        pl_swapchain_swap_buffers(swap);
    }
    pl_gpu_finish(gpu->gpu);
    pl_unmap_avframe(gpu->gpu, &source);
    for (auto &texture : textures) if (texture) pl_tex_destroy(gpu->gpu, &texture);
    av_frame_free(&frame);
    pl_renderer_destroy(&renderer);
    pl_swapchain_destroy(&swap);
    pl_opengl_destroy(&gpu);
    const bool red = state.center[0] > 200 && state.center[1] < 30
        && state.center[2] < 30 && state.center[3] > 240;
    std::printf("PURE no_compute=%d known_red=%d\n", noCompute, red);
    return red ? 0 : 3;
}
