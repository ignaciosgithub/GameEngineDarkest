@echo off
echo Building GameEngine Script with Visual Studio...

REM Set up Visual Studio environment
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

REM Compile the script
cl.exe /LD /EHsc ^
    /I"..\src" ^
    /DGAMEENGINESCRIPT_EXPORTS ^
    /std:c++20 ^
    "%1" ^
    /Fe:"%~n1.dll" ^
    /link /LIBPATH:"..\build\Release" ^
    Core.lib Rendering.lib

if %ERRORLEVEL% EQU 0 (
    echo Script compiled successfully: %~n1.dll
) else (
    echo Failed to compile script: %1
    pause
    exit /b 1
)

echo Build complete!
pause
