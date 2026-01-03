
#include <iostream>
#include <thread>
#include <unordered_map>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include <glm/gtx/norm.hpp>

#define CL_MANAGER_GL
#include "CLManager.h"
#include "kernels.h"

#include "mandelbrot.h"
#include "Key.h"



GLFWwindow* window;
float frameDuration; //seconds
uint32_t desiredFrameDuration = 16666; //microseconds

int windowWidth, windowHeight;
float moveMult;

static std::unordered_map<int, Key> keyMap = {
	{GLFW_KEY_A, Key()},
	{GLFW_KEY_B, Key()},
	{GLFW_KEY_C, Key()},
	{GLFW_KEY_D, Key()},
	{GLFW_KEY_E, Key()},
	{GLFW_KEY_F, Key()},
	{GLFW_KEY_G, Key()},
	{GLFW_KEY_H, Key()},
	{GLFW_KEY_I, Key()},
	{GLFW_KEY_J, Key()},
	{GLFW_KEY_K, Key()},
	{GLFW_KEY_L, Key()},
	{GLFW_KEY_M, Key()},
	{GLFW_KEY_N, Key()},
	{GLFW_KEY_O, Key()},
	{GLFW_KEY_P, Key()},
	{GLFW_KEY_Q, Key()},
	{GLFW_KEY_R, Key()},
	{GLFW_KEY_S, Key()},
	{GLFW_KEY_T, Key()},
	{GLFW_KEY_U, Key()},
	{GLFW_KEY_V, Key()},
	{GLFW_KEY_W, Key()},
	{GLFW_KEY_X, Key()},
	{GLFW_KEY_Y, Key()},
	{GLFW_KEY_Z, Key()},
	{GLFW_KEY_0, Key()},
	{GLFW_KEY_1, Key()},
	{GLFW_KEY_2, Key()},
	{GLFW_KEY_3, Key()},
	{GLFW_KEY_4, Key()},
	{GLFW_KEY_5, Key()},
	{GLFW_KEY_6, Key()},
	{GLFW_KEY_7, Key()},
	{GLFW_KEY_8, Key()},
	{GLFW_KEY_9, Key()},
	{GLFW_KEY_LEFT, Key()},
	{GLFW_KEY_RIGHT, Key()},
	{GLFW_KEY_UP, Key()},
	{GLFW_KEY_DOWN, Key()},
	{GLFW_KEY_LEFT_SHIFT, Key()}
};


static void onKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (keyMap.count(key) == 0)
	{
		//key pressed which is not set to be used in the keymap
		return;
	}

	switch (action)
	{
	case GLFW_PRESS:
		keyMap.at(key).setDown();
		break;

	case GLFW_RELEASE:
		keyMap.at(key).setUp();
		break;
	}
}

void onWindowResize(GLFWwindow* w, int width, int height)
{
	if (width == 0 || height == 0) return;

	glViewport(0, 0, width, height);

	Mandelbrot::onResize(width, height);
	Mandelbrot::requestUpdate();
}

bool init()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//check monitor size to set appropriate window size and centred position
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	int monitorX, monitorY, monitorWidth, monitorHeight;
	glfwGetMonitorWorkarea(monitor, &monitorX, &monitorY, &monitorWidth, &monitorHeight);

	windowWidth = monitorWidth * 0.8f;
	windowHeight = monitorHeight * 0.8f;
	window = glfwCreateWindow(windowWidth, windowHeight, "Mandelbrot", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}

	int spareWidth = monitorWidth - windowWidth;
	int spareHeight = monitorHeight - windowHeight;
	glfwSetWindowPos(window, monitorX + spareWidth / 2, monitorY + spareHeight / 2);

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return false;
	}

	glfwSetFramebufferSizeCallback(window, onWindowResize);
	glfwSetKeyCallback(window, onKeyPress);

	//initialise OpenCL with the window and kernel source code
	if (!CLManager::init(window, createKernelSource()))
	{
		std::cout << "failed to initialise CLManager, exiting" << std::endl;
		glfwTerminate();
		return false;
	}

	if (!Mandelbrot::init(windowWidth, windowHeight, 1)) return false;

	moveMult = 1.0f; //changes camera movement speed

	return true;
}

bool update()
{
	//camera controls
	glm::vec3 moveInputs = glm::vec3(0.0f);
	if (keyMap.at(GLFW_KEY_W).getHeld()) moveInputs.y += 1;
	if (keyMap.at(GLFW_KEY_S).getHeld()) moveInputs.y -= 1;
	if (keyMap.at(GLFW_KEY_A).getHeld()) moveInputs.x -= 1;
	if (keyMap.at(GLFW_KEY_D).getHeld()) moveInputs.x += 1;
	if (keyMap.at(GLFW_KEY_Q).getHeld()) moveInputs.z -= 1;
	if (keyMap.at(GLFW_KEY_E).getHeld()) moveInputs.z += 1;

	if (keyMap.at(GLFW_KEY_C).getReleased()) moveMult *= 2.0f;
	if (keyMap.at(GLFW_KEY_X).getReleased()) moveMult *= 0.5f;

	//calculate camera movement speed, adjust based on zoom to make it feel consistent
	float minMoveMult = 1.0f / (1 << 10);
	if (moveMult < minMoveMult) moveMult = minMoveMult;
	float moveSpeed = moveMult / Mandelbrot::getCamZoom() * frameDuration;

	if (glm::length2(moveInputs) > 0)
	{
		Mandelbrot::updateCam(moveInputs * moveSpeed, pow(2.0f, moveInputs.z * 2.0f * frameDuration));
		Mandelbrot::requestUpdate(); //whenever the view of the scene changes, it should be recalculated and drawn
	}

	if (keyMap.at(GLFW_KEY_Z).getReleased()) { Mandelbrot::resetCam(); Mandelbrot::requestUpdate(); }

	//change supersampling and maximum number of iterations
	if (keyMap.at(GLFW_KEY_T).getReleased()) { Mandelbrot::updateSuperSampling(1); Mandelbrot::requestUpdate(); }
	if (keyMap.at(GLFW_KEY_G).getReleased()) { Mandelbrot::updateSuperSampling(-1); Mandelbrot::requestUpdate(); }
	if (keyMap.at(GLFW_KEY_R).getReleased()) { Mandelbrot::updateMaxIterations(2.0f); Mandelbrot::requestUpdate(); }
	if (keyMap.at(GLFW_KEY_F).getReleased()) { Mandelbrot::updateMaxIterations(0.5f); Mandelbrot::requestUpdate(); }
	
	//recalculate the samples for each pixel (if an update has been requested)
	Mandelbrot::update();


	//update press/hold/release states for next frame
	for (auto it = keyMap.begin(); it != keyMap.end(); it++)
	{
		Key& k = it->second;
		k.updateStates();
	}	

	return true;
}

void draw()
{
	//draw the samples if an update was requested (and then swap the front and back buffers)
	if (Mandelbrot::draw())
	{
		glfwSwapBuffers(window);
	}
}

void destroy()
{
	glfwTerminate();
}

int main()
{
	if (!init()) return -1;

	std::chrono::steady_clock::time_point t0;

	while (!glfwWindowShouldClose(window))
	{
		t0 = std::chrono::steady_clock::now();

		glfwPollEvents();

		if (!update()) break;
		draw();


		//run program at a desired frame rate
		std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
		uint32_t frameDurationMicro = (t1 - t0).count() / 1e3;
		if (frameDurationMicro < desiredFrameDuration)
		{
			std::this_thread::sleep_for(std::chrono::microseconds(desiredFrameDuration - frameDurationMicro));
			frameDuration = desiredFrameDuration / 1e6;
		}
		else
		{
			frameDuration = frameDurationMicro / 1e6;
		}
	}

	destroy();
}
