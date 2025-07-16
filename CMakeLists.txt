cmake_minimum_required(VERSION 3.20)

project(Folding
  VERSION 1.1.1
  DESCRIPTION "Folding utility for BayesNet library"
  HOMEPAGE_URL "https://github.com/rmontanana/folding"
  LANGUAGES CXX
)

find_package(Torch REQUIRED)

if (POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif ()

# Global CMake variables
# ----------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED            ON)
set(CMAKE_CXX_EXTENSIONS                  OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS          ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# Options
# -------
option(ENABLE_TESTING "Unit testing build" OFF)

# Subdirectories
# --------------
add_subdirectory(config)

# Testing
# -------
if (ENABLE_TESTING)
  MESSAGE("Testing enabled")
  find_package(Catch2 REQUIRED)
  find_package(arff-files REQUIRED)
  find_package(fimdlp REQUIRED)
  include(CTest)
  add_subdirectory(tests)
endif (ENABLE_TESTING)

# Library
# --------
add_library(folding INTERFACE folding.hpp)

target_include_directories(folding INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/configured_files/include>
    $<INSTALL_INTERFACE:include>
)

# Install
# -------
install(TARGETS folding EXPORT FoldingTargets
  INCLUDES DESTINATION include
)
install(EXPORT FoldingTargets
  FILE FoldingTargets.cmake
  NAMESPACE Folding::
  DESTINATION lib/cmake/Folding
)

# Install the main header file
install(FILES folding.hpp
  DESTINATION include
)

# Install the generated configuration header
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/configured_files/include/folding_config.h"
  DESTINATION include
)

# Install documentation files
install(FILES LICENSE README.md
  DESTINATION share/doc/Folding
)

# Create and install package configuration files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/FoldingConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FoldingConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/FoldingConfig.cmake"
  INSTALL_DESTINATION lib/cmake/Folding
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/FoldingConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/FoldingConfigVersion.cmake"
  DESTINATION lib/cmake/Folding
)
