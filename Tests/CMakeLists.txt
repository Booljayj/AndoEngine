cmake_minimum_required(VERSION 3.10)
project(Tests)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(Tests WIN32 source/main.cpp)
target_include_directories(Tests PUBLIC ${PROJECT_SOURCE_DIR}/source)
target_precompile_headers(Tests PRIVATE source/PCH.h)

#Copy the contents directory to the same directory as the executable
add_custom_command(
    TARGET Tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/content $<TARGET_FILE_DIR:Tests>/content
)

#Library library
add_dependencies(Tests Library)
target_link_libraries(Tests PUBLIC Library)
target_include_directories(Tests PUBLIC Library)

#EditorLibrary library
add_dependencies(Tests EditorLibrary)
target_link_libraries(Tests PUBLIC EditorLibrary)
target_include_directories(Tests PUBLIC EditorLibrary)
