cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)
project(Lirch C CXX)

## BASIC OPTIONS

# Force a build type to be declared
set(CMAKE_BUILD_TYPES "(None|Debug|Release|RelWithDebInfo|MinSizeRel)")
if(NOT CMAKE_BUILD_TYPE MATCHES "${CMAKE_BUILD_TYPES}")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "${CMAKE_BUILD_TYPES}" FORCE)
endif(NOT CMAKE_BUILD_TYPE MATCHES "${CMAKE_BUILD_TYPES}")

# Version info
set(LIRCH_VERSION_MAJOR 0)
set(LIRCH_VERSION_MINOR 1)
set(LIRCH_VERSION_PATCH 0)
set(LIRCH_VERSION_TWEAK 1)
set(LIRCH_VERSION_STRING "${LIRCH_VERSION_MAJOR}.${LIRCH_VERSION_MINOR}.${LIRCH_VERSION_PATCH}.${LIRCH_VERSION_TWEAK}")

# Find git and fetch a hash
find_program(GIT_EXECUTABLE git)
if(GIT_EXECUTABLE)
  execute_process(COMMAND "${GIT_EXECUTABLE}" rev-parse -q HEAD
                  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
                  OUTPUT_VARIABLE LIRCH_BUILD_HASH
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
else(GIT_EXECUTABLE)
  set(LIRCH_BUILD_HASH "GIT_NOT_FOUND")
endif(GIT_EXECUTABLE)

# Configure some string literals and include build dir
configure_file(lirch_constants.h.in "${CMAKE_BINARY_DIR}/lirch_constants.h")
include_directories("${CMAKE_BINARY_DIR}")

# Basic build options
option(BUILD_QT_CLIENT      "Build the Qt4 interface"     ON)
option(BUILD_NCURSES_CLIENT "Build the ncurses interface" OFF)
option(BUILD_BASIC_CLIENT   "Build the basic interface"   OFF)
option(BUILD_PLUGINS        "Build the plugin suite"      ON)

## LIBRARIES

# Figure out which Qt components to load, if any
if(BUILD_NCURSES_CLIENT OR BUILD_PLUGINS)
  set(QT_COMPONENTS ${QT_COMPONENTS} ; QtCore ; QtNetwork)
endif(BUILD_NCURSES_CLIENT OR BUILD_PLUGINS)
if(BUILD_QT_CLIENT)
  set(QT_COMPONENTS ${QT_COMPONENTS} ; QtGui)
endif(BUILD_QT_CLIENT)
if(BUILD_TESTING)
  set(QT_COMPONENTS ${QT_COMPONENTS} ; QtTest)
endif(BUILD_TESTING)
# Load them
if(QT_COMPONENTS)
  set(QT_MIN_VERSION 4.8.0)
  find_package(Qt4 ${QT_MIN_VERSION} COMPONENTS ${QT_COMPONENTS} REQUIRED)
endif(QT_COMPONENTS)

# At some point maybe we'll consider supporting compilers that aren't gcc
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
# TODO when does this become unnecessary?
add_definitions(-D_GLIBCXX_USE_NANOSLEEP)
# Find the libraries we'll be needing (for core)
find_library(PTHREAD_LIBRARY_PATH pthread REQUIRED)
# TODO is this is *nix only? Windows alternative?
find_library(DL_LIBRARY_PATH dl REQUIRED)

## COMPILATION

# Specify some default directories
include_directories("${CMAKE_SOURCE_DIR}")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")

# Always build the core
add_subdirectory(core)

if(BUILD_PLUGINS)
  add_subdirectory(plugins)
endif(BUILD_PLUGINS)

if(BUILD_BASIC_CLIENT)
  add_subdirectory(ui/basic)
endif(BUILD_BASIC_CLIENT)

if(BUILD_NCURSES_CLIENT)
  add_subdirectory(ui/ncurses)
endif(BUILD_NCURSES_CLIENT)

if(BUILD_QT_CLIENT)
  add_subdirectory(ui/qt)
endif(BUILD_QT_CLIENT)

# TESTING

include(CTest)
if(BUILD_TESTING)
  set(CTEST_PROJECT_NAME ${CMAKE_PROJECT_NAME})
  add_subdirectory(test)
endif(BUILD_TESTING)

# PACKAGING

include(CPack)
