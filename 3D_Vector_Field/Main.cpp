//Headers
#include "Common.h"
#include "Resource.h"

#include <stdio.h>
#include <iostream>
#include <strsafe.h>

#include "OGLApplication.h"

// #define CONSOLE_APPLICATION 1
#define SS_GL_OPENGL_MAJOR_VERSION 4
#define SS_GL_OPENGL_MINOR_VERSION 5

//Global function declaration
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM);

IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

//Global variables
#ifdef CONSOLE_APPLICATION
static TCHAR gWindowText[] = TEXT("Shader Studio (Console)");
#else
static TCHAR gWindowText[] = TEXT("Shader Studio (Win32)");
#endif

TCHAR *szClassName = TEXT("ShaderApp");
HWND  ghwnd;
HDC   ghdc;
HGLRC ghrc;

DWORD style;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };

int windowWidth  = 800;
int windowHeight = 600;

bool gbActiveWindow = false;
bool gbFullscreen = false;
bool gbDone = false;
uint32_t FrameRate_FPS = 0;

const char* logFileName = "log.txt";

bool bIsDebugOpenGLContext = false;

//WinMain()
#ifdef CONSOLE_APPLICATION
int main()
{
    HINSTANCE hInstance = GetModuleHandle(nullptr);
    int iCmdShow = SW_NORMAL;
#else
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInsatnce, LPSTR szCmdLine, int iCmdShow)
{
#endif
    //function declarations
    bool CreateDummyWindowToInitializeOpenGLAPIs(HINSTANCE hInstance);
    bool CreateMainWindow(HINSTANCE hInstance);
    bool Initialize( void);
    void RunGameLoop();
    void UnInitialize( void);

    //code
    FILE *fp = nullptr;
    if(fopen_s( &fp, logFileName, "w") != 0)   // Recreate File
    {
        MessageBox( NULL, TEXT("Error while creating log file"), TEXT("Error"), MB_OK | MB_TOPMOST | MB_ICONSTOP);
        return( EXIT_FAILURE);
    }
    else
    {
        fclose(fp);
        fp = nullptr;
        LogInfo("Log File Opened...");
    }

    if(!CreateMainWindow(hInstance))
    {
        LogError("CreateMainWindow() Failed.");
        return( EXIT_FAILURE);
    }

    if(!CreateDummyWindowToInitializeOpenGLAPIs(hInstance))
    {
        LogError("CreateDummyWindowToInitializeOpenGLAPIs() Failed.");
        return( EXIT_FAILURE);
    }
    
    if(!Initialize())
    {
        LogError("Initialize() Failed.");
        return 1;
    }

    // Enter Game Loop
    RunGameLoop();
    
    UnInitialize();

    UnregisterClass( szClassName, hInstance);

    return(EXIT_SUCCESS);
}

//Error Log
void PrintLog( const char *verbose, int text_color, int background_color, int lineNo, const char *fileName, const char *functionName, const char *format, ...)
{
    // code
    FILE *fp = nullptr;
    fopen_s( &fp, logFileName, "a");
    if( fp)
    {
        va_list argList;

        va_start( argList, format);

            fprintf( fp, "[%s\\%s() : %d]: [%s] ", fileName, functionName, lineNo, verbose);
            vfprintf( fp, format, argList);
            fprintf( fp, "\n");
            fflush( fp);

            fclose(fp);
            fp = nullptr;

#ifdef CONSOLE_APPLICATION
        printf( "[%s\\%s() : %d]: \033[%d;%dm[%s] ", fileName, functionName, lineNo, background_color, text_color, verbose);
        vprintf( format, argList);
        printf("\033[m");
        printf( "\n");
#endif

        va_end( argList);
    }
}

bool CreateDummyWindowToInitializeOpenGLAPIs(HINSTANCE hInstance)
{
    // code
    WNDCLASSEX wndclass{};
    HWND hwnd;

    wndclass.cbSize         = sizeof(WNDCLASSEX);
    wndclass.style          = CS_HREDRAW | CS_VREDRAW  | CS_OWNDC;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.hInstance      = hInstance;
    wndclass.hbrBackground  = (HBRUSH) GetStockObject( LTGRAY_BRUSH);
    wndclass.lpszClassName  = TEXT("DummyWindow");
    wndclass.lpszMenuName   = NULL;
    wndclass.lpfnWndProc    = DefWindowProc;
    
    RegisterClassEx( &wndclass);
    
    hwnd = CreateWindow(
        TEXT("DummyWindow"),
        TEXT("Dummy OpenGL Window"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL,
        hInstance, NULL
    );

    if(hwnd == NULL)
    {
        LogError("Failed to create dummy window\n");
        return false;
    }

    HDC dummyDC = GetDC(hwnd);
    PIXELFORMATDESCRIPTOR pfd{};
    int pixelFormat = ChoosePixelFormat(dummyDC, &pfd);
    SetPixelFormat(dummyDC, pixelFormat, &pfd);

    HGLRC dummyContext = wglCreateContext(dummyDC);

    if(wglMakeCurrent(dummyDC, dummyContext) == FALSE)
    {
        LogError("Failed to make dummy OpenGL context current\n");

        DestroyWindow(hwnd);
        return false;
    }

    if(GLEW_OK != glewInit())
    {
        LogError("Failed to initialize GLEW\n");

        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(dummyContext);
        ReleaseDC(hwnd, dummyDC);
        DestroyWindow(hwnd);
        return false;
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(dummyContext);
    ReleaseDC(hwnd, dummyDC);
    DestroyWindow(hwnd);

    return true;
}

// CreateMainWindow()
bool CreateMainWindow(HINSTANCE hInstance)
{
    //variable declarations
    WNDCLASSEX wndclass;

    //Initialize window attributes
    wndclass.cbSize         = sizeof(WNDCLASSEX);
    wndclass.style          = CS_HREDRAW | CS_VREDRAW  | CS_OWNDC;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.hInstance      = hInstance;
    wndclass.hIcon          = LoadIcon( hInstance, MAKEINTRESOURCE(IDI_MYICON));
    wndclass.hIconSm        = LoadIcon( hInstance, MAKEINTRESOURCE(IDI_MYICON));
    wndclass.hCursor        = LoadCursor( NULL, IDC_ARROW);
    wndclass.hbrBackground  = (HBRUSH) GetStockObject( LTGRAY_BRUSH);
    wndclass.lpszClassName  = szClassName;
    wndclass.lpszMenuName   = NULL;
    wndclass.lpfnWndProc    = WndProc;
    
    if(!RegisterClassEx( &wndclass))
    {
        LogError( "Class Not Registered\n");
        return false;
    }
    
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    ghwnd = CreateWindowEx(
        WS_EX_APPWINDOW,
        szClassName,
        gWindowText,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
        (screenWidth - windowWidth) / 2, (screenHeight - windowHeight) / 2, windowWidth, windowHeight,
        NULL, NULL,
        hInstance, NULL
    );

    ShowWindow( ghwnd, SW_MAXIMIZE);
    SetForegroundWindow(ghwnd);
    SetFocus( ghwnd);

        // Get the device context
    ghdc = GetDC( ghwnd);

    return true;
}

// PollWindowEvent()
bool PollWindowEvent()
{
    // code
    MSG   msg;
    bool bIsMessageOccur = false;

    bIsMessageOccur = PeekMessage( &msg, NULL, 0, 0, PM_REMOVE);
    if(bIsMessageOccur)
    {
        if(msg.message == WM_QUIT)
        {
            gbDone = true;
        }
        else
        {
            TranslateMessage( &msg);
            DispatchMessage( &msg);
        }
    }

    return bIsMessageOccur;
}

// RunGameLoop()
void RunGameLoop()
{
    // Function declarations
    void Display( double dt);

    //Game Loop
    LARGE_INTEGER StartTime;
    LARGE_INTEGER EndTime;
    LARGE_INTEGER Frequency;
    double deltaTime = 0.f;
    double totalTime = 0.f;
    uint32_t FPS = 0;

    QueryPerformanceCounter( &StartTime);
    QueryPerformanceFrequency( &Frequency);
    double oneDivideByFrequency = 1.0 / (double)Frequency.QuadPart;

    while( gbDone == false)
    {
        if(!PollWindowEvent())
        {
            Display(deltaTime);

            QueryPerformanceCounter( &EndTime);
            deltaTime = (double)(EndTime.QuadPart - StartTime.QuadPart) * oneDivideByFrequency;
            StartTime = EndTime;

            totalTime += deltaTime;
            FPS++;
            if(totalTime >= 1.0f)
            {
                TCHAR buffer[128];

                StringCchPrintf(buffer, 128, TEXT("%s [FrameRate_FPS: %u]"), gWindowText, FPS);
                SetWindowText(ghwnd, buffer);

                totalTime = totalTime - 1.0f;
                FrameRate_FPS = FPS;
                FPS = 0;
            }
        }
    }
}

//WndProc()
LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    //function declaration
    void Resize( int, int);
    void ToggleFullScreen( void);
    void UnInitialize( void);
    
    //variables
	static int mousePosX, mousePosY;
	static int iAccumDelta, zDelta;

	int mouseX, mouseY;
	int mouseDx, mouseDy;

	int newX = -1;
	int newY = -1;

	POINT pt;
    
    //code
    if(ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
    {
        return true;
    }

    switch(message)
    {
        case WM_SETFOCUS:
            gbActiveWindow = true;
        break;

        case WM_KILLFOCUS:
            gbActiveWindow = false;
        break;
        
        case WM_SIZE:
            windowWidth = LOWORD(lParam);
            windowHeight = HIWORD(lParam);
            
            Resize( windowWidth, windowHeight);
        break;

        case WM_KEYDOWN:
            OglApplication::KeyDownEventHandler(wParam);
        break;

        case WM_KEYUP:
        case WM_SYSKEYUP:
            if(wParam == VK_ESCAPE)
            {
                DestroyWindow(hwnd);
            }
            else if(wParam == 'F' || wParam == 'f')
            {
                ToggleFullScreen();
            }
            else
            {
                OglApplication::KeyUpEventHandler(wParam);
            }
        break;
        
        case WM_LBUTTONDOWN:
            SetCapture( hwnd);
            OglApplication::MouseDownEventHandler(EMouseButton::Left);
        break;
        case WM_RBUTTONDOWN:
            OglApplication::MouseDownEventHandler(EMouseButton::Right);
        break;
        case WM_MBUTTONDOWN:
            OglApplication::MouseDownEventHandler(EMouseButton::Middle);
        break;

        case WM_LBUTTONUP:
            ReleaseCapture();
            OglApplication::MouseUpEventHandler(EMouseButton::Left);
        break;
        case WM_RBUTTONUP:
            OglApplication::MouseUpEventHandler(EMouseButton::Right);
        break;
        case WM_MBUTTONUP:
            OglApplication::MouseUpEventHandler(EMouseButton::Middle);
        break;

        case WM_MOUSEMOVE:
            OglApplication::MouseMoveEventHandler((EMouseButton)wParam, LOWORD(lParam), HIWORD(lParam));
        break;

        case WM_MOUSEWHEEL:
            OglApplication::MouseWheelEventHandler(
                (EMouseButton)GET_KEYSTATE_WPARAM(wParam),
                LOWORD(lParam),
                HIWORD(lParam),
                (float)GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA
            );
        break;

        case WM_CLOSE:
        break;
        
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
    }
    
    return( DefWindowProc(hwnd, message, wParam, lParam));
}

//ToggleFullScreen()
void ToggleFullScreen( void)
{
    //variable declarations
    MONITORINFO mi = { sizeof(MONITORINFO) };
    
    //code
    if(gbFullscreen == false)
    {
        style = GetWindowLong(ghwnd, GWL_STYLE);
        if(style & WS_OVERLAPPEDWINDOW)
        {
            if(
                GetWindowPlacement(ghwnd, &wpPrev) &&
                GetMonitorInfo(
                    MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY),
                    &mi
                )
            )
            {
                SetWindowLong(ghwnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(
                    ghwnd,
                    HWND_TOP,
                    mi.rcMonitor.left,
                    mi.rcMonitor.top,
                    mi.rcMonitor.right - mi.rcMonitor.left,
                    mi.rcMonitor.bottom - mi.rcMonitor.top,
                    SWP_NOZORDER | SWP_FRAMECHANGED
                );
            }
        }
        //ShowCursor( FALSE);

        gbFullscreen = true;
    }
    else
    {
        SetWindowLong( ghwnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement( ghwnd, &wpPrev);
        SetWindowPos(
            ghwnd,
            HWND_TOP,
            0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED
        );
        
        gbFullscreen = false;
        //ShowCursor( TRUE);
    }
}

//InitializeOpenGLByTraditionalMethod()
bool InitializeOpenGLByTraditionalMethod( HWND hwnd, HDC hDC, HGLRC *pHGLRC)
{
    //variable declarations
    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex;
    HGLRC hglrc;

    //code
    if(hwnd == NULL)
    {
        return (false);
    }

    *pHGLRC = NULL;

    ZeroMemory( &pfd, sizeof(PIXELFORMATDESCRIPTOR));
    
        //Initialize PIXELFORMATDESCRIPTOR
    pfd.nSize       = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion    = 1;
    pfd.dwFlags     = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType  = PFD_TYPE_RGBA;
    pfd.cColorBits  = 32;
    pfd.cRedBits    = 8;
    pfd.cGreenBits  = 8;
    pfd.cBlueBits   = 8;
    pfd.cAlphaBits  = 8;
    pfd.cDepthBits  = 32;

    //choose pixel format
    iPixelFormatIndex = ChoosePixelFormat( hDC, &pfd);
    if(iPixelFormatIndex == 0)
    {
        ReleaseDC( hwnd, hDC);
        hDC = NULL;

        DestroyWindow( hwnd);
        return (false);
    }
    
    //set pixel format
    if(SetPixelFormat( hDC, iPixelFormatIndex, &pfd) == false)
    {
        ReleaseDC( hwnd, hDC);
        hDC = NULL;

        DestroyWindow( hwnd);
        return (false);
    }
    
    //get rendering context
    hglrc = wglCreateContext( hDC);
    if(hglrc == NULL)
    {
        ReleaseDC( hwnd, hDC);
        hDC = NULL;

        DestroyWindow( hwnd);
        return (false);
    }
    
    if(wglMakeCurrent(hDC, hglrc) == false)
    {
        wglDeleteContext( hglrc);
        hglrc = NULL;

        ReleaseDC(hwnd, hDC);
        hDC = NULL;
        
        DestroyWindow( hwnd);
        
        return (false);
    }

    *pHGLRC = hglrc;
   
    return( true);
}

bool InitializeOpenGLByModernMethod( HWND hwnd, HDC hDC, HGLRC *pHGLRC)
{
    // code
    const int attributes[] =
    {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        0   // End of attributes list
    };

    int pixelFormat;
    UINT numFormats;

    // code
    // we retrive pixel format index or pixel format
    if( wglChoosePixelFormatARB( hDC, attributes, nullptr, 1, &pixelFormat, &numFormats) == FALSE)
    {
        LogError( "wglChoosePixelFormatARB() Failed.");
        return false;
    }

    PIXELFORMATDESCRIPTOR pfd{};
    // we fill pfd with information for pixelFormat
    int pixel_description = DescribePixelFormat( hDC, pixelFormat, sizeof( pfd), &pfd);

    // we store pfd on to hDC
    if(SetPixelFormat( hDC, pixelFormat, &pfd) == FALSE)
    {
        LogError( "SetPixelFormat() Failed.");
        return false;
    }

#if defined(DEBUG) || defined(_DEBUG)
    /*
        OpenGL Programming Guide 9th Edition
            Appendix G. Debugging and Profiling OpenGL 
                - Create a Debug Context
     */

    int attribs[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, SS_GL_OPENGL_MAJOR_VERSION,
        WGL_CONTEXT_MINOR_VERSION_ARB, SS_GL_OPENGL_MINOR_VERSION,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
        0    // End of the array
    };

    bIsDebugOpenGLContext = true;
    LogInfo( "Setting debugging OpenGL context.");
#else

    int attribs[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, SS_GL_OPENGL_MAJOR_VERSION,
        WGL_CONTEXT_MINOR_VERSION_ARB, SS_GL_OPENGL_MINOR_VERSION,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0, 0    // End of the array
    };

    bIsDebugOpenGLContext = false;
    LogInfo( "Setting non-debugging OpenGL context.");
#endif

    // Creat OpenGL context with specified attributes
    *pHGLRC = wglCreateContextAttribsARB( hDC, 0, attribs);
    if(*pHGLRC == NULL)
    {
        LogError( "wglCreateContextAttribsARB() Failed.");
        return false;
    }

    return wglMakeCurrent( hDC, *pHGLRC) == TRUE;
}

#if defined(DEBUG) || defined(_DEBUG)
/**
 * @brief GLDebugCallbackFunction()
 */
void GLAPIENTRY GLDebugCallbackFunction( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    // variable declaration
    char *source_str = nullptr;
    char *type_str = nullptr;
    char *severity_str = nullptr;

#define ToString(x) #x

    // code
    switch( source)
    {
        case GL_DEBUG_SOURCE_API:               source_str = (char *) ToString(Api);            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     source_str = (char *) ToString(WindowSystem);   break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:   source_str = (char *) ToString(ShaderCompiler); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:       source_str = (char *) ToString(ThirdParty);     break;
        case GL_DEBUG_SOURCE_APPLICATION:       source_str = (char *) ToString(Application);    break;
        case GL_DEBUG_SOURCE_OTHER:             source_str = (char *) ToString(Other);          break;
        default:                                source_str = (char *) "Unknown";                break;
    }


    switch(type)
    {
        case GL_DEBUG_TYPE_ERROR:               type_str = (char *) ToString(Error);        break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: type_str = (char *) ToString(Deprecated);   break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  type_str = (char *) ToString(Undefined);    break;
        case GL_DEBUG_TYPE_PERFORMANCE:         type_str = (char *) ToString(Performance);  break;
        case GL_DEBUG_TYPE_PORTABILITY:         type_str = (char *) ToString(Portability);  break;
        case GL_DEBUG_TYPE_MARKER:              type_str = (char *) ToString(Marker);       break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          type_str = (char *) ToString(PushGroup);    break;
        case GL_DEBUG_TYPE_POP_GROUP:           type_str = (char *) ToString(PopGroup);     break;
        case GL_DEBUG_TYPE_OTHER:               type_str = (char *) ToString(Other);        break;
        default:                                type_str = (char *) "Unknown";              break;
    }


    switch(severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         severity_str = (char *) ToString(High);          break;
        case GL_DEBUG_SEVERITY_MEDIUM:       severity_str = (char *) ToString( Medium);       break;
        case GL_DEBUG_SEVERITY_LOW:          severity_str = (char *) ToString( Low);          break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: severity_str = (char *) ToString( Notification); break;
        default:                             severity_str = (char *) "Unknown";               break;
    }

    if( severity == GL_DEBUG_SEVERITY_NOTIFICATION)
    {
        return;
    }

    LogDebug(
        "\n[OpenGL Message]:Id: [0x%08X], Source: [%s], Type: [%s], Severity: [%s]\nMessage: %s\n",
        id, source_str, type_str, severity_str, message
    );

#undef ToString
}
#endif

/**
 * @brief InitializeImGUI()
 */
bool InitializeImGUI( HWND hwnd)
{
    // code
    IMGUI_CHECKVERSION();

    if( ImGui::CreateContext() == nullptr)
    {
        Log( "ImGui::CreateContext() Failed.\n");
        return false;
    }

    if( ImGui_ImplWin32_Init( hwnd) == false)
    {
        Log( "ImGui_ImplWin32_Init() Failed.\n");
        return false;
    }

    const char *glsl_version = "#version 130";
    if( ImGui_ImplOpenGL3_Init() == false)
    {
        Log( "ImGui_ImplOpenGL_Init() Failed.\n");
        return false;
    }

        // style
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_TitleBg]          = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive]    = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 1.0f, 0.1f);
    style.Colors[ImGuiCol_MenuBarBg]        = ImVec4(0.0f, 0.0f, 1.0f, 0.4f);
    style.Colors[ImGuiCol_Header]           = ImVec4(0.0f, 0.0f, 0.8f, 0.4f);
    style.Colors[ImGuiCol_HeaderActive]     = ImVec4(0.0f, 0.0f, 1.0f, 0.4f);
    style.Colors[ImGuiCol_HeaderHovered]    = ImVec4(0.0f, 0.0f, 1.0f, 0.4f);
    style.Colors[ImGuiCol_FrameBg]          = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_CheckMark]        = ImVec4(0.0f, 0.0f, 1.0f, 0.8f);
    style.Colors[ImGuiCol_SliderGrab]       = ImVec4(0.0f, 0.0f, 1.0f, 0.4f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.0f, 1.0f, 0.8f);
    style.Colors[ImGuiCol_FrameBgHovered]   = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
    style.Colors[ImGuiCol_FrameBgActive]    = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
    style.Colors[ImGuiCol_Button]           = ImVec4(0.0f, 0.0f, 1.0f, 0.4f);
    style.Colors[ImGuiCol_ButtonHovered]    = ImVec4(0.0f, 0.0f, 1.0f, 0.6f);
    style.Colors[ImGuiCol_ButtonActive]     = ImVec4(0.0f, 0.0f, 1.0f, 0.8f);

    return true;
}

//Initialize()
bool Initialize(void)
{
    //function declarations
    void Resize( int, int);
    void UnInitialize( void);

    //code
    RECT rc;
    GetClientRect( ghwnd, &rc);
    windowWidth = rc.right - rc.left;
    windowHeight = rc.bottom - rc.top;

    if(wglewIsSupported("WGL_ARB_create_context") == GL_FALSE)
    {
        if( !InitializeOpenGLByTraditionalMethod( ghwnd, ghdc, &ghrc))
        {
            LogError("InitializeOpenGLByTraditionalMethod() Failed.");
            DestroyWindow( ghwnd);
            return false;
        }
        LogInfo("OpenGL Context created using traditional method. Modern OpenGL features may not be available.");
    }
    else
    {
        if( InitializeOpenGLByModernMethod( ghwnd, ghdc, &ghrc) == false)
        {
            LogError("InitializeOpenGLByModernMethod() Failed.");
            DestroyWindow( ghwnd);
            return false;
        }
        LogInfo("OpenGL Context created using modern method. Modern OpenGL features are available.");
    }

        //Initialize GLEW
    // GLenum glew_error = glewInit();
    // if(glew_error != GLEW_OK)
    // {
    //     LogError("glewInit() Failed.");
    //     DestroyWindow( ghwnd);
    //     return false;
    // }

#if defined(DEBUG) || defined(_DEBUG)
    if(bIsDebugOpenGLContext)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(GLDebugCallbackFunction, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

        LogInfo("OpenGL Debug Context initialized. Debug messages will be logged.");
    }
#endif

    if(!InitializeImGUI(ghwnd))
    {
        LogError("InitializeImGUI() Failed.");
        DestroyWindow( ghwnd);
        return false;
    }

        // Initialize Application
    if(!OglApplication::Init())
    {
        LogError("OglApplication::Init() Failed.");
        DestroyWindow( ghwnd);
        return false;
    }

    //warm-up Resize call
    Resize( windowWidth, windowHeight);

    return true;
}

//Resize()
void Resize( int width, int height)
{
    //code
    if(height == 0)
    {
        height = 1;
    }

    OglApplication::Resize(width, height);
}

//Display()
void Display(double dt)
{
    // code
    OglApplication::Render(dt);

    // ImGui Rendering
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        OglApplication::RenderImGui(dt);

        ImGui::Render();
        // show on screen
        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData());

        ImGui::EndFrame();
    }

    SwapBuffers(ghdc);
}

//UnInitialize()
void UnInitialize( void)
{
    //code
    if(gbFullscreen == true)
    {
        SetWindowLong( ghwnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement( ghwnd, &wpPrev);
        SetWindowPos(
            ghwnd,
            HWND_TOP,
            0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED
        );
        
        ShowCursor(TRUE);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    OglApplication::Destroy();

    if(ghdc)
    {
        ReleaseDC( ghwnd, ghdc);
        ghdc = NULL;
    }

    LogInfo("Log File Closed...");
}

