
#version 460 core


in vec2 texCoord;

uniform uint screenWidth, screenHeight;
uniform uint texWidth;
uniform uint superSampling;
uniform uint maxIterations;


layout (std430) readonly buffer BufTex
{
    uint texIterations[];
};

out vec4 outColour;

vec4 getColour(float iterations)
{
    float L = min(log(iterations) / 4.0f, 0.8f);
    if (iterations == maxIterations) L = 0.0f;
    float C = 0.1f;
    float h = mod(iterations, 200.0f) / 200.0f * 6.283185f;

    float a = C * cos(h);
    float b = C * sin(h);

    float l_ = L + 0.3963377774f * a + 0.2158037573f * b;
    float m_ = L - 0.1055613458f * a - 0.0638541728f * b;
    float s_ = L - 0.0894841775f * a - 1.2914855480f * b;

    float l = l_ * l_ * l_;
    float m = m_ * m_ * m_;
    float s = s_ * s_ * s_;

    vec3 rgb = vec3(
         4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
        -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
        -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s
    );

    rgb = clamp(rgb, 0.0f, 1.0f);
    return vec4(rgb, 1.0f);
}

void main()
{
    uint x = int(texCoord.x * screenWidth);
    uint y = int(texCoord.y * screenHeight);

    float iterations = 0.0f;
    for (uint s = 0; s < superSampling * superSampling; s++)
    {
        uint sx = s % superSampling;
        uint sy = s / superSampling;
        uint i = (y * superSampling + sy) * texWidth + (x * superSampling + sx);
        iterations += float(texIterations[i]);
    }

    iterations /= superSampling * superSampling;

	outColour = getColour(iterations);
}