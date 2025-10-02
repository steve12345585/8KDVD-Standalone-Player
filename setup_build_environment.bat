@echo off
REM 8KDVD Build Environment Setup
REM This script permanently adds all build dependencies to Windows PATH

echo ========================================
echo 8KDVD Build Environment Setup
echo ========================================
echo.

REM Check if running as Administrator
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: This script must be run as Administrator
    echo Right-click and select "Run as administrator"
    pause
    exit /b 1
)

echo Checking for build dependencies...
echo.

REM ============================================
REM 1. Visual Studio 2022 C++ Compiler (cl.exe)
REM ============================================
echo [1/8] Checking Visual Studio C++ compiler...

set "VS_PATHS[0]=C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC"
set "VS_PATHS[1]=C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\MSVC"
set "VS_PATHS[2]=C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Tools\MSVC"
set "VS_PATHS[3]=C:\Program Files\Microsoft Visual Studio\2022\BuildTools\VC\Tools\MSVC"

set VS_FOUND=0
for /L %%i in (0,1,3) do (
    if exist "!VS_PATHS[%%i]!" (
        for /d %%v in ("!VS_PATHS[%%i]!\*") do (
            set "VS_BIN=%%v\bin\Hostx64\x64"
            if exist "!VS_BIN!\cl.exe" (
                echo   Found: !VS_BIN!
                setx PATH "%PATH%;!VS_BIN!" /M >nul
                set VS_FOUND=1
                goto :vs_done
            )
        )
    )
)
:vs_done

if %VS_FOUND%==0 (
    echo   NOT FOUND - Please install Visual Studio 2022 with C++ workload
    echo   Download: https://visualstudio.microsoft.com/downloads/
) else (
    echo   OK - Visual Studio added to PATH
)
echo.

REM ============================================
REM 2. CMake
REM ============================================
echo [2/8] Checking CMake...

where cmake.exe >nul 2>&1
if %errorLevel% equ 0 (
    echo   OK - CMake already in PATH
) else (
    if exist "C:\Program Files\CMake\bin\cmake.exe" (
        echo   Found: C:\Program Files\CMake\bin
        setx PATH "%PATH%;C:\Program Files\CMake\bin" /M >nul
        echo   OK - CMake added to PATH
    ) else if exist "C:\Program Files (x86)\CMake\bin\cmake.exe" (
        echo   Found: C:\Program Files (x86)\CMake\bin
        setx PATH "%PATH%;C:\Program Files (x86)\CMake\bin" /M >nul
        echo   OK - CMake added to PATH
    ) else (
        echo   NOT FOUND - Installing CMake...
        echo   Run: winget install Kitware.CMake
    )
)
echo.

REM ============================================
REM 3. Python and pip
REM ============================================
echo [3/8] Checking Python/pip...

where python.exe >nul 2>&1
if %errorLevel% equ 0 (
    echo   OK - Python already in PATH
) else (
    if exist "C:\Python3\python.exe" (
        echo   Found: C:\Python3
        setx PATH "%PATH%;C:\Python3;C:\Python3\Scripts" /M >nul
        echo   OK - Python added to PATH
    ) else if exist "%LOCALAPPDATA%\Programs\Python" (
        for /d %%p in ("%LOCALAPPDATA%\Programs\Python\Python*") do (
            echo   Found: %%p
            setx PATH "%PATH%;%%p;%%p\Scripts" /M >nul
            echo   OK - Python added to PATH
            goto :python_done
        )
    ) else (
        echo   NOT FOUND - Please install Python
        echo   Download: https://www.python.org/downloads/
    )
)
:python_done
echo.

REM ============================================
REM 4. Meson Build System
REM ============================================
echo [4/8] Checking Meson...

where meson.exe >nul 2>&1
if %errorLevel% equ 0 (
    echo   OK - Meson already in PATH
) else (
    if exist "%LOCALAPPDATA%\Programs\Python\Python*\Scripts\meson.exe" (
        for /d %%p in ("%LOCALAPPDATA%\Programs\Python\Python*\Scripts") do (
            echo   Found: %%p
            setx PATH "%PATH%;%%p" /M >nul
            echo   OK - Meson added to PATH
            goto :meson_done
        )
    ) else (
        echo   NOT FOUND - Install with: pip install meson ninja
    )
)
:meson_done
echo.

REM ============================================
REM 5. Ninja Build System
REM ============================================
echo [5/8] Checking Ninja...

where ninja.exe >nul 2>&1
if %errorLevel% equ 0 (
    echo   OK - Ninja already in PATH
) else (
    echo   NOT FOUND - Install with: pip install ninja
)
echo.

REM ============================================
REM 6. vcpkg Package Manager
REM ============================================
echo [6/8] Checking vcpkg...

where vcpkg.exe >nul 2>&1
if %errorLevel% equ 0 (
    echo   OK - vcpkg already in PATH
) else (
    if exist "C:\vcpkg\vcpkg.exe" (
        echo   Found: C:\vcpkg
        setx PATH "%PATH%;C:\vcpkg" /M >nul
        echo   OK - vcpkg added to PATH
    ) else if exist "C:\Users\%USERNAME%\source\repos\vcpkg\vcpkg.exe" (
        echo   Found: C:\Users\%USERNAME%\source\repos\vcpkg
        setx PATH "%PATH%;C:\Users\%USERNAME%\source\repos\vcpkg" /M >nul
        echo   OK - vcpkg added to PATH
    ) else if exist "%USERPROFILE%\vcpkg\vcpkg.exe" (
        echo   Found: %USERPROFILE%\vcpkg
        setx PATH "%PATH%;%USERPROFILE%\vcpkg" /M >nul
        echo   OK - vcpkg added to PATH
    ) else (
        echo   NOT FOUND - Please install vcpkg
        echo   Clone: git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
        echo   Build: cd C:\vcpkg && bootstrap-vcpkg.bat
    )
)
echo.

REM ============================================
REM 7. Git (for vcpkg and VLC source)
REM ============================================
echo [7/8] Checking Git...

where git.exe >nul 2>&1
if %errorLevel% equ 0 (
    echo   OK - Git already in PATH
) else (
    if exist "C:\Program Files\Git\bin\git.exe" (
        echo   Found: C:\Program Files\Git\bin
        setx PATH "%PATH%;C:\Program Files\Git\bin" /M >nul
        echo   OK - Git added to PATH
    ) else (
        echo   NOT FOUND - Install with: winget install Git.Git
    )
)
echo.

REM ============================================
REM 8. Windows SDK (for build tools)
REM ============================================
echo [8/8] Checking Windows SDK...

if exist "C:\Program Files (x86)\Windows Kits\10\bin" (
    for /d %%w in ("C:\Program Files (x86)\Windows Kits\10\bin\10.*") do (
        set "WIN_SDK=%%w\x64"
        echo   Found: !WIN_SDK!
        setx PATH "%PATH%;!WIN_SDK!" /M >nul
        echo   OK - Windows SDK added to PATH
        goto :sdk_done
    )
) else (
    echo   Included with Visual Studio
)
:sdk_done
echo.

REM ============================================
REM Summary
REM ============================================
echo ========================================
echo Setup Complete!
echo ========================================
echo.
echo The following tools should now be in your PATH:
echo   - Visual Studio C++ Compiler (cl.exe)
echo   - CMake
echo   - Python/pip
echo   - Meson
echo   - Ninja
echo   - vcpkg
echo   - Git
echo   - Windows SDK tools
echo.
echo IMPORTANT: Close and reopen your command prompt for changes to take effect
echo.
echo Next steps:
echo   1. Install vcpkg dependencies: vcpkg install openssl:x64-windows ffmpeg:x64-windows
echo   2. Install Meson/Ninja: pip install meson ninja
echo   3. Run build_8kdvd.ps1 or build_8kdvd_clean.ps1
echo.

pause

