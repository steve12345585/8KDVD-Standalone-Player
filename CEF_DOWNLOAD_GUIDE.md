# CEF Download Guide

## Manual Download Steps

Since the automated download is failing, please follow these steps:

### 1. Download CEF Binaries
1. Visit: https://cef-builds.spotifycdn.com/index.html
2. Look for the latest Windows 64-bit build (usually named like `cef_binary_XXX_windows64.tar.bz2`)
3. Download the file to `dependencies/cef/`

### 2. Extract the Archive
Once downloaded, extract the `.tar.bz2` file. You can use:
- 7-Zip
- WinRAR
- Windows built-in extraction (if supported)

### 3. Directory Structure
After extraction, you should have a structure like:
```
dependencies/cef/
├── cef_binary_XXX_windows64/
│   ├── CMakeLists.txt
│   ├── include/
│   ├── libcef_dll/
│   ├── Release/
│   ├── Debug/
│   ├── Resources/
│   └── tests/
```

### 4. Required Files for VLC Integration
We need these key components:
- `include/` - Header files
- `Release/` - DLLs and libraries for release builds
- `Debug/` - DLLs and libraries for debug builds
- `Resources/` - CEF resource files

### 5. Alternative Download Sources
If the main site is down, try:
- GitHub releases: https://github.com/chromiumembedded/cef/releases
- Mirror sites (check CEF forums)

## Next Steps
Once CEF is downloaded and extracted:
1. We'll configure VLC's CMake to find CEF
2. Add CEF include paths to VLC build
3. Link against CEF libraries
4. Implement the CEF integration code

## Current Status
- [ ] CEF binaries downloaded
- [ ] CEF extracted to correct location
- [ ] VLC CMake configuration updated
- [ ] CEF integration code implemented
