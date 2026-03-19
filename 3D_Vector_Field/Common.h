#pragma once

#ifdef _WIN32
    #include <Windows.h>
    #include <windowsx.h>
#endif

#include "glew/include/GL/glew.h"
#include "glew/include/GL/wglew.h"
#include <gl/gl.h>  //OpenGL API

#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>

#include "include/ShaderProgram.h"
#include "include/LogMacro.h"

// Single Vertex Attributes/Properties
enum EVertexAttributeIndex
{
    Position = 0,
    Color,
    Normal,
    Texcoord,
    Tangent,
    Bitangent,
};

enum EMouseButton
{
    Left = MK_LBUTTON,
    Middle = VK_MBUTTON,
    Right = VK_RBUTTON,
};
