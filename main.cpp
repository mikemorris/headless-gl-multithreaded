#include "view.hpp"

#include <memory>
#include <stdexcept>

int main() {
    uint16_t width = 400;
    uint16_t height = 300;
    float pixelRatio = 1.0;

    View view;
    view.resize(width, height, pixelRatio);

    const unsigned int w = width * pixelRatio;
    const unsigned int h = height * pixelRatio;

    const std::unique_ptr<uint32_t[]> pixels(new uint32_t[w * h]);

    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.get());

    return 0;
}
