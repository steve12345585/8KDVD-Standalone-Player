@echo off
setlocal enabledelayedexpansion

REM Verify all build dependencies are properly installed and in PATH

echo ========================================
echo 8KDVD Build Dependencies Verification
echo ========================================
echo.

set MISSING=0

echo Checking required tools...
echo.

REM Check Visual Studio C++ Compiler
echo [1/10] Visual Studio C++ Compiler (cl.exe)...
where cl.exe >nul 2>&1
if %errorLevel% equ 0 (
    for /f "tokens=*" %%i in ('where cl.exe') do echo   OK: %%i
) else (
    echo   MISSING - Install Visual Studio 2022 with C++ workload
    set /a MISSING+=1
)
echo.

REM Check CMake
echo [2/10] CMake...
where cmake.exe >nul 2>&1
if %errorLevel% equ 0 (
    for /f "tokens=*" %%i in ('cmake --version ^| findstr /C:"version"') do echo   OK: %%i
    for /f "tokens=*" %%i in ('where cmake.exe') do echo   Path: %%i
) else (
    echo   MISSING - Run: winget install Kitware.CMake
    set /a MISSING+=1
)
echo.

REM Check Python
echo [3/10] Python...
where python.exe >nul 2>&1
if %errorLevel% equ 0 (
    for /f "tokens=*" %%i in ('python --version') do echo   OK: %%i
    for /f "tokens=*" %%i in ('where python.exe') do echo   Path: %%i
) else (
    echo   MISSING - Check if Miniconda/Anaconda is installed
    if exist "C:\Users\%USERNAME%\miniconda3\python.exe" (
        echo   Found Miniconda but not in PATH - Run setup_complete_build_environment.bat
    ) else if exist "C:\Users\%USERNAME%\anaconda3\python.exe" (
        echo   Found Anaconda but not in PATH - Run setup_complete_build_environment.bat
    ) else (
        echo   Install from: https://www.python.org/downloads/
    )
    set /a MISSING+=1
)
echo.

REM Check pip
echo [4/10] pip...
where pip.exe >nul 2>&1
if %errorLevel% equ 0 (
    for /f "tokens=*" %%i in ('pip --version') do echo   OK: %%i
) else (
    echo   MISSING - Should come with Python
    set /a MISSING+=1
)
echo.

REM Check Meson
echo [5/10] Meson Build System...
where meson.exe >nul 2>&1
if %errorLevel% equ 0 (
    for /f "tokens=*" %%i in ('meson --version') do echo   OK: Version %%i
    for /f "tokens=*" %%i in ('where meson.exe') do echo   Path: %%i
) else (
    echo   MISSING - Run: pip install meson
    set /a MISSING+=1
)
echo.

REM Check Ninja
echo [6/10] Ninja Build System...
where ninja.exe >nul 2>&1
if %errorLevel% equ 0 (
    for /f "tokens=*" %%i in ('ninja --version') do echo   OK: Version %%i
    for /f "tokens=*" %%i in ('where ninja.exe') do echo   Path: %%i
) else (
    echo   MISSING - Run: pip install ninja
    set /a MISSING+=1
)
echo.

REM Check vcpkg
echo [7/10] vcpkg Package Manager...
where vcpkg.exe >nul 2>&1
if %errorLevel% equ 0 (
    for /f "tokens=*" %%i in ('vcpkg version ^| findstr /C:"version"') do echo   OK: %%i
    for /f "tokens=*" %%i in ('where vcpkg.exe') do echo   Path: %%i
) else (
    echo   MISSING - Clone and build vcpkg from GitHub
    set /a MISSING+=1
)
echo.

REM Check Git
echo [8/10] Git...
where git.exe >nul 2>&1
if %errorLevel% equ 0 (
    for /f "tokens=*" %%i in ('git --version') do echo   OK: %%i
    for /f "tokens=*" %%i in ('where git.exe') do echo   Path: %%i
) else (
    echo   MISSING - Run: winget install Git.Git
    set /a MISSING+=1
)
echo.

REM Check for OpenSSL (vcpkg)
echo [9/10] OpenSSL (via vcpkg)...
if exist "C:\vcpkg\installed\x64-windows\include\openssl" (
    echo   OK: Found in C:\vcpkg\installed\x64-windows\include\openssl
) else if exist "C:\Users\%USERNAME%\source\repos\vcpkg\installed\x64-windows\include\openssl" (
    echo   OK: Found in C:\Users\%USERNAME%\source\repos\vcpkg\installed\x64-windows\include\openssl
) else (
    echo   MISSING - Run: vcpkg install openssl:x64-windows
    set /a MISSING+=1
)
echo.

REM Check for FFmpeg (vcpkg)
echo [10/10] FFmpeg (via vcpkg)...
if exist "C:\vcpkg\installed\x64-windows\include\libavcodec" (
    echo   OK: Found in C:\vcpkg\installed\x64-windows\include\libavcodec
) else if exist "C:\Users\%USERNAME%\source\repos\vcpkg\installed\x64-windows\include\libavcodec" (
    echo   OK: Found in C:\Users\%USERNAME%\source\repos\vcpkg\installed\x64-windows\include\libavcodec
) else (
    echo   MISSING - Run: vcpkg install ffmpeg:x64-windows
    set /a MISSING+=1
)
echo.

REM Summary
echo ========================================
echo Verification Summary
echo ========================================
echo.

if %MISSING% equ 0 (
    echo ALL DEPENDENCIES OK!
    echo You are ready to build 8KDVD Player!
    echo.
    echo Next steps:
    echo   1. Run: cd vlc-master\vlc-master
    echo   2. Run: meson setup builddir --prefix=C:/vlc-8kdvd
    echo   3. Run: meson compile -C builddir
) else (
    echo MISSING DEPENDENCIES: %MISSING%
    echo Please install missing dependencies and run this script again
    echo.
    echo Quick install commands:
    echo   - Visual Studio: https://visualstudio.microsoft.com/downloads/
    echo   - CMake: winget install Kitware.CMake
    echo   - Python: https://www.python.org/downloads/
    echo   - Meson/Ninja: pip install meson ninja
    echo   - vcpkg: git clone https://github.com/microsoft/vcpkg.git C:\vcpkg ^&^& cd C:\vcpkg ^&^& bootstrap-vcpkg.bat
    echo   - Git: winget install Git.Git
    echo   - OpenSSL: vcpkg install openssl:x64-windows
    echo   - FFmpeg: vcpkg install ffmpeg:x64-windows
)
echo.

pause

