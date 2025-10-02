@echo off
REM Install Meson and Ninja via pip

echo ========================================
echo Installing Meson and Ninja
echo ========================================
echo.

echo Checking Python installation...
where python.exe >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: Python not found in PATH
    echo Please run setup_complete_build_environment.bat first
    pause
    exit /b 1
)

echo Found Python:
python --version
echo.

echo Installing Meson and Ninja via pip...
pip install meson ninja

echo.
echo Verifying installation...
where meson.exe >nul 2>&1
if %errorLevel% equ 0 (
    echo   OK: Meson installed
    meson --version
) else (
    echo   ERROR: Meson not found after installation
)

where ninja.exe >nul 2>&1
if %errorLevel% equ 0 (
    echo   OK: Ninja installed
    ninja --version
) else (
    echo   ERROR: Ninja not found after installation
)

echo.
echo ========================================
echo Installation Complete
echo ========================================
echo.
echo If meson/ninja are not found, you may need to:
echo 1. Close and reopen your terminal
echo 2. Or run: setup_complete_build_environment.bat
echo.

pause
