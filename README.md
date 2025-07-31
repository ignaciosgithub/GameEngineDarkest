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

### Demo Controls
- **WASD**: Camera movement (forward/left/back/right)
- **Mouse**: Look around (first-person camera)
- **1-5**: Switch between rendering pipeline test scenes
- **ESC**: Exit application

### Demo Features
- **Scene 1**: Forward rendering with colorful cubes
- **Scene 2**: Deferred rendering with G-buffer visualization
- **Scene 3**: Raytracing pipeline demonstration
- **Scene 4**: 3D physics simulation with collision detection
- **Scene 5**: 2D physics world with rigid body dynamics

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
│   ├── Core/           # ECS, Math, Platform abstraction
│   ├── Rendering/      # Graphics pipelines and rendering
│   ├── Physics/        # 3D and 2D physics systems
│   └── UI/             # Engine interface and tools
├── demo/               # Demo application
├── docs/               # Documentation and guides
└── CMakeLists.txt      # Build configuration
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

## 📚 Documentation

- [Phase 0: Meta-Planning](PHASE_0_META_PLANNING.md) - Project overview and architecture
- [Phase 2: Rendering](PHASE_2_RENDERING.md) - Rendering system documentation
- [Phase 3: Physics](PHASE_3_PHYSICS.md) - 3D physics implementation
- [Phase 4: 2D Physics](PHASE_4_2D_PHYSICS.md) - 2D physics system
- [Windows Build Guide](WINDOWS_BUILD_INSTRUCTIONS.md) - Detailed Windows setup

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
