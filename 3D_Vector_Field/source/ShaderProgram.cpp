#include "include/ShaderProgram.h"
#include "Common.h"

#include <string>

#define CHAR_BUFFER_SIZE 1024

/**
 * @name ReadFile
 */
static bool ReadFile( const char *fileName, std::string& outData)
{
	//code
	if (fileName == nullptr)
		return false;

	FILE *fp = nullptr;

#ifdef _WIN32
	fopen_s(&fp, fileName, "r");
#else
	fp = fopen(fileName, "r");
#endif

	if (fp == nullptr)
	{
        LogError("Error While Opening File '%s'\n", fileName);
		return false;
	}

	fseek(fp, 0, SEEK_END);
	long length = ftell(fp);
	fseek(fp, 0, SEEK_SET);

    // Log("File \"%s\" Length : %d bytes\n", fileName, length);

	outData.clear();
    outData.resize(length + 1);
	size_t data_read = fread( (void*)outData.data(), length, 1, fp);
	// outData[data_read] = '\0';

	fclose(fp);
	fp = nullptr;

	return true;
}

/**
 * @name GetShaderType
 */
static char* GetShaderType( GLenum type)
{
	switch( type)
	{
		case GL_VERTEX_SHADER:          return "GL_VERTEX_SHADER";
		case GL_TESS_CONTROL_SHADER:    return "GL_TESS_CONTROL_SHADER";
		case GL_TESS_EVALUATION_SHADER: return "GL_TESS_EVALUATION_SHADER";
		case GL_GEOMETRY_SHADER:        return "GL_GEOMETRY_SHADER";
		case GL_FRAGMENT_SHADER:        return "GL_FRAGMENT_SHADER";
        case GL_COMPUTE_SHADER:         return "GL_COMPUTE_SHADER";

		default: return "UNKNOWN_SHADER";
	}
}

namespace ss_gl {
    /**
     * @name CreateProgram
     */
    GLuint CreateProgram(SShadersInfo *shaderInfo, int shaderCount, SBindAttributesInfo *attribInfo, int attribCount, SFeedbackInfo *feedBack)
    {
        //variables
        GLuint programID = 0;
        GLint infoLogLength = 0;
        GLint shaderCompiledStatus, programLinkStatus;

        //code
        programID = glCreateProgram();

        for(int i = 0; i < shaderCount; i++)
        {
            std::string shaderSource;
            if (shaderInfo[i].shaderLoadAs == EShaderLoadAs::File)
            {
                if (ReadFile(shaderInfo[i].shaderFileName, shaderSource) == false)
                {
                    LogError( "Failed to reading file \"%s\".\n", shaderInfo[i].shaderFileName);
                    DeleteProgram(programID);
                    return 0;
                }
            }
            else
            {
                shaderSource = (char *) ( shaderInfo[i].shaderSource);
            }

            // LogWarning("\n%s\n", shaderSource.c_str());

            shaderInfo[i].shaderID = glCreateShader(shaderInfo[i].shaderType);
            if (shaderInfo[i].shaderID == 0)
            {
                LogError( "Shader Craation Failed.");
                DeleteProgram(programID);
                return 0;
            }

            const char *s = shaderSource.data();
            glShaderSource( shaderInfo[i].shaderID, 1, &s, nullptr);
            glCompileShader( shaderInfo[i].shaderID);

            //Compilation status
            glGetShaderiv( shaderInfo[i].shaderID, GL_COMPILE_STATUS, &shaderCompiledStatus);
            if( shaderCompiledStatus == GL_FALSE)   //Compilation Failed
            {
                //get shader compile log length
                glGetShaderiv( shaderInfo[i].shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
                if(infoLogLength > 0)
                {
                    //allocate memory for log
                    std::string infoLog;
                    infoLog.resize(infoLogLength);
                        //get log
                    glGetShaderInfoLog( shaderInfo[i].shaderID, infoLogLength, nullptr, (GLchar*)infoLog.data());
                    LogError("%s Compile Log : \n%s\n", GetShaderType(shaderInfo[i].shaderType), infoLog.c_str());

                    DeleteProgram(programID);
                    return 0;
                }
            }

            glAttachShader(programID, shaderInfo[i].shaderID);
        }

        //bind attributes
        if( attribInfo)
        {
            for (int i = 0; i < attribCount; i++)
            {
                glBindAttribLocation(programID, (attribInfo + i)->index, (attribInfo + i)->attribute);
            }
        }

        //transform feedback
        if( feedBack)
        {
            glTransformFeedbackVaryings( programID, feedBack->varyingCount, feedBack->varying, feedBack->bufferMode);
        }

        //Link Program
        glLinkProgram(programID);

        //linking status
        glGetProgramiv(programID, GL_LINK_STATUS, &programLinkStatus);
        if (programLinkStatus == GL_FALSE)   //Linking Failed
        {
            //get link log length
            glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
            if (infoLogLength > 0)
            {
                std::string infoLog;
                infoLog.resize(infoLogLength + 1, '\0');
                glGetProgramInfoLog(programID, infoLogLength, nullptr, (GLchar*)infoLog.data());
                LogError( "Shader Program Linking Log : %d\n%s\n", infoLogLength, infoLog.c_str());
                DeleteProgram(programID);
                return 0;
            }
        }

        return programID;
    }

    /**
     * @name DeleteProgram
     */
    void DeleteProgram(GLuint program)
    {
        //varriables
        GLsizei shaderCount;
        GLsizei actualShaderCount;

        //code
        glUseProgram(program);

        glGetProgramiv(program, GL_ATTACHED_SHADERS, &shaderCount);
        GLuint *pShader = (GLuint *)malloc(shaderCount * sizeof(GLuint));

        glGetAttachedShaders(program, shaderCount, &actualShaderCount, pShader);
        
        for (int i = 0; i < shaderCount; i++)
        {
            glDetachShader(program, pShader[i]);
            glDeleteShader(pShader[i]);
        }

        glDeleteProgram(program);
        program = 0;

        glUseProgram(0);

        free(pShader);
        pShader = 0;
    }
}

#undef CHAR_BUFFER_SIZE

