# Horde Survival

A 2D top-down horde survival game engine built with modern C++23, SFML 3, and Dear ImGui.

---

## Features

- **Core Gameplay**:
  - Wave-based enemy spawning with scaling stats and customizable waves.
  - Directional melee combat with attack animation.
  - Dynamic smooth player camera.
  - Player HUD displaying health, wave counters, and active enemy counts.
- **Level & Wave Editor**:
  - In-game ImGui editor.
  - Switching between Play Mode and Edit Mode.
  - Wave configuration, spawner positioning, and enemy stat tuning.
  - Per-wave testing and persistence via JSON.
  - Free-camera navigation with WASD and scroll zooming.
- **Graphics & Lighting**:
  - 2D lighting system with radial lightmaps and single-pass vertex mesh optimization.
  - TMX Tiled map parser and renderer with depth sorting.
  - Batched vertex array enemy rendering.
- **Engine Architecture**:
  - State Machine separating `MenuState`, `GameState`, and `DefeatState`.
  - Object pooling for allocation-free enemy lifecycles.
  - Spatial grid for enemy flocking and collision.
  - Start menu, defeat screen, and settings stored in json file.
- **Debugging & Diagnostics**:
  - ImGui debug window and mini-HUD stats overlay.
  - Real-time frame time graph, FPS counter, world coordinates, and lighting controls.
  - Immortality toggle, heal/kill controls, and camera zoom slider.

---

## Controls

| Key / Input         | Action                                                                  |
| ------------------- | ----------------------------------------------------------------------- |
| `W`, `A`, `S`, `D`  | Move player (Play Mode) / Pan camera (Edit Mode)                        |
| `Left Mouse Button` | Melee attack towards cursor (Play Mode) / Place spawn point (Edit Mode) |
| `Mouse Wheel`       | Zoom in / out centered at cursor (Edit Mode)                            |
| `F1`                | Toggle Debug UI overlay                                                 |
| `F2`                | Toggle Level & Wave Editor                                              |
| `Escape`            | Pause / return to main menu, unfocus UI                                 |

---

## Requirements

- **CMake**: version 3.28 or newer
- **C++23 Compiler**:
  - macOS: Apple Clang 15+
  - Windows: Visual Studio 2022 MSVC 19.38+
  - Linux: GCC 13+ or Clang 17+

## Building and Running

### macOS

1. Ensure the Apple command line tools and Ninja are installed:

   ```sh
   xcode-select --install
   brew install ninja
   ```

2. Build and run (Debug):

   ```sh
   cmake --preset macos-debug
   cmake --build --preset macos-debug
   ./build/macos-debug/mygame
   ```

   _Debug builds point `RESOURCES_PATH` directly to the repository folder, allowing execution from any directory._

3. Build (Release):

   ```sh
   cmake --preset macos-release
   cmake --build --preset macos-release
   ./build/macos-release/mygame
   ```

   _Release builds set `PRODUCTION_BUILD=ON` (`RESOURCES_PATH="./resources/"`), so run the binary from the repository root._

### Windows (Visual Studio 2022)

From Developer Command Prompt or PowerShell:

```powershell
# Debug build
cmake --preset windows-debug
cmake --build --preset windows-debug
.\build\windows-debug\mygame.exe

# Release build
cmake --preset windows-release
cmake --build --preset windows-release
.\build\windows-release\mygame.exe
```

### Manual / Other Platforms

Configure with any CMake generator:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/mygame
```

Set `-DPRODUCTION_BUILD=ON` to package with relative resource paths.
