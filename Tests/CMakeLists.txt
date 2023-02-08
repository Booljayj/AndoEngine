cmake_minimum_required(VERSION 3.10)
project(Tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(Tests WIN32 source/main.cpp)
target_include_directories(Tests PUBLIC ${PROJECT_SOURCE_DIR}/source)
target_precompile_headers(Tests PRIVATE source/PCH.h)
set_target_properties(Tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}"
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

#Library library
add_dependencies(Tests Library)
target_link_libraries(Tests PUBLIC Library)
target_include_directories(Tests PUBLIC Library)

#EditorLibrary library
add_dependencies(Tests EditorLibrary)
target_link_libraries(Tests PUBLIC EditorLibrary)
target_include_directories(Tests PUBLIC EditorLibrary)
