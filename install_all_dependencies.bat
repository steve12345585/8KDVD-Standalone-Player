@echo off
REM Automated installation of all 8KDVD build dependencies

echo ========================================
echo 8KDVD Dependencies Auto-Installer
echo ========================================
echo.
echo This script will install:
echo   - CMake
echo   - Git  
echo   - Python (if not installed)
echo   - Meson and Ninja (via pip)
echo   - vcpkg
echo   - OpenSSL, FFmpeg, Opus, libvpx (via vcpkg)
echo.
echo NOTE: Visual Studio 2022 must be installed manually
echo       Download from: https://visualstudio.microsoft.com/downloads/
echo       Select "Desktop development with C++" workload
echo.

pause

REM Check for winget
where winget.exe >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: winget is not available
    echo Please update to Windows 10 1809+ or Windows 11
    pause
    exit /b 1
)

echo.
echo [1/7] Installing CMake...
winget install Kitware.CMake --silent --accept-package-agreements --accept-source-agreements
echo.

echo [2/7] Installing Git...
winget install Git.Git --silent --accept-package-agreements --accept-source-agreements
echo.

echo [3/7] Installing Python...
winget install Python.Python.3.12 --silent --accept-package-agreements --accept-source-agreements
echo.

echo Refreshing PATH...
call refreshenv >nul 2>&1

echo [4/7] Installing Meson and Ninja...
pip install meson ninja
echo.

echo [5/7] Installing vcpkg...
if not exist "C:\vcpkg" (
    git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
    cd C:\vcpkg
    call bootstrap-vcpkg.bat
    echo.
) else (
    echo vcpkg already exists, skipping...
)

echo [6/7] Adding vcpkg to PATH permanently...
setx PATH "%PATH%;C:\vcpkg" /M
echo.

echo [7/7] Installing C++ libraries via vcpkg...
echo This may take 30-60 minutes...
C:\vcpkg\vcpkg install openssl:x64-windows
C:\vcpkg\vcpkg install ffmpeg:x64-windows
C:\vcpkg\vcpkg install opus:x64-windows
C:\vcpkg\vcpkg install libvpx:x64-windows
echo.

echo ========================================
echo Installation Complete!
echo ========================================
echo.
echo IMPORTANT NEXT STEPS:
echo.
echo 1. Install Visual Studio 2022 with C++ workload
echo    Download: https://visualstudio.microsoft.com/downloads/
echo.
echo 2. Close and reopen your command prompt
echo.
echo 3. Run: verify_build_dependencies.bat
echo.
echo 4. If all dependencies OK, run: build_8kdvd.ps1
echo.

pause

