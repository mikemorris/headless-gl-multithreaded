#include <stdint.h>

// Mac
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#define USE_CGL 1

/*
// Linux
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#define USE_GLX 1
*/

class View {
public:
    View();
    ~View();

    void resize(uint16_t width, uint16_t height, float pixelRatio);
    void make_active();
    void make_inactive();

private:
    void clear_buffers();

private:
#if USE_CGL
    CGLContextObj gl_context;
    GLuint fbo = 0;
    GLuint fbo_depth_stencil = 0;
    GLuint fbo_color = 0;
#endif

#if USE_GLX
    GLXContext gl_context = nullptr;
    XVisualInfo *x_info = nullptr;
    Display *x_display = nullptr;
    Pixmap x_pixmap = 0;
    GLXPixmap glx_pixmap = 0;
#endif
};
