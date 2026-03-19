#ifndef __SS_GL_LOAD_SHADER_H__
#define __SS_GL_LOAD_SHADER_H__

#include <Windows.h>
#include <stdio.h>

#include "glew\include\GL\glew.h"
#include <gl/gl.h>

namespace ss_gl {
    enum EShaderLoadAs {
        File = 0x1,
        String = 0x2
    };

    typedef struct SBindAttributesInfo
    {
        const char *attribute;
        GLuint index;

    } SBindAttributesInfo;

    typedef struct SShadersInfo
    {
        GLenum shaderType;
        EShaderLoadAs shaderLoadAs;
        union
        {
            const char *shaderSource;
            const char *shaderFileName;
        };

        GLuint shaderID;

    }SShadersInfo;

    typedef struct SFeedbackInfo
    {
        const char **varying;
        int varyingCount;
        GLenum bufferMode;
    } SFeedbackInfo;


    //function declaration
    GLuint CreateProgram(SShadersInfo *shaderInfo, int shaderCount, SBindAttributesInfo *attribInfo, int attribCount, SFeedbackInfo *feedBack);
    void DeleteProgram(GLuint program);
}

#endif  // __SS_GL_LOAD_SHADER_H__
