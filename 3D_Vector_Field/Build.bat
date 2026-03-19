@echo off
cls

@REM call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
setlocal EnableDelayedExpansion

REM =================================================
REM ANSI Colors
REM =================================================
REM Color	Standard(Dark)	Bright(Vibrant)
REM  Black	    30	        90
REM  Red	    31	        91
REM  Green	    32	        92
REM  Yellow	    33	        93
REM  Blue	    34	        94
REM  Magenta	35	        95
REM  Cyan	    36	        96
REM  White	    37	        97
for /F %%a in ('echo prompt $E ^| cmd') do set "ESC=%%a"
set DARK_RED=%ESC%[31m
set DARK_YELLOW=%ESC%[33m

set RED=%ESC%[91m
set GREEN=%ESC%[92m
set YELLOW=%ESC%[93m
set BLUE=%ESC%[94m
set MAGENTA=%ESC%[95m
set CYAN=%ESC%[96m

set RESET=%ESC%[0m

REM =================================================
REM Config
REM =================================================
set OutputFileName=App.exe
set GLEW_INCLUDE_PATH=third_party\glew\include
set GLEW_LIB_PATH=third_party\glew\lib\Release\x64
set RUN_AS_CONSOLE=1
set OBJDIR_Release=build\release\objs
set OBJDIR_Debug=build\debug\objs

@REM 1. Get the Absolute Path of the current directory to ensure consistency
set "ROOT_DIR=%~dp0"
@REM Remove trailing backslash if it exists (except for drive roots)
if "%ROOT_DIR:~-1%"=="\" set "ROOT_DIR=%ROOT_DIR:~0,-1%"

REM Clean/ Debug / Release
if /I "%1"=="clean" (
    echo %YELLOW%Cleaning build artifacts...%RESET%
    if exist "%OBJDIR_Release%" rd /s /q "%OBJDIR_Release%"
    if exist "%OBJDIR_Debug%" rd /s /q "%OBJDIR_Debug%"
    if exist "%OutputFileName%" del "%OutputFileName%"
    echo %GREEN%Clean complete.%RESET%
    exit /b 0
) else if /I "%1"=="debug" (
    set BUILD_MODE=Debug
    set OPT_FLAGS=/Zi /Od /DDEBUG
    set OBJDIR=%OBJDIR_Debug%
) else (
    set BUILD_MODE=Release
    set OPT_FLAGS=/O2 /DNDEBUG
    set OBJDIR=%OBJDIR_Release%
)

if not exist "%OBJDIR%" mkdir "%OBJDIR%"

echo %CYAN%=== Build Mode: %BUILD_MODE% ===%RESET%

REM =================================================
REM Compiler Flags
REM =================================================
set COMMON_FLAGS=/nologo /EHsc /MD /MP %OPT_FLAGS% ^
 /I"%GLEW_PATH%\Include" ^
 /I"include" ^
 /I"third_party" ^
 /I"." ^
 /DCONSOLE_APPLICATION=%RUN_AS_CONSOLE% ^
 /Fd"%OBJDIR%\vc.pdb"

REM =================================================
REM Source Files
REM =================================================
set SOURCES=^
 Main.cpp ^
 OGLApplication.cpp ^
 source\Geometry.cpp ^
 source\OBJModel.cpp ^
 source\ShaderProgram.cpp ^
 source\TextureLoading.cpp ^
 third_party\imgui\imgui.cpp ^
 third_party\imgui\imgui_draw.cpp ^
 third_party\imgui\imgui_impl_opengl3.cpp ^
 third_party\imgui\imgui_impl_win32.cpp ^
 third_party\imgui\imgui_tables.cpp ^
 third_party\imgui\imgui_widgets.cpp

echo.
echo ===== COMPILING RESOURCE =====
set RESOURCE_COMPILATION_FAILED=0
RC.exe Resource.h
if errorlevel 1 (
    echo %RED%Resource Compilation Failed. Fallback To Default%RESET%
    set RESOURCE_COMPILATION_FAILED=1
)

echo.
echo ===== COMPILING =====
set COMPILATION_FAILED=0

for %%S in (%SOURCES%) do (
    set SRC=%%S
    REM ---- Unique object name ----
    set REL=!SRC!
    set REL=!REL:\=_!
    set REL=!REL:.cpp=.obj!
    set REL=!REL:.c=.obj!
    set OBJ=%OBJDIR%\!REL!

    echo %YELLOW%[BUILD]%RESET% !SRC!

    cl.exe %COMMON_FLAGS% /c "!SRC!" /Fo"!OBJ!" 2>&1
    if errorlevel 1 (
        set COMPILATION_FAILED=1
        echo %RED%[FAIL] !SRC!
    ) else (
        echo %GREEN%[OK]%RESET% !SRC!
    )
)

if %COMPILATION_FAILED%==1 (
    echo.
    echo %RED%Build failed. Skipping link.%RESET%
    echo.

    exit /b 1
)

REM =================================================
REM Linking
REM =================================================
echo.
echo ===== LINKING =====

if %RESOURCE_COMPILATION_FAILED%==0 (
    set ResourcePath="Resource.res"
) else (
    set ResourcePath=
)

echo %CYAN%!ResourcePath!%RESET%

@REM ⚖️ Should You Use /DEBUG in Release?
@REM ✅ Pros
@REM You can debug crashes from user machines
@REM Call stacks in crash dumps are readable
@REM No runtime performance cost

link.exe /nologo /DEBUG ^
 /OUT:"%OBJDIR%\%OutputFileName%" ^
 /LIBPATH:"%GLEW_LIB_PATH%" ^
 "%OBJDIR%\*.obj" ^
 !ResourcePath! ^
 user32.lib gdi32.lib opengl32.lib glew32.lib

if errorlevel 1 goto BUILD_FAILED

copy "%OBJDIR%\%OutputFileName%" .

echo.
echo %GREEN%===== BUILD FINISHED SUCCESSFULLY =====%RESET%

Copy third_party\glew\lib\Release\x64\glew32.dll .
goto :eof

:BUILD_FAILED
echo.
echo %RED%===== LINK FAILED =====%RESET%
echo.
exit /b 1
