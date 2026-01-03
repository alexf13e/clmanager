
#include "mandelbrot.h"

#include <string>
#include <vector>

#include "glm/glm.hpp"

#define CL_MANAGER_GL
#define CL_MANAGER_IMPL
#include "CLManager.h"

#include "ShaderProgram.h"
#include "Camera2D.h"


namespace Mandelbrot
{
	//a single triangle covering the screen will be used for rendering the iteration buffer with a fragment shader
	ShaderProgram shFullScreenTri;
	GLuint vao_fullScreenTri;

	//create strings for referring to OpenCL buffers and kernels
    std::string glb_texIterations = "texIterations"; //buffer of iteration counts for each pixel
	std::string k_mandelbrot = "mandelbrot"; //name of the kernel which will be run
	std::vector<cl::Memory> glObjectsToAcquire;

	uint32_t screenWidth, screenHeight;
	uint32_t texWidth, texHeight;
	Camera2D cam;
	uint32_t texSuperSampling;
	bool doUpdate;

	uint32_t maxIterations;


    void acquireGLObjects()
	{
		int error = CLManager::queue.enqueueAcquireGLObjects(&glObjectsToAcquire);
		if (error != CL_SUCCESS)
		{
			std::cout << "error acquiring GL object: " << CLManager::getErrorString(error) << std::endl;
		}
	}

	void releaseGLObjects()
	{
		int error = CLManager::queue.enqueueReleaseGLObjects(&glObjectsToAcquire);
		if (error != CL_SUCCESS)
		{
			std::cout << "error releasing GL object: " << CLManager::getErrorString(error) << std::endl;
		}
	}

    void onResize(uint32_t width, uint32_t height)
	{
		cam.setAspectRatio(width, height);
		screenWidth = width;
		screenHeight = height;
		texWidth = screenWidth * texSuperSampling;
		texHeight = screenHeight * texSuperSampling;
		createIterationBuffer();
	}

	float getCamZoom()
	{
		return cam.zoom;
	}

	void updateCam(const glm::vec2& deltaPos, const float deltaZoom)
	{
		cam.updatePosition(deltaPos);
		cam.updateView(deltaZoom);
	}

	void resetCam()
	{
		cam.reset();
	}

	void createIterationBuffer()
	{
		//delete buffer from list of OpenGL objects to acquire if it exists (e.g. when resizing cell buffer)
		for (uint32_t i = 0; i < glObjectsToAcquire.size(); i++)
		{
			if (glObjectsToAcquire[i] == CLManager::glBuffers[glb_texIterations].clBuffer)
			{
				glObjectsToAcquire.erase(glObjectsToAcquire.begin() + i);
			}
		}

		//create buffer which can be shared between OpenGL and OpenCL
		//use shader storage buffer with no relation to a specific VAO as its easier to work with (than e.g. a texture)
		//each element will store the number of iterations a sample point took to "diverge"
		CLManager::createGLBufferNoVAO<uint32_t>(glb_texIterations, GL_SHADER_STORAGE_BUFFER, texWidth * texHeight);
		
		//OpenCL needs the cl::Memory associated with the OpenGL buffer when acquiring, so give .clBuffer
		glObjectsToAcquire.push_back(CLManager::glBuffers[glb_texIterations].clBuffer);


		glUseProgram(shFullScreenTri.getID());

		//tell shader where the buffer is
		int bufferBlockBinding = 0;
		int bufferBlockIndex = glGetProgramResourceIndex(shFullScreenTri.getID(), GL_SHADER_STORAGE_BLOCK, "BufTex");
		glShaderStorageBlockBinding(shFullScreenTri.getID(), bufferBlockIndex, bufferBlockBinding);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bufferBlockBinding, CLManager::glBuffers[glb_texIterations].glBuffer);

		//tell shader the shape of the render
		glUniform1ui(glGetUniformLocation(shFullScreenTri.getID(), "screenWidth"), screenWidth);
		glUniform1ui(glGetUniformLocation(shFullScreenTri.getID(), "screenHeight"), screenHeight);
		glUniform1ui(glGetUniformLocation(shFullScreenTri.getID(), "texWidth"), texWidth);
		glUseProgram(0);
	}

	void updateSuperSampling(int delta)
	{
		texSuperSampling += delta;
		if (texSuperSampling < 1) texSuperSampling = 1;
		if (texSuperSampling > 8) texSuperSampling = 8;
		
		texWidth = screenWidth * texSuperSampling;
		texHeight = screenHeight * texSuperSampling;

		createIterationBuffer();

		glUseProgram(shFullScreenTri.getID());
		glUniform1ui(glGetUniformLocation(shFullScreenTri.getID(), "superSampling"), texSuperSampling);
		glUseProgram(0);
	}

	void updateMaxIterations(float delta)
	{
		maxIterations *= delta;
		if (maxIterations < 1) maxIterations = 1;
		if (maxIterations > 4096) maxIterations = 4096;

		glUseProgram(shFullScreenTri.getID());
		glUniform1ui(glGetUniformLocation(shFullScreenTri.getID(), "maxIterations"), maxIterations);
		glUseProgram(0);
	}

	void requestUpdate()
	{
		doUpdate = true;
	}

    bool init(uint32_t width, uint32_t height, uint32_t superSampling)
    {
		//create shader for rendering a triangle over the whole screen, with a fragment shader which reads the values
		//calculated in OpenCL
		if (!shFullScreenTri.init("./shaders/fullScreenTri.vert", "./shaders/mandelbrot.frag")) return false;
		glGenVertexArrays(1, &vao_fullScreenTri);

		screenWidth = width;
		screenHeight = height;
		texSuperSampling = 1;
		maxIterations = 100;
		updateSuperSampling(0);
		updateMaxIterations(1.0f);
		
		cam.init(texWidth, texHeight, glm::vec2(0.0f));
		
        createIterationBuffer();
		CLManager::createKernel(k_mandelbrot);
		
		doUpdate = true;

		return true;
    }

    void update()
    {
		if (!doUpdate) return; //don't recalculate every frame when the camera is stationary

		acquireGLObjects(); //must "acquire" memory from OpenGL so that it can be modified by OpenCL
		CLManager::setKernelRange(k_mandelbrot, texWidth * texHeight);
		CLManager::setKernelParamGLBuffer(k_mandelbrot, 0, { glb_texIterations }); //important to note param *GL* buffer
		CLManager::setKernelParamValue(k_mandelbrot, 1, texWidth);
		CLManager::setKernelParamValue(k_mandelbrot, 2, texHeight);
		CLManager::setKernelParamValue(k_mandelbrot, 3, maxIterations);
		CLManager::setKernelParamValue(k_mandelbrot, 4, cam.getInverseMatViewCL());
		CLManager::runKernel(k_mandelbrot);
		releaseGLObjects(); //give the memory back when done
    }

	bool draw()
	{
		if (!doUpdate) return false; //don't draw when nothing to draw

		glUseProgram(shFullScreenTri.getID());
		glBindVertexArray(vao_fullScreenTri);
		glDrawArrays(GL_TRIANGLES, 0, 3); //vertex shader will set the position of 3 points, so just ask to draw them
		glBindVertexArray(0);
		glUseProgram(0);

		doUpdate = false; //update has been complete, don't do another one until requested

		return true;
	}
}