cmake_minimum_required(VERSION 3.28)

project(mygame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#! ! ! ! ! ! !
#set this to true to ship the game!
#basically this will change the RESOURCES_PATH to be the local path
#! ! ! ! ! ! !
#DELETE THE OUT FOLDER AFTER CHANGING THIS BECAUSE VISUAL STUDIO DOESN'T SEEM TO RECOGNIZE THIS CHANGE AND REBUILD!
option(PRODUCTION_BUILD "Use a relative resource path for a production build" OFF)

if(MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "$<$<CONFIG:Debug>:MultiThreadedDebug>$<$<CONFIG:Release>:MultiThreaded>")
	add_compile_options(/arch:AVX2) #make sure SIMD optimizations take place
endif()

# Dependencies via FetchContent
include(FetchContent)

# 1. SFML 3.1.0
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build SFML as static libraries" FORCE)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_TEST_SUITE OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        3.1.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(SFML)

# 2. tinyxml2 11.0.0
set(tinyxml2_BUILD_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    tinyxml2
    GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
    GIT_TAG        11.0.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(tinyxml2)

# 3. Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.91.8
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)
set(IMGUI_DIR "${imgui_SOURCE_DIR}" CACHE PATH "Dear ImGui directory" FORCE)

# 4. ImGui-SFML
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "Disable find_package for SFML" FORCE)
set(IMGUI_SFML_IMGUI_DEMO ON CACHE BOOL "Build ImGui demo" FORCE)

FetchContent_Declare(
    ImGui-SFML
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
    GIT_TAG        v3.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(ImGui-SFML)

# MY_SOURCES is defined to be a list of all the source files for my game 
# DON'T ADD THE SOURCES BY HAND, they are already added with this macro
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE MY_HEADERS CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_executable("${CMAKE_PROJECT_NAME}")

set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 23)

if(PRODUCTION_BUILD)
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PRIVATE RESOURCES_PATH="./resources/" PRODUCTION_BUILD=1)
else()
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PRIVATE RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/" PRODUCTION_BUILD=0)
endif()

target_sources("${CMAKE_PROJECT_NAME}" PRIVATE 
    ${MY_SOURCES}
	${MY_HEADERS}
)

if(MSVC) # If using the VS compiler...
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PRIVATE _CRT_SECURE_NO_WARNINGS)
	set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    		VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	)
endif()

target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC 
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Link dependencies
target_link_libraries("${CMAKE_PROJECT_NAME}" 
	SFML::Graphics 
	SFML::Window 
	SFML::System 
	SFML::Audio 
	SFML::Network
	tinyxml2::tinyxml2
	ImGui-SFML::ImGui-SFML
)
