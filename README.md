# SFML Game

## Requirements

- CMake 3.28 or newer
- A C++23 compiler (Xcode Command Line Tools on macOS)
- Ninja for the included macOS presets (`brew install ninja` if needed)

SFML 3.0.2 is included in `thirdparty/SFML-3.0.2` and is built with the project.

## Build on macOS

Install and initialize Apple's command line toolchain:

```sh
xcode-select --install
sudo xcodebuild -license
```

If Xcode is installed in a non-default location, select it with
`sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer`.

From the repository root, configure and build Debug:

```sh
cmake --preset macos-debug
cmake --build --preset macos-debug
```

The executable is `build/macos-debug/mygame`. Debug builds use the repository's
absolute resource directory, so the executable can be launched from any working
directory.

For a Release build:

```sh
cmake --preset macos-release
cmake --build --preset macos-release
```

Release uses `./resources/`; launch it with the repository root as the working
directory. This relative path is suitable for local builds; packaging resources
inside a distributable macOS app bundle is not configured yet.

## Other platforms

The project can also be configured with another CMake generator, for example:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

On Windows, CMake can generate Visual Studio projects with
`-G "Visual Studio 17 2022"`.

With Visual Studio 2022 and its C++ workload installed, configure and build
using the Windows presets:

```powershell
cmake --preset windows-debug
cmake --build --preset windows-debug
```

For Release, use `windows-release` in both commands. The Windows presets target
64-bit builds; Debug uses the repository resource path, while Release expects
`resources` in the executable's working directory.
