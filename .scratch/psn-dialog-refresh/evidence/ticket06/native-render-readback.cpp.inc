
QImage QmlMainWindow::nativeQaImage() {
    qApp->setProperty("qaQuickTexture",quick_tex!=nullptr);
    qApp->setProperty("qaSwapchain",placebo_swapchain!=nullptr);
    if(render_backend==RenderBackend::OpenGL) {
        if(!quick_fbo || !makeOpenGLContextCurrent()) return {};
        const auto image=quick_fbo->toImage(); doneOpenGLContextCurrent(); return image;
    }
    QImage image(swapchain_size,QImage::Format_RGBA8888);
    auto download=[&] {
        endFrame();
        if(!quick_tex) { image={}; return; }
        pl_tex_transfer_params params{}; params.tex=quick_tex; params.row_pitch=image.bytesPerLine(); params.ptr=image.bits();
        if(!pl_tex_download(placeboGpu(),&params)) image={};
    };
    if(quick_render->thread()==QThread::currentThread()) download();
    else QMetaObject::invokeMethod(quick_render,download,Qt::BlockingQueuedConnection);
    return image;
}
