# CEF (Chromium Embedded Framework) Dependencies

## Directory Structure
```
dependencies/cef/
├── include/          # CEF header files (cef.h, cef_app.h, etc.)
├── lib/             # CEF library files (.lib files for linking)
├── bin/             # CEF DLL files (libcef.dll, etc.)
└── resources/       # CEF resource files (locales, etc.)
```

## Required Files (to be downloaded)
- `include/cef.h` - Main CEF header
- `include/cef_app.h` - Application interface
- `include/cef_browser.h` - Browser interface
- `include/cef_client.h` - Client interface
- `lib/libcef.lib` - CEF library for linking
- `bin/libcef.dll` - CEF runtime DLL
- `bin/chrome_elf.dll` - Chrome ELF loader
- `resources/` - Locale files and other resources

## Integration with VLC
Once CEF is properly downloaded and extracted:
1. VLC CMake will be configured to find these directories
2. CEF headers will be included in VLC modules
3. CEF libraries will be linked during VLC build
4. CEF DLLs will be copied to VLC output directory

## Current Status
- [ ] CEF binaries downloaded from https://cef-builds.spotifycdn.com/
- [ ] Files extracted to correct directories
- [ ] VLC CMake configuration updated
- [ ] CEF integration code implemented
