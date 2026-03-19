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
#include "include/TextureLoading.h"
#include "include/LogMacro.h"

#define DELETE_BUFFER( vbo) \
    if(vbo)  \
    {   \
        glDeleteBuffers( 1, &vbo); \
        vbo = 0; \
    }

#define DELETE_VERTEX_ARRAY( vao) \
    if(vao)  \
    {   \
        glDeleteVertexArrays( 1, &vao); \
        vao = 0; \
    }

#define DELETE_TEXTURE( tex) \
    if(tex)  \
    {   \
        glDeleteTextures( 1, &tex); \
        tex = 0; \
    }

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
