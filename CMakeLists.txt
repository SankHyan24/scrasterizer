set(_PROJECT_NAME_ "sc_rasterizer") # project name
set(_EXE_NAME_ "scra") # target name
set(_SRC_FILE_NAME_ "src") # source file location

cmake_minimum_required(VERSION 3.15)

project(${_PROJECT_NAME_} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    message(STATUS "using MSVC")
    message(STATUS "warning: If u use other compiler, please modify the CMakeLists.txt")
endif()

add_executable(${_EXE_NAME_})
aux_source_directory(${_SRC_FILE_NAME_} _SOURCE_)
target_sources(${_EXE_NAME_} PUBLIC ${_SOURCE_})
target_include_directories(${_EXE_NAME_} PUBLIC "./include")

find_package(OpenGL REQUIRED)
# add glfw
add_subdirectory(submodules/glfw)
target_include_directories(${_EXE_NAME_} PUBLIC "submodules/glfw/include")
target_link_libraries(${_EXE_NAME_} glfw OpenGL::GL)

# add imgui
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/submodules/imgui)
file(GLOB IMGUI_SOURCES ${IMGUI_DIR}/*.cpp ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${IMGUI_DIR})
target_include_directories(imgui PUBLIC ${IMGUI_DIR}/backends)

target_link_libraries(imgui glfw OpenGL::GL)
target_link_libraries(${_EXE_NAME_} imgui)
