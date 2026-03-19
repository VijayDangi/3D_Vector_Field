#version 450 core

layout(location = 0) in vec3 in_nearPoint;
layout(location = 1) in vec3 in_farPoint;
layout(location = 2) in vec2 in_ndc;

layout(location = 0) out vec4 o_fragColor;

// Uniform
uniform mat4 u_view;
uniform mat4 u_projection;
uniform float u_gridIntensity;

/**
* Only For Plane XZ
**/
// Procedural grid generation function
vec4 CalculateGrid(vec3 fragPos3D, float scale)
{
    vec2 coord = fragPos3D.xz * scale;

    // Use derivatives to anti-alias the grid lines
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);

    float minimumZ = min(derivative.y, 1.0);
    float minimumX = min(derivative.x, 1.0);

    vec4 color = vec4(u_gridIntensity, u_gridIntensity, u_gridIntensity, 1.0 - min(line, 1.0));

    // Highlight the main X and Z axes
    if(fragPos3D.x > -0.1 * minimumX && fragPos3D.x < 0.1 * minimumX)
    {
        color.x = 0.0;
        color.y = 0.0;
        color.z = 2.0; // Z axis (Blue)
        color.w = 1.0;
    }
    if(fragPos3D.z > -0.1 * minimumZ && fragPos3D.z < 0.1 * minimumZ)
    {
        color.x = 2.0; // X axis (Red)
        color.y = 0.0;
        color.z = 0.0;
        color.w = 1.0;
    }

    return color;
}

void main()
{
    float t = -in_nearPoint.y / (in_farPoint.y - in_nearPoint.y);

    if(t < 0.0)
    {
        discard;
    }

    vec3 fragPos3D = in_nearPoint + t * (in_farPoint - in_nearPoint);

    o_fragColor = CalculateGrid(fragPos3D, 10)  +
                  CalculateGrid(fragPos3D, 1)   +
                  CalculateGrid(fragPos3D, 0.1); // +
                //   CalculateGrid(fragPos3D, 0.01);
    
    float linearDepth = length(fragPos3D - in_nearPoint);
    float fading = max(0.0, (0.7 - linearDepth / 100.0));   // Fade out over 100 units.

    o_fragColor.a *= fading;

    vec4 clipSpacePos = u_projection * u_view * vec4(fragPos3D, 1.0);
    gl_FragDepth = (clipSpacePos.z / clipSpacePos.w) * 0.5 + 0.5;
}
