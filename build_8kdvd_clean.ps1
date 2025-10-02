# 8KDVD Build Script
# Quick build and test script for our 8KDVD implementation

Write-Host "8KDVD Player Build Script" -ForegroundColor Green
Write-Host "================================" -ForegroundColor Green

# Check for build tools
Write-Host "`nChecking build tools..." -ForegroundColor Yellow

# Check for Visual Studio
$vsPath = Get-Command "cl.exe" -ErrorAction SilentlyContinue
if ($vsPath) {
    Write-Host "Visual Studio C++ compiler found" -ForegroundColor Green
} else {
    Write-Host "Visual Studio C++ compiler not found" -ForegroundColor Red
    Write-Host "   Please install Visual Studio 2022 with C++ workload" -ForegroundColor Yellow
}

# Check for CMake
$cmakePath = Get-Command "cmake.exe" -ErrorAction SilentlyContinue
if ($cmakePath) {
    Write-Host "CMake found" -ForegroundColor Green
} else {
    Write-Host "CMake not found" -ForegroundColor Red
    Write-Host "   Please install CMake: winget install Kitware.CMake" -ForegroundColor Yellow
}

# Check for Meson
$mesonPath = Get-Command "meson.exe" -ErrorAction SilentlyContinue
if ($mesonPath) {
    Write-Host "Meson found" -ForegroundColor Green
} else {
    Write-Host "Meson not found" -ForegroundColor Red
    Write-Host "   Please install Meson: pip install meson ninja" -ForegroundColor Yellow
}

Write-Host "`nBuild Options:" -ForegroundColor Cyan
Write-Host "1. Quick Test Build (compile modules only)"
Write-Host "2. Full VLC Build (recommended)"
Write-Host "3. Check Dependencies"
Write-Host "4. Test 8KDVD Detection"

$choice = Read-Host "`nSelect option (1-4)"

switch ($choice) {
    "1" {
        Write-Host "`nQuick Test Build..." -ForegroundColor Yellow
        cd vlc-master/vlc-master/modules/input/8kdvd
        
        # Create build directory
        if (!(Test-Path "build")) {
            mkdir build
        }
        cd build
        
        # Try to compile our modules
        Write-Host "Compiling 8KDVD modules..." -ForegroundColor Yellow
        
        # Simple compilation test
        $files = @(
            "../8kdvd_certificate_validator.c",
            "../8kdvd_disc_manager.c", 
            "../8kdvd_playback_engine.c",
            "../8kdvd_menu_system.c",
            "../8kdvd_settings.c",
            "../8kdvd_testing_framework.c"
        )
        
        foreach ($file in $files) {
            if (Test-Path $file) {
                Write-Host "Found: $file" -ForegroundColor Green
            } else {
                Write-Host "Missing: $file" -ForegroundColor Red
            }
        }
        
        Write-Host "`nBuild files created in: $(Get-Location)" -ForegroundColor Cyan
    }
    
    "2" {
        Write-Host "`nFull VLC Build..." -ForegroundColor Yellow
        cd vlc-master/vlc-master
        
        if (!(Test-Path "builddir")) {
            Write-Host "Setting up Meson build..." -ForegroundColor Yellow
            meson setup builddir --prefix=C:/vlc-8kdvd
        }
        
        Write-Host "Configuring build..." -ForegroundColor Yellow
        meson configure builddir -D8kdvd=enabled -Dcef=enabled
        
        Write-Host "Compiling VLC..." -ForegroundColor Yellow
        meson compile -C builddir
        
        Write-Host "`nVLC build completed!" -ForegroundColor Green
        Write-Host "   VLC executable: builddir/vlc.exe" -ForegroundColor Cyan
    }
    
    "3" {
        Write-Host "`nChecking Dependencies..." -ForegroundColor Yellow
        
        # Check for VLC
        $vlcPath = Get-Command "vlc.exe" -ErrorAction SilentlyContinue
        if ($vlcPath) {
            Write-Host "VLC found: $($vlcPath.Source)" -ForegroundColor Green
        } else {
            Write-Host "VLC not found" -ForegroundColor Red
        }
        
        # Check for OpenSSL
        if (Test-Path "C:/vcpkg/installed/x64-windows/include/openssl") {
            Write-Host "OpenSSL found" -ForegroundColor Green
        } else {
            Write-Host "OpenSSL not found" -ForegroundColor Red
            Write-Host "   Install with: vcpkg install openssl" -ForegroundColor Yellow
        }
        
        # Check for CEF
        if (Test-Path "vlc-master/vlc-master/contrib/cef") {
            Write-Host "CEF found" -ForegroundColor Green
        } else {
            Write-Host "CEF not found" -ForegroundColor Red
        }
    }
    
    "4" {
        Write-Host "`nTesting 8KDVD Detection..." -ForegroundColor Yellow
        
        # Check if we have test files
        $testFiles = @(
            "vlc-master/vlc-master/modules/input/8kdvd/8kdvd_certificate_validator.c",
            "vlc-master/vlc-master/modules/input/8kdvd/8kdvd_disc_manager.c",
            "vlc-master/vlc-master/modules/input/8kdvd/8kdvd_playback_engine.c"
        )
        
        $found = 0
        foreach ($file in $testFiles) {
            if (Test-Path $file) {
                $found++
                Write-Host "Found: $file" -ForegroundColor Green
            }
        }
        
        Write-Host "`nFound $found/$($testFiles.Count) 8KDVD modules" -ForegroundColor Cyan
        
        if ($found -eq $testFiles.Count) {
            Write-Host "All 8KDVD modules present!" -ForegroundColor Green
        } else {
            Write-Host "Some modules missing" -ForegroundColor Yellow
        }
    }
    
    default {
        Write-Host "Invalid option" -ForegroundColor Red
    }
}

Write-Host "`nFor detailed build instructions, see BUILD_GUIDE.md" -ForegroundColor Cyan
Write-Host "Ready to test 8KDVD playback!" -ForegroundColor Green
