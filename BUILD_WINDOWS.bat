@echo off
echo GameEngine Darkest - Windows Build Script
echo ========================================

REM Check if vcpkg is available
where vcpkg >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: vcpkg not found in PATH
    echo Please install vcpkg and add it to your PATH
    echo See WINDOWS_BUILD_INSTRUCTIONS.md for details
    pause
    exit /b 1
)

REM Create build directory
if not exist build mkdir build
cd build

echo Configuring CMake...
cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed
    pause
    exit /b 1
)

echo Building project...
cmake --build . --config Release
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed
    pause
    exit /b 1
)

echo Build completed successfully!
echo Run demo\GameEngineDemo.exe to test the engine
pause
