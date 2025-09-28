# CEF Setup Script for 8KDVD Player
# This script downloads and sets up CEF dependencies

Write-Host "Setting up CEF dependencies for 8KDVD Player..." -ForegroundColor Green

# Create directories
$cefDir = "dependencies\cef"
$includeDir = "$cefDir\include"
$binDir = "$cefDir\bin"
$libDir = "$cefDir\lib"
$resourcesDir = "$cefDir\resources"

Write-Host "Creating directory structure..." -ForegroundColor Yellow
New-Item -ItemType Directory -Path $cefDir -Force | Out-Null
New-Item -ItemType Directory -Path $includeDir -Force | Out-Null
New-Item -ItemType Directory -Path $binDir -Force | Out-Null
New-Item -ItemType Directory -Path $libDir -Force | Out-Null
New-Item -ItemType Directory -Path $resourcesDir -Force | Out-Null

# CEF download URL (latest as of setup)
$cefUrl = "https://cef-builds.spotifycdn.com/cef_binary_140.1.14%2Bgeb1c06e%2Bchromium-140.0.7339.185_windows64.tar.bz2"
$cefArchive = "$cefDir\cef_binary_windows64.tar.bz2"

Write-Host "Downloading CEF binaries (this may take a while)..." -ForegroundColor Yellow
Write-Host "URL: $cefUrl" -ForegroundColor Gray

try {
    Invoke-WebRequest -Uri $cefUrl -OutFile $cefArchive -UseBasicParsing
    Write-Host "Download completed!" -ForegroundColor Green
} catch {
    Write-Host "Download failed: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "Please download manually from: https://cef-builds.spotifycdn.com/" -ForegroundColor Yellow
    exit 1
}

Write-Host "Extracting CEF binaries..." -ForegroundColor Yellow
try {
    # Extract using tar (available in Windows 10+)
    Set-Location $cefDir
    tar -xf "cef_binary_windows64.tar.bz2"
    Set-Location "..\.."
    
    # Get the extracted folder name
    $extractedFolder = Get-ChildItem -Path $cefDir -Directory | Where-Object { $_.Name -like "cef_binary_*" } | Select-Object -First 1
    
    if ($extractedFolder) {
        Write-Host "Organizing CEF files..." -ForegroundColor Yellow
        
        # Copy files to organized structure
        Copy-Item "$($extractedFolder.FullName)\include\*" "$includeDir\" -Recurse -Force
        Copy-Item "$($extractedFolder.FullName)\Release\*" "$binDir\" -Recurse -Force
        Copy-Item "$($extractedFolder.FullName)\libcef_dll\*" "$libDir\" -Recurse -Force
        Copy-Item "$($extractedFolder.FullName)\Resources\*" "$resourcesDir\" -Recurse -Force
        
        Write-Host "CEF setup completed successfully!" -ForegroundColor Green
        Write-Host "Files organized in:" -ForegroundColor Cyan
        Write-Host "  - $includeDir (headers)" -ForegroundColor Gray
        Write-Host "  - $binDir (DLLs)" -ForegroundColor Gray
        Write-Host "  - $libDir (libraries)" -ForegroundColor Gray
        Write-Host "  - $resourcesDir (resources)" -ForegroundColor Gray
        
        # Clean up
        Remove-Item $cefArchive -Force
        Write-Host "Cleanup completed." -ForegroundColor Green
    } else {
        Write-Host "Error: Could not find extracted CEF folder" -ForegroundColor Red
        exit 1
    }
} catch {
    Write-Host "Extraction failed: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "Please extract manually using 7-Zip or similar tool" -ForegroundColor Yellow
    exit 1
}

Write-Host "`nCEF setup complete! You can now build the 8KDVD Player." -ForegroundColor Green
