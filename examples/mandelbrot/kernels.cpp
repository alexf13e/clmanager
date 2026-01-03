
#include "kernels.h"
#include "KernelRString.h"


std::string createKernelSource()
{

std::string strMat4MulVec4 = KERNEL_R_STRING(
float4 mat4MulVec4(float16 mat, float4 vec)
{
	return (float4)(
		mat.s0 * vec.x + mat.s1 * vec.y + mat.s2 * vec.z + mat.s3 * vec.w,
		mat.s4 * vec.x + mat.s5 * vec.y + mat.s6 * vec.z + mat.s7 * vec.w,
		mat.s8 * vec.x + mat.s9 * vec.y + mat.sA * vec.z + mat.sB * vec.w,
		mat.sC * vec.x + mat.sD * vec.y + mat.sE * vec.z + mat.sF * vec.w
	);
}
);

std::string strMandelbrot = KERNEL_R_STRING(
kernel void mandelbrot(global uint* texIterations, uint texWidth, uint texHeight, uint maxIterations,
	float16 matScreenToWorld)
{
	uint i = get_global_id(0);
	if (i >= texWidth * texHeight) return;

	float2 screenUV = (float2)((float)(i % texWidth) / texWidth, (float)(i / texWidth) / texHeight);
	float4 screenNorm = (float4)(screenUV * 2.0f - 1.0f, 0.0f, 1.0f);
	float4 worldPos = mat4MulVec4(matScreenToWorld, screenNorm);

	float2 z = (float2)(0.0f);
	float2 c = (float2)(worldPos.x, worldPos.y);

	uint iterations = 0;
	while (iterations < maxIterations)
	{
		z = (float2)(z.x * z.x - z.y * z.y, 2.0f * z.x * z.y) + c;

		if (length(z) > 2.0f) break;
		iterations++;
	}

	texIterations[i] = iterations;
}
);


    std::string fullKernelSource =
		strMat4MulVec4 +
		strMandelbrot;

    return formatKernelString(fullKernelSource);
}