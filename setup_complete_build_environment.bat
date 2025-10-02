@echo off
setlocal enabledelayedexpansion

REM ============================================
REM 8KDVD Complete Build Environment Setup
REM Adds ALL Visual Studio and Cygwin tools to PATH permanently
REM ============================================

echo ========================================
echo 8KDVD Complete Build Environment Setup
echo ========================================
echo.

REM Check for Administrator privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: This script must be run as Administrator
    echo Right-click and select "Run as administrator"
    pause
    exit /b 1
)

echo Scanning for build tools...
echo.

set "NEW_PATHS="

REM ============================================
REM CYGWIN TOOLS
REM ============================================
echo [CYGWIN]
echo Checking C:\cygwin64...

if exist "C:\cygwin64\bin" (
    echo   Found: C:\cygwin64\bin
    echo     - gcc, g++, make, and Unix tools
    set "NEW_PATHS=!NEW_PATHS!C:\cygwin64\bin;"
)

if exist "C:\cygwin64\usr\bin" (
    echo   Found: C:\cygwin64\usr\bin
    set "NEW_PATHS=!NEW_PATHS!C:\cygwin64\usr\bin;"
)

if exist "C:\cygwin64\usr\local\bin" (
    echo   Found: C:\cygwin64\usr\local\bin
    set "NEW_PATHS=!NEW_PATHS!C:\cygwin64\usr\local\bin;"
)

echo.

REM ============================================
REM VISUAL STUDIO MSVC COMPILER
REM ============================================
echo [VISUAL STUDIO]
echo Checking for MSVC compiler...

set VS_FOUND=0
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC" (
    for /f "delims=" %%v in ('dir /b /ad /o-n "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC"') do (
        set "MSVC_VER=%%v"
        set "MSVC_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\!MSVC_VER!\bin\Hostx64\x64"
        if exist "!MSVC_PATH!\cl.exe" (
            echo   Found: !MSVC_PATH!
            echo     - cl.exe, link.exe, lib.exe
            set "NEW_PATHS=!NEW_PATHS!!MSVC_PATH!;"
            set VS_FOUND=1
            goto :vs_done
        )
    )
)
:vs_done

if %VS_FOUND%==0 (
    echo   NOT FOUND - Install Visual Studio 2022 with C++ workload
)

echo.

REM ============================================
REM VISUAL STUDIO BUILD TOOLS
REM ============================================
echo [VISUAL STUDIO BUILD TOOLS]

if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE" (
    echo   Found: C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE
    set "NEW_PATHS=!NEW_PATHS!C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE;"
)

if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools" (
    echo   Found: C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools
    set "NEW_PATHS=!NEW_PATHS!C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools;"
)

if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin" (
    echo   Found: C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin
    echo     - msbuild.exe
    set "NEW_PATHS=!NEW_PATHS!C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin;"
)

echo.

REM ============================================
REM VISUAL STUDIO CMAKE
REM ============================================
echo [VISUAL STUDIO CMAKE]

if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin" (
    echo   Found: Visual Studio CMake
    set "NEW_PATHS=!NEW_PATHS!C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;"
)

echo.

REM ============================================
REM WINDOWS SDK
REM ============================================
echo [WINDOWS SDK]
echo Checking for Windows 10/11 SDK...

set SDK_FOUND=0
if exist "C:\Program Files (x86)\Windows Kits\10\bin" (
    for /f "delims=" %%s in ('dir /b /ad /o-n "C:\Program Files (x86)\Windows Kits\10\bin\10.*"') do (
        set "SDK_VER=%%s"
        set "SDK_PATH=C:\Program Files (x86)\Windows Kits\10\bin\!SDK_VER!\x64"
        if exist "!SDK_PATH!" (
            echo   Found: !SDK_PATH!
            echo     - rc.exe, mt.exe, signtool.exe
            set "NEW_PATHS=!NEW_PATHS!!SDK_PATH!;"
            set SDK_FOUND=1
            goto :sdk_done
        )
    )
)
:sdk_done

if %SDK_FOUND%==0 (
    echo   NOT FOUND - Included with Visual Studio
)

echo.

REM ============================================
REM CMAKE (Standalone)
REM ============================================
echo [CMAKE]

if exist "C:\Program Files\CMake\bin" (
    echo   Found: C:\Program Files\CMake\bin
    set "NEW_PATHS=!NEW_PATHS!C:\Program Files\CMake\bin;"
) else (
    echo   NOT FOUND - Run: winget install Kitware.CMake
)

echo.

REM ============================================
REM PYTHON
REM ============================================
echo [PYTHON]

set PYTHON_FOUND=0

REM Check Miniconda/Anaconda
if exist "C:\Users\%USERNAME%\miniconda3\python.exe" (
    echo   Found: C:\Users\%USERNAME%\miniconda3
    set "NEW_PATHS=!NEW_PATHS!C:\Users\%USERNAME%\miniconda3;C:\Users\%USERNAME%\miniconda3\Scripts;C:\Users\%USERNAME%\miniconda3\condabin;"
    set PYTHON_FOUND=1
    goto :python_done
)

if exist "C:\Users\%USERNAME%\anaconda3\python.exe" (
    echo   Found: C:\Users\%USERNAME%\anaconda3
    set "NEW_PATHS=!NEW_PATHS!C:\Users\%USERNAME%\anaconda3;C:\Users\%USERNAME%\anaconda3\Scripts;C:\Users\%USERNAME%\anaconda3\condabin;"
    set PYTHON_FOUND=1
    goto :python_done
)

REM Check AppData
if exist "%LOCALAPPDATA%\Programs\Python" (
    for /f "delims=" %%p in ('dir /b /ad /o-n "%LOCALAPPDATA%\Programs\Python\Python*"') do (
        set "PYTHON_DIR=%LOCALAPPDATA%\Programs\Python\%%p"
        if exist "!PYTHON_DIR!\python.exe" (
            echo   Found: !PYTHON_DIR!
            set "NEW_PATHS=!NEW_PATHS!!PYTHON_DIR!;!PYTHON_DIR!\Scripts;"
            set PYTHON_FOUND=1
            goto :python_done
        )
    )
)

REM Check Program Files
if exist "C:\Program Files\Python*" (
    for /f "delims=" %%p in ('dir /b /ad /o-n "C:\Program Files\Python*"') do (
        set "PYTHON_DIR=C:\Program Files\%%p"
        if exist "!PYTHON_DIR!\python.exe" (
            echo   Found: !PYTHON_DIR!
            set "NEW_PATHS=!NEW_PATHS!!PYTHON_DIR!;!PYTHON_DIR!\Scripts;"
            set PYTHON_FOUND=1
            goto :python_done
        )
    )
)

REM Check C:\Python
for /f "delims=" %%p in ('dir /b /ad /o-n "C:\Python*" 2^>nul') do (
    set "PYTHON_DIR=C:\%%p"
    if exist "!PYTHON_DIR!\python.exe" (
        echo   Found: !PYTHON_DIR!
        set "NEW_PATHS=!NEW_PATHS!!PYTHON_DIR!;!PYTHON_DIR!\Scripts;"
        set PYTHON_FOUND=1
        goto :python_done
    )
)

:python_done

if %PYTHON_FOUND%==0 (
    echo   NOT FOUND - Download from: https://www.python.org/downloads/
)

echo.

REM ============================================
REM MESON AND NINJA
REM ============================================
echo [MESON AND NINJA]

set MESON_FOUND=0
set NINJA_FOUND=0

REM Check if meson/ninja are already in PATH
where meson.exe >nul 2>&1
if %errorLevel% equ 0 (
    echo   Found: Meson already in PATH
    set MESON_FOUND=1
)

where ninja.exe >nul 2>&1
if %errorLevel% equ 0 (
    echo   Found: Ninja already in PATH
    set NINJA_FOUND=1
)

REM If not found, search common locations
if %MESON_FOUND%==0 (
    echo   Searching for Meson...
    REM Check Python Scripts directories
    if exist "C:\Users\%USERNAME%\miniconda3\Scripts\meson.exe" (
        echo   Found: C:\Users\%USERNAME%\miniconda3\Scripts\meson.exe
        set "NEW_PATHS=!NEW_PATHS!C:\Users\%USERNAME%\miniconda3\Scripts;"
        set MESON_FOUND=1
    )
    if exist "C:\Users\%USERNAME%\anaconda3\Scripts\meson.exe" (
        echo   Found: C:\Users\%USERNAME%\anaconda3\Scripts\meson.exe
        set "NEW_PATHS=!NEW_PATHS!C:\Users\%USERNAME%\anaconda3\Scripts;"
        set MESON_FOUND=1
    )
    REM Check Cygwin
    if exist "C:\cygwin64\bin\meson.exe" (
        echo   Found: C:\cygwin64\bin\meson.exe
        set "NEW_PATHS=!NEW_PATHS!C:\cygwin64\bin;"
        set MESON_FOUND=1
    )
)

if %NINJA_FOUND%==0 (
    echo   Searching for Ninja...
    REM Check Python Scripts directories
    if exist "C:\Users\%USERNAME%\miniconda3\Scripts\ninja.exe" (
        echo   Found: C:\Users\%USERNAME%\miniconda3\Scripts\ninja.exe
        set "NEW_PATHS=!NEW_PATHS!C:\Users\%USERNAME%\miniconda3\Scripts;"
        set NINJA_FOUND=1
    )
    if exist "C:\Users\%USERNAME%\anaconda3\Scripts\ninja.exe" (
        echo   Found: C:\Users\%USERNAME%\anaconda3\Scripts\ninja.exe
        set "NEW_PATHS=!NEW_PATHS!C:\Users\%USERNAME%\anaconda3\Scripts;"
        set NINJA_FOUND=1
    )
    REM Check Cygwin
    if exist "C:\cygwin64\bin\ninja.exe" (
        echo   Found: C:\cygwin64\bin\ninja.exe
        set "NEW_PATHS=!NEW_PATHS!C:\cygwin64\bin;"
        set NINJA_FOUND=1
    )
)

if %MESON_FOUND%==0 (
    echo   NOT FOUND - Install with: pip install meson
)

if %NINJA_FOUND%==0 (
    echo   NOT FOUND - Install with: pip install ninja
)

echo.

REM ============================================
REM VCPKG
REM ============================================
echo [VCPKG]

set VCPKG_FOUND=0

if exist "C:\vcpkg\vcpkg.exe" (
    echo   Found: C:\vcpkg
    set "NEW_PATHS=!NEW_PATHS!C:\vcpkg;"
    set VCPKG_FOUND=1
) else if exist "C:\Users\%USERNAME%\source\repos\vcpkg\vcpkg.exe" (
    echo   Found: C:\Users\%USERNAME%\source\repos\vcpkg
    set "NEW_PATHS=!NEW_PATHS!C:\Users\%USERNAME%\source\repos\vcpkg;"
    set VCPKG_FOUND=1
) else if exist "%USERPROFILE%\vcpkg\vcpkg.exe" (
    echo   Found: %USERPROFILE%\vcpkg
    set "NEW_PATHS=!NEW_PATHS!%USERPROFILE%\vcpkg;"
    set VCPKG_FOUND=1
)

if %VCPKG_FOUND%==0 (
    echo   NOT FOUND - Clone from: https://github.com/microsoft/vcpkg.git
)

echo.

REM ============================================
REM GIT
REM ============================================
echo [GIT]

if exist "C:\Program Files\Git\bin" (
    echo   Found: C:\Program Files\Git\bin
    set "NEW_PATHS=!NEW_PATHS!C:\Program Files\Git\bin;"
) else if exist "C:\Program Files\Git\cmd" (
    echo   Found: C:\Program Files\Git\cmd
    set "NEW_PATHS=!NEW_PATHS!C:\Program Files\Git\cmd;"
) else (
    echo   NOT FOUND - Run: winget install Git.Git
)

echo.

REM ============================================
REM ADD ALL TO SYSTEM PATH
REM ============================================
echo ========================================
echo Adding to System PATH...
echo ========================================
echo.

REM Get current PATH
for /f "tokens=2*" %%a in ('reg query "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path 2^>nul') do set "CURRENT_PATH=%%b"

REM Combine with new paths
set "FINAL_PATH=%CURRENT_PATH%;%NEW_PATHS%"

REM Remove duplicate semicolons
set "FINAL_PATH=!FINAL_PATH:;;=;!"

REM Set the new PATH
reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path /t REG_EXPAND_SZ /d "!FINAL_PATH!" /f >nul

echo.
echo ========================================
echo PATH Updated Successfully!
echo ========================================
echo.
echo The following directories have been added to your PATH:
echo.
echo !NEW_PATHS:;=^

!

echo.
echo ========================================
echo IMPORTANT
echo ========================================
echo.
echo 1. CLOSE ALL COMMAND PROMPTS AND TERMINALS
echo 2. Open a NEW command prompt to use the updated PATH
echo 3. Run: verify_build_dependencies.bat
echo.
echo The PATH will persist across system reboots.
echo.

pause

