# CEF Download Script
# This script downloads the latest CEF binaries for Windows 64-bit

$cefVersion = "120.1.10+g3ce3184+chromium-120.0.6099.109"
$cefUrl = "https://cef-builds.spotifycdn.com/cef_binary_$($cefVersion)_windows64.tar.bz2"
$outputFile = "cef_binary_windows64.tar.bz2"

Write-Host "Downloading CEF version: $cefVersion"
Write-Host "URL: $cefUrl"

try {
    # Download with progress
    $ProgressPreference = 'Continue'
    Invoke-WebRequest -Uri $cefUrl -OutFile $outputFile -UseBasicParsing
    Write-Host "Download completed successfully!"
    
    # Check if file was downloaded
    if (Test-Path $outputFile) {
        $fileSize = (Get-Item $outputFile).Length
        Write-Host "File size: $([math]::Round($fileSize/1MB, 2)) MB"
    }
} catch {
    Write-Host "Error downloading CEF: $($_.Exception.Message)"
    Write-Host "Trying alternative download method..."
    
    # Try with curl if available
    try {
        curl -L -o $outputFile $cefUrl
        Write-Host "Download completed with curl!"
    } catch {
        Write-Host "Both PowerShell and curl failed. Please download manually from:"
        Write-Host "https://cef-builds.spotifycdn.com/index.html"
    }
}
