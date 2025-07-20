# Windows Build Instructions

## Prerequisites

### Required Software
1. **Visual Studio 2022** (Community Edition or higher)
   - Install with "Desktop development with C++" workload
   - Ensure Windows 10/11 SDK is included

2. **Git for Windows**
   - Download from: https://git-scm.com/download/win

3. **vcpkg** (C++ Package Manager)
   ```cmd
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg integrate install
   ```

### System Requirements
- Windows 10/11 (64-bit)
- DirectX 11 compatible graphics card
- 4GB RAM minimum, 8GB recommended
- 2GB free disk space

## Build Steps

### 1. Clone Repository
```cmd
git clone https://github.com/ignaciosgithub/GameEngineDarkest.git
cd GameEngineDarkest
```

### 2. Install Dependencies
The project uses vcpkg for dependency management. Dependencies will be automatically installed during CMake configuration.

Required packages (automatically handled):
- OpenGL
- GLFW3
- GLEW
- GLM

### 3. Generate Visual Studio Solution
```cmd
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake
```

Replace `[path-to-vcpkg]` with your actual vcpkg installation path.

### 4. Build the Project
Option A - Command Line:
```cmd
cmake --build . --config Release
```

Option B - Visual Studio:
1. Open `GameEngineDarkest.sln` in Visual Studio
2. Set build configuration to "Release"
3. Build → Build Solution (Ctrl+Shift+B)

### 5. Run the Demo
```cmd
cd demo
.\GameEngineDemo.exe
```

## Controls
- **WASD**: Move camera
- **Mouse**: Look around
- **1-5**: Switch between rendering pipeline test scenes
- **ESC**: Exit application

## Features Demonstrated
- 3D navigation with WASD controls
- Multiple rendering pipelines (Deferred, Forward, Raytracing)
- 3D physics simulation with collision detection
- 2D physics system with rigid body dynamics
- Spatial partitioning for efficient collision detection

## Troubleshooting

### Common Issues

**CMake Configuration Fails**
- Ensure vcpkg path is correct in the cmake command
- Verify Visual Studio 2022 is installed with C++ workload

**Build Errors**
- Check that Windows SDK is installed
- Ensure all dependencies are properly installed via vcpkg

**Runtime Errors**
- Verify graphics drivers are up to date
- Ensure DirectX 11 is available on the system

**Performance Issues**
- Switch to Release build configuration
- Update graphics drivers
- Close unnecessary background applications

### Debug Build
For development, use Debug configuration:
```cmd
cmake --build . --config Debug
```

## Architecture Overview
The engine follows a modular ECS (Entity-Component-System) architecture:

- **Core**: ECS framework, math library, platform abstraction
- **Rendering**: OpenGL renderer with multiple pipeline support
- **Physics**: 3D and 2D physics systems with spatial partitioning
- **UI**: Engine interface (temporarily disabled)

## Next Steps
After successful build and testing:
1. Explore the demo scenes using number keys 1-5
2. Test WASD navigation and mouse look
3. Observe physics simulation in action
4. Review the modular architecture for expansion

For development questions or issues, please create an issue on the GitHub repository.
