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
endif()

# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) #link time optimization

# Ensure SFML is built as static libraries
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build SFML as static libraries" FORCE)

#for imgui-sfml
#set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/imgui-core/imgui" CACHE PATH "..." FORCE)

if(MSVC) 
	add_compile_options(/arch:AVX2) #make sure SIMD optimizations take place
endif()

#Add SFML, and here you would add other libraries if needed!
add_subdirectory(thirdparty/SFML-3.0.2)
#add_subdirectory(thirdparty/imgui-core)
#add_subdirectory(thirdparty/imgui-sfml-3.0)


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
    thirdparty/tinyxml2/tinyxml2.cpp
)

if(MSVC) # If using the VS compiler...
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PRIVATE _CRT_SECURE_NO_WARNINGS)
	set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    		VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
	)

endif()

target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC 
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/tinyxml2"
)

# Link SFML libraries statically, Here wou would add other libraries!
target_link_libraries("${CMAKE_PROJECT_NAME}" 
	sfml-graphics 
	sfml-window 
	sfml-system 
	sfml-audio 
	sfml-network
	#imgui
	#ImGui-SFML::ImGui-SFML
)

