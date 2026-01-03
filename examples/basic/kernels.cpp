
#include "kernels.h"
#include "KernelRString.h"


std::string createKernelSource()
{
//preprocessor statements don't use KERNEL_R_STRING as newlines need to be preserved
std::string strPreProc = "\
#include \"common_def.h\"\n\
#define TEST_DEFINITION 12345\n\
";

std::string strAddBuffers = KERNEL_R_STRING(
kernel void addBuffers(global float* a, global float* b)
{
	uint i = get_global_id(0);
	a[i] = (a[i] + b[i]) * magicNumberFromCommonDef;
	b[i] = TEST_DEFINITION;
}
);

std::string strMulBuffers = KERNEL_R_STRING(
kernel void mulBuffers(global float* a, global float* b)
{
	uint i = get_global_id(0);
	a[i] = a[i] * b[i];
}
);


    std::string fullKernelSource =
		strAddBuffers +
		strMulBuffers;
	
	//preprocessor statements not put through formatKernelString() as spacing gets interfered with, especially with
	//#defines where the existence of a value cannot be easily determined
    return strPreProc + formatKernelString(fullKernelSource);
}