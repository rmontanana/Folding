cmake_minimum_required(VERSION 3.20)

project(Folding
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
