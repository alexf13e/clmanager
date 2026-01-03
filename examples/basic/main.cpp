
#define CL_MANAGER_IMPL //this should only be set in one place, as the functions will be multiply defined otherwise
#include "CLManager.h"
#include "kernels.h"
#include "common_def.h"


int main()
{
    //string value must match kernel name in kernels.cpp
    std::string k_addBuffers = "addBuffers";
    std::string k_mulBuffers = "mulBuffers";

    //buffer names can be anything, as long as unique. the name will be used in error messages
    std::string b_buffer1 = "buffer1";
    std::string b_buffer2 = "buffer2";

    //initialise the OpenCL device and context, and create a program from kernel source code string
    //most functions return a bool for whether they were successful or not
    if (!CLManager::init(createKernelSource()))
    {
        return -1;
    }

    //example number of values to be processed
    const uint32_t N = 1000;

    //create kernel objects for the kernels defined in kernels.cpp
    //the range of the kernel can be optionally set here, or later with setKernelRange()
    CLManager::createKernel(k_addBuffers, N);
    CLManager::createKernel(k_mulBuffers, N);

    //create buffer objects with a given name and number of elements with optional parameter for array of initial values
    CLManager::createBuffer<float>(b_buffer1, N);
    CLManager::createBuffer<float>(b_buffer2, N);

    //fillBuffer() is used to assign a single value to all elements in the buffer
    //writeBuffer() can be used to write an array of varying values
    CLManager::fillBuffer<float>(b_buffer1, N, 1.0f);
    CLManager::fillBuffer<float>(b_buffer2, N, 2.0f);

    //setting kernel parameters requires knowing the name of the kernel, the position for the first parameter, and a
    //list of parameters to be set consecutively
    CLManager::setKernelParamBuffer(k_addBuffers, 0, { b_buffer1, b_buffer2 });
    CLManager::setKernelParamBuffer(k_mulBuffers, 0, { b_buffer1, b_buffer2 });    

    //run the kernel once everything is set
    CLManager::runKernel(k_addBuffers);

    //read the result back, which should be 30 (buffer1 (1) + buffer2 (2) = 3, then multiply by magicNumberFromCommonDef
    //which is 10 to get 30)
    float* values = new float[N];
    CLManager::readBuffer(b_buffer1, N, values);
    std::cout << std::to_string(values[0]) << std::endl;
    delete[] values;

    //run another kernel, reading back only the element at index 10
    //buffer1 got values of 30 in each index after running addBuffers, and buffer2 was assigned TEST_DEFINITION (12345).
    //so buffer1 is 30 * 12345 = 370350
    float singleValue;
    CLManager::runKernel(k_mulBuffers);
    CLManager::readBuffer(b_buffer1, 1, &singleValue, 10);
    std::cout << std::to_string(singleValue) << std::endl;

    return 0;
}