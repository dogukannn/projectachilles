cmake_minimum_required(VERSION 3.15)

project(ProjectAchilles)

set(EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/External)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Dependencies
# ------------

set(ENET_STATIC 1)
add_subdirectory(${EXTERNAL_DIR}/enet)

# SDL2 (Minimal setup)
set(SDL_ATOMIC OFF CACHE BOOL "" FORCE)
set(SDL_CPUINFO OFF CACHE BOOL "" FORCE)
set(SDL_EVENTS ON CACHE BOOL "" FORCE)
set(SDL_FILE OFF CACHE BOOL "" FORCE)
set(SDL_FILESYSTEM OFF CACHE BOOL "" FORCE)
set(SDL_HAPTIC OFF CACHE BOOL "" FORCE)
set(SDL_LOCALE OFF CACHE BOOL "" FORCE)
set(SDL_POWER OFF CACHE BOOL "" FORCE)
set(SDL_RENDER OFF CACHE BOOL "" FORCE)
set(SDL_SENSOR OFF CACHE BOOL "" FORCE)
set(SDL_TIMERS OFF CACHE BOOL "" FORCE)
set(SDL_DLOPEN OFF CACHE BOOL "" FORCE)
set(SDL_AUDIO OFF CACHE BOOL "" FORCE)
set(SDL_LOADSO ON  CACHE BOOL "" FORCE)
set(SDL_VIDEO ON  CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON  CACHE BOOL "" FORCE)
set(SDL_STATIC_PIC ON CACHE BOOL "" FORCE)
add_subdirectory(${EXTERNAL_DIR}/SDL)

#add_subdirectory(${EXTERNAL_DIR}/DirectX-Headers)
#add_subdirectory(${EXTERNAL_DIR}/DirectXTK12)

add_subdirectory(${EXTERNAL_DIR}/tinyobjloader)
add_subdirectory(${EXTERNAL_DIR}/glm)

# imgui
set(IMGUI_DIR ${EXTERNAL_DIR}/imgui)
set(IMGUI_SOURCES 
${IMGUI_DIR}/imgui.cpp
${IMGUI_DIR}/imgui_draw.cpp
${IMGUI_DIR}/imgui_tables.cpp
${IMGUI_DIR}/imgui_widgets.cpp
${IMGUI_DIR}/imgui_demo.cpp
${IMGUI_DIR}/backends/imgui_impl_dx12.cpp
${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
)
set(IMGUI_INCLUDE_DIR ${IMGUI_DIR} ${IMGUI_DIR}/backends) 
add_library(imgui STATIC ${IMGUI_SOURCES})
target_link_libraries(imgui PUBLIC SDL2-static)
target_include_directories(imgui PUBLIC ${IMGUI_INCLUDE_DIR})

#dxri 
add_subdirectory(${EXTERNAL_DIR}/dxri)

set(CMAKE_CXX_STANDARD 17)




file(GLOB_RECURSE SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h
)

file(GLOB_RECURSE SHADER_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/Assets/*.hlsl
)

add_custom_target(Shaders SOURCES ${SHADER_FILES})

add_executable(${PROJECT_NAME} ${SOURCES})

target_precompile_headers(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Source/pch.h)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2-static DirectX-Headers DirectXTK12 d3d12 dxcompiler dxgi dxguid glm::glm tinyobjloader imgui enet_static dxri)
