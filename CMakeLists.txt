cmake_minimum_required(VERSION 3.15)
project(AIML_project4 VERSION 1.0.0 LANGUAGES CXX)

# Set the C++ standard globally for targets
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Useful for LSP/IDE autocomplete

# Define the executable target
add_executable(mdp_program)
target_sources(mdp_program PRIVATE
    src/mdp_main.cpp
)

# Include directories (e.g., if you have an 'include' folder)
target_include_directories(mdp_program PRIVATE "inc" "include")

add_executable(qlearning_program)
target_sources(qlearning_program PRIVATE
    src/qlearning_main.cpp
)

target_include_directories(qlearning_program PRIVATE "inc" "include")


# Compiler-specific warning flags
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(mdp_program PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(qlearning_program PRIVATE -Wall -Wextra -Wpedantic)
endif()
