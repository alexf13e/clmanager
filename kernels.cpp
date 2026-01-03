
#include "kernels.h"
#include "KernelRString.h"


std::string createKernelSource()
{

std::string strKernelPlaceholder = KERNEL_R_STRING(
kernel void kernelPlaceholder(global float* a, global float* b, global float* c, uint N)
{
	uint i = get_global_id(0);
	if (i >= N) return;
	c[i] = a[i] + b[i];
}
);

    return formatKernelString(strKernelPlaceholder);
}