# CMakeLists.txt

cmake_minimum_required(VERSION 3.10)

project(MyCMakeProject)

# Set C++ standard and compiler flags
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Specify Boost library
find_package(Boost REQUIRED COMPONENTS system)

# Add the executable target
add_executable(test_connection main2.cpp)

# Link against Boost library
target_link_libraries(test_connection PRIVATE Boost::system)

# Set output directory for the binary
set_target_properties(test_connection PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

# Define a custom target for running the executable
add_custom_target(run
    COMMAND $<TARGET_FILE:test_connection>
    DEPENDS test_connection
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/build
)

# Define a custom target for cleaning
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/build
)

# Add 'my_all' target as an alias for 'run'
add_custom_target(my_all DEPENDS run)
