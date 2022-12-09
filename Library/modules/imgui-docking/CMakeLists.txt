cmake_minimum_required(VERSION 3.10)
project(IMGUI)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE SourceFiles LIST_DIRECTORIES false RELATIVE ${PROJECT_SOURCE_DIR} CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/*.cpp)

find_package(SDL2 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)

add_library(IMGUI STATIC ${SourceFiles})
target_include_directories(IMGUI PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/backends)

target_link_libraries(IMGUI PUBLIC SDL2::SDL2main)
target_link_libraries(IMGUI PUBLIC Vulkan::Vulkan)
