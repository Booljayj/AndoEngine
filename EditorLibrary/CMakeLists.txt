cmake_minimum_required(VERSION 3.24)
project(EditorLibrary)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE SourceFiles LIST_DIRECTORIES false RELATIVE ${PROJECT_SOURCE_DIR} CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/source/*.cpp)
file(GLOB_RECURSE GeneratedFiles LIST_DIRECTORIES false RELATIVE ${PROJECT_SOURCE_DIR} CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/generated/*.cpp)

add_library(EditorLibrary STATIC ${SourceFiles} ${GeneratedFiles})
target_include_directories(EditorLibrary PUBLIC ${PROJECT_SOURCE_DIR}/source)

#Primary Library
add_dependencies(EditorLibrary Library)
target_link_libraries(EditorLibrary PUBLIC Library)

#Boost library
set(Boost_USE_STATIC_LIBS 1)
find_package(Boost REQUIRED)
target_include_directories(EditorLibrary PRIVATE ${Boost_INCLUDE_DIR})

#GLM library
find_package(glm REQUIRED)
target_link_libraries(EditorLibrary PUBLIC glm::glm)

#Vulkan library
find_package(Vulkan REQUIRED shaderc_combined)
target_link_libraries(EditorLibrary PUBLIC Vulkan::Vulkan)
target_link_libraries(EditorLibrary PUBLIC Vulkan::shaderc_combined)
target_include_directories(EditorLibrary PUBLIC Vulkan::shaderc_combined)
