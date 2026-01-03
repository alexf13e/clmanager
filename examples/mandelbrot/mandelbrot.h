
#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <cstdint>

#include "glm/glm.hpp"


namespace Mandelbrot
{
    void acquireGLObjects();
    void releaseGLObjects();
    void onResize(uint32_t width, uint32_t height);
	float getCamZoom();
	void updateCam(const glm::vec2& deltaPos, const float deltaZoom);
    void resetCam();
	void createIterationBuffer();
    void updateSuperSampling(int delta);
    void updateMaxIterations(float delta);
    void requestUpdate();
    bool init(uint32_t width, uint32_t height, uint32_t superSampling=1);
    void update();
    bool draw();
}

#endif //MANDELBROT_H