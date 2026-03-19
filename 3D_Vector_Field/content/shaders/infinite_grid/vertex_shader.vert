#version 450 core

// Full screen quad vertices
const vec2 gridPositions[6] = vec2[6](
    vec2( 1.0, -1.0), // 1
    vec2(-1.0, -1.0), // 0
    vec2( 1.0,  1.0), // 2

    vec2(-1.0,  1.0), // 3
    vec2( 1.0,  1.0), // 2
    vec2(-1.0, -1.0) // 0
);

layout(location = 0) out vec3 o_nearPoint;
layout(location = 1) out vec3 o_farPoint;
layout(location = 2) out vec2 o_ndc;

// Uniform
uniform mat4 u_inverse_view;
uniform mat4 u_inverse_projection;

vec3 UnprojectPoint(float x, float y, float z)
{
    vec4 unprojectedPoint = u_inverse_view * u_inverse_projection * vec4(x, y, z, 1.0);
    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main()
{
    // Unproject the 2D screen coordinate to 3D world space points
    vec2 screenPos = gridPositions[gl_VertexID];
    o_nearPoint = UnprojectPoint(screenPos.x, screenPos.y, 0.0);
    o_farPoint = UnprojectPoint(screenPos.x, screenPos.y, 1.0);
    o_ndc = screenPos;

    gl_Position = vec4(screenPos, 1.0, 1.0);
}
