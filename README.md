# GameEngineDarkest

## Windows Build (Visual Studio 2022, x64)
- Use “x64 Native Tools Command Prompt for VS 2022” or Developer PowerShell
- Commands:
  ```
  mkdir build && cd build
  cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release ..
  cmake --build . --config Release -j
  ```
- Or open build\GameEngineDarkest.sln in Visual Studio, select “Release” + “x64”, then Build Solution.

Outputs:
- Editor: build\bin\GameEngineEditor.exe
- Demos:  build\demo\GameEngineDemo.exe
          build\demo\GameObjectDemo.exe

Optional script:
- PowerShell: scripts\build_windows.ps1
  ```
  pwsh scripts\build_windows.ps1 -Config Release
  ```

Notes for Windows:
- If MSVC reports M_PI undeclared errors in some files, add:
  ```
  #ifndef M_PI
  #define M_PI 3.14159265358979323846
  #endif
  ```
  to the affected source files.
- If you see “OpenGL header already included … glad already provides it”, ensure GLAD is included before any OpenGL headers across Core and Rendering modules.
- If you see size_t→GLsizei or double→float conversion warnings treated as errors, add explicit casts where needed.
- If ImGui linking errors occur (cannot open imgui.lib), verify CMake finds and links ImGui, or integrate via vcpkg and set CMAKE_TOOLCHAIN_FILE accordingly.
# GameEngineDarkest

A modular 3D/2D game engine built with C++20, featuring Unity-comparable functionality with production-ready architecture. The engine follows ECS (Entity-Component-System) design principles and supports multiple rendering pipelines, comprehensive physics simulation, and cross-platform deployment.

## 🚀 Features

### Core Engine
- **ECS Architecture**: Entity-Component-System with modular design
- **Cross-Platform**: Windows, macOS, Linux support
- **Memory Management**: Custom memory allocators and leak detection
- **Math Library**: Optimized Vector3, Matrix4, Quaternion operations
- **Logging System**: Multi-level logging with file output

### Rendering System
- **Multiple Pipelines**: Forward, Deferred, and Raytracing rendering
- **OpenGL Backend**: OpenGL 4.5+ with modern shader support
- **Camera System**: 3D navigation with WASD controls and mouse look
- **Mesh Rendering**: Optimized vertex/index buffer management
- **Lighting**: Directional, point, and spot light support
- **Post-Processing**: Extensible post-processing stack

### Physics Simulation
- **3D Physics**: Rigid body dynamics with collision detection
- **2D Physics**: Separate 2D physics world with specialized colliders
- **Spatial Partitioning**: Octree (3D) and QuadTree (2D) optimization
- **Collision Detection**: Continuous and discrete collision detection
- **Physics Materials**: Configurable friction and restitution

### Development Tools
- **Engine UI**: ImGui-based editor interface
- **Scene Management**: Hierarchical scene graph
- **Asset Pipeline**: Texture and mesh loading
- **Build System**: CMake with vcpkg dependency management

## 📋 System Requirements

### Minimum Requirements
- **OS**: Windows 10/11 (64-bit), Ubuntu 20.04+, macOS 10.15+
- **Graphics**: DirectX 11 or OpenGL 4.5 compatible GPU
- **RAM**: 4GB minimum, 8GB recommended
- **Storage**: 2GB free disk space
- **Compiler**: Visual Studio 2022, GCC 10+, or Clang 12+

### Development Dependencies
- **CMake**: 3.20 or higher
- **vcpkg**: Microsoft C++ package manager
- **Git**: Version control system

## 🛠️ Installation

### Quick Start (Windows)

1. **Install Prerequisites**
   ```cmd
   # Install Visual Studio 2022 with C++ workload
   # Install Git for Windows
   
   # Clone and setup vcpkg
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg integrate install
   ```

2. **Clone and Build**
   ```cmd
   git clone https://github.com/ignaciosgithub/GameEngineDarkest.git
   cd GameEngineDarkest
   mkdir build && cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake
   cmake --build . --config Release
   ```

3. **Run Demo**
   ```cmd
   cd demo
   .\GameEngineDemo.exe
   ```

### Linux Installation

1. **Install Dependencies**
   ```bash
   sudo apt update
   sudo apt install build-essential cmake git pkg-config
   sudo apt install libgl1-mesa-dev libglu1-mesa-dev
   ```

2. **Setup vcpkg**
   ```bash
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   ./bootstrap-vcpkg.sh
   ./vcpkg integrate install
   ```

3. **Build Project**
   ```bash
   git clone https://github.com/ignaciosgithub/GameEngineDarkest.git
   cd GameEngineDarkest
   mkdir build && cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake
   make -j$(nproc)
   ```

## 🎮 Usage

### Controls
- WASD: Intended FPS-style movement (W/S forward/back, A/D strafe left/right)
- Mouse: Look around (first-person camera)
- 1-5: Switch between rendering/pipeline test scenes
- ESC: Exit application

Known issue
- A/D may rotate instead of strafing in current builds. This is actively being addressed (see branch: fix-wasd-controls-and-json-saving). The intended behavior is standard FPS strafing.

### Demo Features
- **Scene 1**: Forward rendering with colorful cubes
- **Scene 2**: Deferred rendering with G-buffer visualization
- **Scene 3**: Raytracing pipeline demonstration
- **Scene 4**: 3D physics simulation with collision detection
- **Scene 5**: 2D physics world with rigid body dynamics

## 🧰 Unity-like Editor

The engine includes an ImGui-based editor focused on a Unity-like workflow.

Panels
- Hierarchy: Manage scene objects and parent/child relationships
- Inspector: View and edit selected entity components and properties
- Viewport: Scene view with camera controls and gizmos
- Console: Engine logs and diagnostics
- Project: File browser for assets and project files

Play and Edit Modes
- Edit Mode: Modify scenes, components, and assets
- Play Mode: Run the simulation to test gameplay logic
- Switching: Toggle between Edit and Play using the engine UI (PlayModeManager)

Project Workflow (JSON)
- Create: Create a new project directory with default structure
- Open: Launch the editor with a project path argument to load an existing project
  - Example: editor/GameEngineEditor &lt;path-to-project&gt;
- Save: Save current project settings and scene data to JSON

Asset Import
- OBJ import supported via the Project panel
- Imported meshes and textures integrate with the engine asset pipeline

Default Scene
- Camera, cube, and light are set up as a simple starting point for quick iteration

## 🧩 External C++ Scripting (Hot Reload)

Write gameplay logic as external C++ scripts that are compiled to DLL/SO and hot-reloaded at runtime.

Quick start
- Templates and examples: Scripts/ScriptTemplate.cpp, Scripts/ExampleCubeRotator.cpp
- Build helpers: Scripts/BUILD_SCRIPT_LINUX.sh, Scripts/ScriptTemplate.vcxproj
- Integration and API docs: see VISUAL_STUDIO_INTEGRATION.md

Runtime behavior
- The engine watches the Scripts/ directory for compiled modules and loads them
- Implement IExternalScript with OnStart, OnUpdate, OnDestroy to control entities and components



## 🏗️ Architecture

The engine follows a modular architecture with clear separation of concerns:

```
┌─────────────────────────────────────────────────────────┐
│                    Application Layer                    │
├─────────────────────────────────────────────────────────┤
│  Editor Tools  │  Scripting  │  Networking  │  Audio   │
├─────────────────────────────────────────────────────────┤
│      UI        │  Animation  │   Physics    │ Rendering │
├─────────────────────────────────────────────────────────┤
│                     Core Engine                         │
│  (ECS, Math, Memory, Platform, Input, Resources)       │
└─────────────────────────────────────────────────────────┘
```

### Core Modules
- **Core**: ECS framework, math library, platform abstraction
- **Rendering**: Multi-pipeline renderer with OpenGL backend
- **Physics**: 3D/2D physics with spatial optimization
- **UI**: ImGui-based engine interface

## 🔧 Development

### Building from Source

The project uses CMake with vcpkg for dependency management. All required dependencies are automatically installed:

- **Graphics**: OpenGL, GLAD, GLFW3
- **Math**: GLM (OpenGL Mathematics)
- **UI**: ImGui with OpenGL bindings
- **Testing**: GoogleTest framework

### Project Structure
```
GameEngineDarkest/
├── src/
│   ├── Core/            # ECS, Math, Platform, Input, Resources, Editor runtime (PlayModeManager), Project
│   ├── Rendering/       # Pipelines (Forward/Deferred/Raytracing), Shaders, Framebuffers, Materials, Lights
│   ├── Physics/         # 3D and 2D physics, Colliders, Spatial structures
│   └── UI/              # ImGui renderer and panels (Hierarchy, Inspector, Viewport, Console, Project)
├── editor/              # Editor application entry (GameEngineEditor)
├── demo/                # Demo applications (GameEngineDemo, GameObjectDemo)
├── Scripts/             # External scripting templates, examples, and build helpers
├── scripts/             # Build scripts (e.g., Windows build helper)
├── docs/                # Documentation and guides
├── *.md                 # Additional documentation (build guides, phase docs)
└── CMakeLists.txt       # Build configuration (root and per-module)
```

### Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📊 Development Status

### Completed Features ✅
- [x] Core ECS architecture
- [x] Cross-platform build system
- [x] Forward rendering pipeline
- [x] Deferred rendering pipeline
- [x] Raytracing pipeline (basic)
- [x] 3D physics with collision detection
- [x] 2D physics system
- [x] WASD camera navigation
- [x] Engine UI framework
- [x] Editor panels (Hierarchy, Inspector, Viewport, Console, Project)
- [x] Project JSON create/load/save (via ProjectManager and EngineUI)
- [x] OBJ model import support
- [x] Shadow mapping (directional, spot, and point/cubemap)


### In Progress 🚧
- [ ] Advanced lighting models (PBR)
- [ ] Animation system
- [ ] Audio system integration
- [ ] C# scripting layer
- [ ] Asset import pipeline

### Planned Features 📋
- [ ] Networking system
- [ ] Mobile platform support
- [ ] WebAssembly deployment
- [ ] Advanced editor tools
- [ ] Sample projects and tutorials

## 🐛 Troubleshooting

### Common Issues

**Build Fails on Windows**
- Ensure Visual Studio 2022 is installed with C++ workload
- Verify vcpkg path is correct in CMake command
- Check that Windows SDK is available

**Runtime Crashes**
- Update graphics drivers to latest version
- Ensure OpenGL 4.5+ support is available
- Try running in Release configuration

**Performance Issues**
- Use Release build configuration
- Close unnecessary background applications
- Verify dedicated GPU is being used

For detailed troubleshooting, see [WINDOWS_BUILD_INSTRUCTIONS.md](WINDOWS_BUILD_INSTRUCTIONS.md).

## ⚠️ Known Issues

- FPS controls: A/D may rotate instead of strafing in some builds. Work in progress on branch fix-wasd-controls-and-json-saving.
- Windows (MSVC):
  - Define M_PI in files where missing
  - Ensure GLAD is included before any OpenGL headers
  - Add explicit casts to resolve warnings-as-errors for size_t→GLsizei, double→float

  - Prefer secure CRT alternatives (e.g., strcpy_s, localtime_s) where needed
- Headless Linux: Running without DISPLAY will fail GLFW initialization; this is expected in CI/headless environments (see LINUX_BUILD_STATUS.md)

## 📚 Documentation

- [Phase 0: Meta-Planning](PHASE_0_META_PLANNING.md) - Project overview and architecture
- [Phase 2: Rendering](PHASE_2_RENDERING.md) - Rendering system documentation
- [Phase 3: Physics](PHASE_3_PHYSICS.md) - 3D physics implementation
- [Phase 4: 2D Physics](PHASE_4_2D_PHYSICS.md) - 2D physics system
- [Windows Build Guide](WINDOWS_BUILD_INSTRUCTIONS.md) - Detailed Windows setup
- [Linux Build Status](LINUX_BUILD_STATUS.md) - Current Linux build and runtime notes
- [Visual Studio Integration](VISUAL_STUDIO_INTEGRATION.md) - External C++ scripting and hot reload

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **OpenGL**: Graphics rendering API
- **GLFW**: Window and input management
- **GLM**: Mathematics library for graphics
- **ImGui**: Immediate mode GUI framework
- **vcpkg**: C++ package management

---

**GameEngineDarkest** - Building the future of game development, one module at a time.

For questions, issues, or contributions, please visit our [GitHub repository](https://github.com/ignaciosgithub/GameEngineDarkest).
