@echo off
REM Build script for compressor
REM Requires MinGW-w64 for Windows compilation

echo Building compressor...

REM Check for MinGW
where x86_64-w64-mingw32-g++ >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Error: MinGW-w64 not found in PATH
    echo Please install MinGW-w64 or use g++ on Linux/macOS
    exit /b 1
)

REM Build for Windows
x86_64-w64-mingw32-g++ -std=c++17 -O3 -static -o compressor.exe compressor.cpp

if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    exit /b 1
)

echo Build successful! compressor.exe created
echo.
echo Usage: type input.txt ^| compressor.exe ^> output.txt
