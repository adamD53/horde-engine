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
