# SFML Top-Down RPG Engine

This repository hosts a 2D game engine core built with **SFML** and modern **C++**. The architecture emphasizes graphics optimization through **Batched Rendering** and clean, separated systems. The game is still not playable and is in the early phase of development.

### Key Architectural Features

1.  **Optimized Tilemap Renderer (Batching):** The core feature is a custom tilemap rendering pipeline that drastically reduces draw calls.
      * **Multi-Layer Geometry:** The `World` class correctly utilizes separate `sf::VertexArray` buffers for each layer (batching all tiles per layer per tileset).
      * **TMX Parsing:** Map structures are loaded from **Tiled Map Editor (.tmx)** files using the efficient **TinyXML2** library.

### Dependencies

  * **SFML 3.0.2** (Graphics, Window, System)
  * **tinyxml2** (Single-file)

### Terrain collision

Player movement uses a 10-by-6-pixel hitbox at the feet. Solid tiles stop movement;
the player can slide along their edges, including during long frames. Enemies and
projectiles do not use terrain collision yet.

Collision is authored in `resources/world/map.tmx` using embedded Tiled tilesets:

* Set a tileset's custom boolean property `solid` to `true` to block its tiles.
* A tile's own `solid` property overrides the tileset default. Grass tiles in the
  floor atlas (local IDs 276–278) are marked `false`.
* Tile collision rectangles from Tiled's collision editor override `solid`.
  Tree tiles are walkable except for rectangles on the trunk-base and stump tiles.
  Rectangles must be unrotated and fit inside their 16-by-16 tile; polygons and
  ellipses are not supported. Collision from different map layers is combined.

The current loader supports finite CSV maps with embedded, unflipped atlas tiles.
After editing collision in Tiled, restart the game to reload the map. The map's
image paths refer to the PNG files beside it so the tilesets open locally.

Build and run the movement and map collision checks with:

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

### Screenshots from the current state of the game

<img width="1280" height="720" alt="image" src="https://github.com/user-attachments/assets/7e86634c-55d0-44e1-9fa4-410225211204" />

-----

### Building Instructions (Terminal CMake)

#### Requirements

  * C++ Compiler (GCC, Clang, MSVC) with support for **C++17/C++20**.
  * **CMake 3.10+**.

#### Step 1: Clone and Configure

```bash
git clone https://github.com/adamD53/sfml-game.git
cd sfml-game
mkdir build
cd build
```

#### Step 2: Generate Build Files

Generate the necessary build files.

```bash
cmake .. 
```
Add ```-G "Visual Studio 17 2022"``` to generate solution file.

#### Step 3: Compile the Project

Use the generic build command to compile the executable.

```bash
cmake --build . 
```
