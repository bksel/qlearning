cmake_minimum_required(VERSION 3.15)
project(AIML_project4 VERSION 1.0.0 LANGUAGES CXX)

# Print the detected compilers
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID})")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

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


include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT error)

if(ipo_supported)
    set_target_properties(qlearning_program PROPERTIES 
        INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
    )
else()
    message(WARNING "IPO/LTO is not supported by this compiler: ${error}")
endif()


# Compiler-specific warning flags
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(mdp_program PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(qlearning_program PRIVATE -Wall -Wextra -Wpedantic
        $<$<CONFIG:Release>:-march=native>
    )
endif()


