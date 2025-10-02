# Setting Up Build Environment PATH Permanently

This guide explains how to permanently add all build dependencies to your Windows PATH so they're available across all sessions and reboots.

## Quick Start

1. **Run the setup script as Administrator:**
   ```
   Right-click setup_complete_build_environment.bat → Run as administrator
   ```

2. **Close ALL terminals/command prompts**

3. **Open a NEW command prompt and verify:**
   ```
   verify_build_dependencies.bat
   ```

## What Gets Added to PATH

### From Cygwin (C:\cygwin64)
- `C:\cygwin64\bin` - GCC, G++, make, and all Unix tools
- `C:\cygwin64\usr\bin` - Additional Unix utilities
- `C:\cygwin64\usr\local\bin` - Local builds

### From Visual Studio 2022
- **MSVC Compiler** (Latest version found):
  - `C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\{VERSION}\bin\Hostx64\x64`
  - Includes: `cl.exe`, `link.exe`, `lib.exe`

- **Build Tools**:
  - `C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE`
  - `C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools`
  - `C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin` - MSBuild

- **CMake**:
  - `C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin`

### From Windows SDK
- **Windows 10/11 SDK** (Latest version found):
  - `C:\Program Files (x86)\Windows Kits\10\bin\{VERSION}\x64`
  - Includes: `rc.exe`, `mt.exe`, `signtool.exe`

### Standalone Tools
- **CMake**: `C:\Program Files\CMake\bin`
- **Python**: Auto-detected from:
  - `%LOCALAPPDATA%\Programs\Python\Python*`
  - `C:\Program Files\Python*`
  - `C:\Python*`
- **Python Scripts**: `{PYTHON_DIR}\Scripts` (for pip-installed tools like meson, ninja)
- **vcpkg**: Auto-detected from:
  - `C:\vcpkg`
  - `C:\Users\{USERNAME}\source\repos\vcpkg`
  - `%USERPROFILE%\vcpkg`
- **Git**: `C:\Program Files\Git\bin` or `C:\Program Files\Git\cmd`

## Build Tools Available After Setup

### Compilers
- **GCC/G++** (Cygwin) - For Unix-style builds
- **cl.exe** (MSVC) - For Windows native builds

### Build Systems
- **make** (Cygwin) - GNU Make
- **ninja** (Cygwin or pip) - Fast build system
- **meson** (pip) - Modern build system (required for VLC)
- **cmake** (Standalone or VS) - Cross-platform build system
- **MSBuild** (Visual Studio) - Microsoft build engine

### Development Tools
- **pkg-config** - Package configuration
- **autoconf/automake** (Cygwin) - GNU build tools
- **libtool** (Cygwin) - Library building
- **rc.exe** (Windows SDK) - Resource compiler
- **mt.exe** (Windows SDK) - Manifest tool
- **signtool.exe** (Windows SDK) - Code signing

### Package Managers
- **vcpkg** - C++ library manager (OpenSSL, FFmpeg, etc.)
- **pip** - Python package manager

### Version Control
- **git** - Source control

## Verifying the Setup

After running the setup script and opening a NEW terminal:

```batch
REM Check individual tools
where cl.exe
where gcc.exe
where cmake.exe
where meson.exe
where ninja.exe
where vcpkg.exe
where git.exe

REM Or run the comprehensive verification
verify_build_dependencies.bat
```

## Required Dependencies for 8KDVD Build

### Must Install via vcpkg:
```batch
vcpkg install openssl:x64-windows
vcpkg install ffmpeg:x64-windows
vcpkg install opus:x64-windows
vcpkg install libvpx:x64-windows
```

### Must Install via pip:
```batch
pip install meson ninja
```

## Troubleshooting

### "Tool not found" after setup
- Make sure you **closed and reopened** your terminal
- System PATH changes require a new session to take effect
- Reboot if the tool still isn't found

### "Access denied" when running setup
- Run the `.bat` file as **Administrator**
- Right-click → "Run as administrator"

### Multiple versions of a tool
- The script adds the latest version found
- If you need a specific version, manually edit your PATH:
  - System Properties → Environment Variables → System variables → Path

### Cygwin tools conflict with Windows tools
- Cygwin is added FIRST in PATH, so Cygwin tools take precedence
- If you need Windows versions first, manually reorder PATH entries

### vcpkg not found
- Install vcpkg first:
  ```batch
  git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
  cd C:\vcpkg
  bootstrap-vcpkg.bat
  ```
- Then re-run the setup script

## Manual PATH Editing

If you need to manually edit your PATH:

1. Windows Key + R → `sysdm.cpl` → Advanced → Environment Variables
2. Under "System variables", find `Path`
3. Click Edit
4. Add/Remove/Reorder entries
5. Click OK on all dialogs
6. Close and reopen terminals

## Build Command Examples

Once setup is complete, you can use any build system:

### Using Meson (VLC build):
```batch
cd vlc-master\vlc-master
meson setup builddir --prefix=C:/vlc-8kdvd
meson compile -C builddir
```

### Using CMake:
```batch
cd vlc-master\vlc-master\modules\input\8kdvd
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### Using GCC (Cygwin):
```batch
cd vlc-master\vlc-master\modules\input\8kdvd
gcc -c *.c -I../../../include
```

### Using MSVC:
```batch
cd vlc-master\vlc-master\modules\input\8kdvd
cl /c *.c /I..\..\..\include
```

## Next Steps

1. ✅ Run `setup_complete_build_environment.bat` as Admin
2. ✅ Close all terminals
3. ✅ Open new terminal and run `verify_build_dependencies.bat`
4. ✅ Install vcpkg dependencies: `vcpkg install openssl:x64-windows ffmpeg:x64-windows`
5. ✅ Install Python build tools: `pip install meson ninja`
6. ✅ Start building: See `BUILD_GUIDE.md`

---

**Ready to build!** 🚀

