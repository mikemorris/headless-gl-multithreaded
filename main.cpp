#include "view.hpp"

#include <memory>
#include <thread>

void readPixels() {
    uint16_t width = 400;
    uint16_t height = 300;
    float pixelRatio = 1.0;

    View view;
    view.resize(width, height, pixelRatio);

    const unsigned int w = width * pixelRatio;
    const unsigned int h = height * pixelRatio;

    const std::unique_ptr<uint32_t[]> pixels(new uint32_t[w * h]);

    view.make_active();
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.get());
    view.make_inactive();
}

int main() {
#if USE_GLX
    XInitThreads();
#endif

    while (true) {
        std::thread t1(readPixels);
        std::thread t2(readPixels);
        std::thread t3(readPixels);

        t1.join();
        t2.join();
        t3.join();
    }

    return 0;
}
