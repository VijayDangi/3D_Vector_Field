#version 430 core

layout (location = 0) in vec4 a_position;
layout (location = 1) in vec4 a_velocity;
layout (location = 2) in float a_age;

uniform mat4 u_viewProjMatrix;
uniform float u_particleSize;

out float v_normalizedValue;

void main()
{
    float maxExpectedSpeed = 1.5;
    v_normalizedValue = length(a_velocity.xyz) / maxExpectedSpeed;

    // float maxAge = 10.0;
    // v_normalizedValue = a_age / 0.7;

    gl_Position = u_viewProjMatrix * vec4(a_position.xyz, 1.0);

    gl_PointSize = u_particleSize;
}