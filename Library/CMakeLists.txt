cmake_minimum_required(VERSION 3.24)
project(Library)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE SourceFiles LIST_DIRECTORIES false RELATIVE ${PROJECT_SOURCE_DIR} CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/source/*.cpp)
file(GLOB_RECURSE GeneratedFiles LIST_DIRECTORIES false RELATIVE ${PROJECT_SOURCE_DIR} CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/generated/*.cpp)

add_library(Library STATIC ${SourceFiles} ${GeneratedFiles})
target_include_directories(Library PUBLIC ${PROJECT_SOURCE_DIR}/source)
target_precompile_headers(Library PRIVATE source/Engine/StandardTypes.h)
target_compile_definitions(Library PUBLIC "$<$<CONFIG:DEBUG>:VULKAN_DEBUG>")
target_compile_features(Library PUBLIC cxx_std_20)

#IMGUI module
add_subdirectory(modules/imgui-docking)
target_include_directories(Library PUBLIC ${IMGUI_INCLUDE_DIR})
target_link_libraries(Library PUBLIC IMGUI)

#Optick module
set(OPTICK_INSTALL_TARGETS OFF CACHE BOOL "" FORCE)
set(OPTICK_USE_VULKAN ON CACHE BOOL "" FORCE)
add_subdirectory(modules/Optick)
target_include_directories(Library PUBLIC ${OptickCore_INCLUDE_DIR})
target_link_libraries(Library PUBLIC OptickCore)

#Boost library
set(Boost_USE_STATIC_LIBS 1)
find_package(Boost REQUIRED)
target_include_directories(Library PRIVATE ${Boost_INCLUDE_DIR})

#GLM library
find_package(glm REQUIRED)
target_link_libraries(Library PUBLIC glm::glm)

#EnTT library
find_package(EnTT REQUIRED)
target_link_libraries(Library PUBLIC EnTT::EnTT)

#SDL library
find_package(SDL2 REQUIRED)
target_link_libraries(Library PUBLIC SDL2::SDL2 PUBLIC SDL2::SDL2main)

#Vulkan library
find_package(Vulkan REQUIRED shaderc_combined)
target_link_libraries(Library PUBLIC Vulkan::Vulkan)
target_link_libraries(Library PUBLIC Vulkan::shaderc_combined)

#FreeType library
find_package(freetype CONFIG REQUIRED)
target_link_libraries(Library PUBLIC freetype)
