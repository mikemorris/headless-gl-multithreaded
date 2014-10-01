#include "view.hpp"
#include <string>

View::View() {
#if USE_CGL
    // TODO: test if OpenGL 4.1 with GL_ARB_ES2_compatibility is supported
    // If it is, use kCGLOGLPVersion_3_2_Core and enable that extension.
    CGLPixelFormatAttribute attributes[] = {
        kCGLPFAOpenGLProfile,
        (CGLPixelFormatAttribute) kCGLOGLPVersion_Legacy,
        kCGLPFAAccelerated,
        (CGLPixelFormatAttribute) 0
    };

    CGLPixelFormatObj pixelFormat;
    GLint num;
    CGLError error = CGLChoosePixelFormat(attributes, &pixelFormat, &num);
    if (error) {
        fprintf(stderr, "Error pixel format: %s\n", CGLErrorString(error));
        return;
    }

    error = CGLCreateContext(pixelFormat, NULL, &gl_context);
    CGLDestroyPixelFormat(pixelFormat);
    if (error) {
        fprintf(stderr, "Error creating GL context object\n");
        return;
    }

    error = CGLEnable(gl_context, kCGLCEMPEngine);
    if (error != kCGLNoError ) {
        fprintf(stderr, "Error enabling OpenGL multithreading\n");
        return;
    }
#endif

#if USE_GLX
    x_display = XOpenDisplay(0);

    if (x_display == nullptr) {
        throw std::runtime_error("Failed to open X display");
    }

    static int pixelFormat[] = {
        GLX_RGBA,
        GLX_DOUBLEBUFFER,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        None
    };

    x_info = glXChooseVisual(x_display, DefaultScreen(x_display), pixelFormat);

    if (x_info == nullptr) {
        throw std::runtime_error("Error pixel format");
    }

    gl_context = glXCreateContext(x_display, x_info, 0, GL_TRUE);
    if (gl_context == nullptr) {
        throw std::runtime_error("Error creating GL context object");
    }
#endif
}

View::~View() {
    clear_buffers();

#if USE_CGL
    CGLDestroyContext(gl_context);
#endif

#if USE_GLX
    glXDestroyContext(x_display, gl_context);
    XFree(x_info);
    XCloseDisplay(x_display);
#endif
}

void View::resize(uint16_t width, uint16_t height, float pixelRatio) {
    clear_buffers();

    width *= pixelRatio;
    height *= pixelRatio;

#if USE_CGL
    make_active();

    // Create depth/stencil buffer
    glGenRenderbuffersEXT(1, &fbo_depth_stencil);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fbo_depth_stencil);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8_EXT, width, height);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

    glGenRenderbuffersEXT(1, &fbo_color);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, fbo_color);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, width, height);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

    glGenFramebuffersEXT(1, &fbo);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);

    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, fbo_color);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER_EXT, fbo_depth_stencil);

    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

    if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
        fprintf(stderr, "Couldn't create framebuffer: ");
        switch (status) {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT: fprintf(stderr, "incomplete attachment\n"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT: fprintf(stderr, "incomplete missing attachment\n"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT: fprintf(stderr, "incomplete dimensions\n"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT: fprintf(stderr, "incomplete formats\n"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT: fprintf(stderr, "incomplete draw buffer\n"); break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT: fprintf(stderr, "incomplete read buffer\n"); break;
            case GL_FRAMEBUFFER_UNSUPPORTED: fprintf(stderr, "unsupported\n"); break;
            default: fprintf(stderr, "other\n"); break;
        }
        return;
    }

    make_inactive();
#endif

#if USE_GLX
    x_pixmap = XCreatePixmap(x_display, DefaultRootWindow(x_display), width, height, 32);
    glx_pixmap = glXCreateGLXPixmap(x_display, x_info, x_pixmap);
#endif
}

void View::clear_buffers() {
#if USE_CGL
    make_active();

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    if (fbo) {
        glDeleteFramebuffersEXT(1, &fbo);
        fbo = 0;
    }

    if (fbo_color) {
        glDeleteTextures(1, &fbo_color);
        fbo_color = 0;
    }

    if (fbo_depth_stencil) {
        glDeleteRenderbuffersEXT(1, &fbo_depth_stencil);
        fbo_depth_stencil = 0;
    }

    make_inactive();
#endif

#if USE_GLX
    if (glx_pixmap) {
        glXDestroyGLXPixmap(x_display, glx_pixmap);
        glx_pixmap = 0;
    }

    if (x_pixmap) {
        XFreePixmap(x_display, x_pixmap);
        x_pixmap = 0;
    }
#endif
}

void View::make_active() {
#if USE_CGL
    CGLError error = CGLSetCurrentContext(gl_context);
    if (error) {
        fprintf(stderr, "Switching OpenGL context failed\n");
    }
#endif

#if USE_GLX
    if (!glXMakeCurrent(x_display, glx_pixmap, gl_context)) {
        fprintf(stderr, "Switching OpenGL context failed\n");
    }
#endif
}

void View::make_inactive() {
#if USE_CGL
    CGLError error = CGLSetCurrentContext(nullptr);
    if (error) {
        fprintf(stderr, "Removing OpenGL context failed\n");
    }
#endif

#if USE_GLX
    if (!glXMakeCurrent(x_display, None, NULL)) {
        fprintf(stderr, "Removing OpenGL context failed\n");
    }
#endif
}
