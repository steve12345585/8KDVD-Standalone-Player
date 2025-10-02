@echo off
REM Clean up the PATH registry - removes duplicates and fixes it

echo ========================================
echo Cleaning up PATH registry
echo ========================================

REM Check for Administrator privileges
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo ERROR: This script must be run as Administrator
    echo Right-click and select "Run as administrator"
    pause
    exit /b 1
)

echo Setting clean PATH...

reg add "HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Environment" /v Path /t REG_EXPAND_SZ /d "C:\Program Files\Microsoft\jdk-17.0.16.8-hotspot\bin;C:\Program Files\Java\jdk-17\bin;c:\Users\drive\AppData\Local\Programs\cursor\resources\app\bin;C:\Ruby32-x64\bin;C:\Program Files\Common Files\Oracle\Java\javapath;C:\Program Files (x86)\Common Files\Intel\Shared Libraries\redist\intel64\compiler;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;C:\WINDOWS\System32\WindowsPowerShell\v1.0\;C:\WINDOWS\System32\OpenSSH\;C:\Program Files (x86)\NVIDIA Corporation\PhysX\Common;C:\ProgramData\chocolatey\bin;C:\Program Files\dotnet\;C:\Users\drive\AppData\Local\Microsoft\WindowsApps;C:\Program Files\Microsoft SQL Server\Client SDK\ODBC\170\Tools\Binn\;C:\Program Files\NVIDIA Corporation\NVIDIA App\NvDLISR;C:\Program Files\PuTTY\;C:\Program Files (x86)\Windows Kits\10\Windows Performance Toolkit\;C:\Program Files\Microsoft SQL Server\150\Tools\Binn\;C:\Program Files\Git\cmd;C:\Program Files\Microsoft SQL Server\130\Tools\Binn\;C:\Program Files\CMake\bin;C:\Program Files (x86)\QuickTime\QTSystem\;C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.16.27023\bin\HostX64\x64;C:\Users\drive\source\repos\vcpkg;C:\Users\drive\miniconda3;C:\Users\drive\miniconda3\Scripts;C:\Users\drive\miniconda3\condabin;C:\Users\drive\miniconda3\envs\openwebui\Scripts;C:\Users\drive\AppData\Local\Packages\PythonSoftwareFoundation.Python.3.12_qbz5n2kfra8p0\LocalCache\local-packages\Python312\Scripts;C:\cygwin64\bin" /f

echo.
echo ========================================
echo PATH cleaned up!
echo ========================================
echo.
echo Close ALL terminals and open a NEW one
echo Then run: verify_build_dependencies.bat
echo.

pause
