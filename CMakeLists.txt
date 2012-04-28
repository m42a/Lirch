cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)
project(Lirch C CXX)

## BUILD SETUP

# Force a build type to be declared
set(CMAKE_BUILD_TYPES "(None|Debug|Release|RelWithDebInfo|MinSizeRel)")
if(NOT CMAKE_BUILD_TYPE MATCHES "${CMAKE_BUILD_TYPES}")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "${CMAKE_BUILD_TYPES}" FORCE)
endif(NOT CMAKE_BUILD_TYPE MATCHES "${CMAKE_BUILD_TYPES}")
# Specify some default directories
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")

# Version info
set(LIRCH_VERSION_MAJOR 0)
set(LIRCH_VERSION_MINOR 1)
set(LIRCH_VERSION_PATCH 1)
set(LIRCH_VERSION_TWEAK 0)
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

# Create a log subdirectory in the build directory
set(DEFAULT_LOG_DIRECTORY "${CMAKE_BINARY_DIR}/log")
file(MAKE_DIRECTORY "${DEFAULT_LOG_DIRECTORY}")
set(LOG_DIR_DOCSTRING "Configure the program to place log files here by default")
set(LIRCH_LOG_DIRECTORY "${DEFAULT_LOG_DIRECTORY}" CACHE STRING "${LOG_DIR_DOCSTRING}")
# Configure some string literals
configure_file(lirch_constants.h.in "${CMAKE_BINARY_DIR}/lirch_constants.h")
configure_file(core/lirch_qtcore.cpp.in "${CMAKE_BINARY_DIR}/core/alirch_qtcore.cpp")
# Include some default directories
include_directories("${CMAKE_BINARY_DIR}")
include_directories("${CMAKE_SOURCE_DIR}")

## BUILD OPTIONS

# Basic build options
option(BUILD_CLIENT_BASIC  "Build the basic interface"       OFF)
option(BUILD_CLIENT_CURSES "Build the curses interface"      OFF)
option(BUILD_CLIENT_QTGUI  "Build the QtGui interface"       ON)
option(BUILD_PLUGINS       "Build the standard plugin suite" ON)
option(BUILD_PLUGINS_EXTRA "Build non-standard plugins"      OFF)
mark_as_advanced(BUILD_PLUGINS_EXTRA FORCE)

# Handle preload option additions (advanced)
macro(add_preload_option PLUGIN_NAME OPTION_STATE)
  string(TOUPPER "PRELOAD_${PLUGIN_NAME}" OPTION_NAME)
  set(OPTION_DOC "Preload the ${PLUGIN_NAME} plugin")
  # Preload defaults to OFF
  option(${OPTION_NAME} ${OPTION_DOC} OFF)
  mark_as_advanced(${OPTION_NAME} FORCE)
  # Disabling the second argument triggers disabling the preload
  if (NOT ${OPTION_STATE})
    set(${OPTION_NAME} OFF CACHE BOOL ${OPTION_DOC} FORCE)
  endif(NOT ${OPTION_STATE})
  # Preloads might modify the configuration of lirch_constants.h
  if(${OPTION_NAME})
    string(TOUPPER "PREPATH_${PLUGIN_NAME}" LIBRARY_NAME)
    # TODO add paths to facilitate finding the correct lib
    # FIXME this needs to use find_library or something for portability
    # set(${LIBRARY_NAME} "${LIBRARY_OUTPUT_PATH}/lib${PLUGIN_NAME}.so")
    # FIXME don't use LIBRARY_OUTPUT_PATH or don't use find_library
    find_library(${LIBRARY_NAME} ${PLUGIN_NAME} "${LIBRARY_OUTPUT_PATH}")	
    mark_as_advanced(${LIBRARY_NAME} FORCE)
    # Provided this preload is selected, add it to LIRCH_PRELOADS
    if(${LIBRARY_NAME})
      set(LIRCH_PRELOADS "${LIRCH_PRELOADS}
	{ \"${PLUGIN_NAME}\", \"${${LIBRARY_NAME}}\" },")
    endif(${LIBRARY_NAME})
  endif(${OPTION_NAME})
endmacro(add_preload_option)
# The active state of each is determined above
add_preload_option("timer"          BUILD_PLUGINS_EXTRA)
add_preload_option("linuxtester"    BUILD_PLUGINS_EXTRA)
add_preload_option("settingstester" BUILD_PLUGINS_EXTRA)

# Any specific client requires certain preloads
macro(add_preload PLUGIN_NAME PRELOADS)
  # FIXME this needs to use find_library or something for portability
  set(LIBRARY_NAME "${LIBRARY_OUTPUT_PATH}/lib${PLUGIN_NAME}.so")
  set(${PRELOADS} "${${PRELOADS}}
	{ \"${PLUGIN_NAME}\", \"${LIBRARY_NAME}\" },")
endmacro(add_preload PLUGIN_NAME PRELOADS)
if(BUILD_CLIENT_BASIC OR BUILD_CLIENT_CURSES OR BUILD_CLIENT_QTGUI)
  # Load all standard plugins
  if(BUILD_PLUGINS)
    add_preload("antenna"     CLIENT_PRELOADS)
    add_preload("logger"      CLIENT_PRELOADS)
    add_preload("meatgrinder" CLIENT_PRELOADS)
    add_preload("masseuse"    CLIENT_PRELOADS)
    add_preload("userlist"    CLIENT_PRELOADS)
  endif(BUILD_PLUGINS)
  # These are for specific UI
  if(BUILD_CLIENT_BASIC)
    set(LIRCH_CLIENT_PRELOADS "${CLIENT_PRELOADS}")
    add_preload("lirch-basic" LIRCH_CLIENT_PRELOADS)
    configure_file(core/lirch_qtcore.cpp.in core/blirch_qtcore.cpp)
  endif(BUILD_CLIENT_BASIC)
  if(BUILD_CLIENT_CURSES)
    set(LIRCH_CLIENT_PRELOADS "${CLIENT_PRELOADS}")
    add_preload("lirch-curses" LIRCH_CLIENT_PRELOADS)
    configure_file(core/lirch_qtcore.cpp.in core/clirch_qtcore.cpp)
  endif(BUILD_CLIENT_CURSES)
  if(BUILD_CLIENT_QTGUI)
    set(LIRCH_CLIENT_PRELOADS "${CLIENT_PRELOADS}")
    add_preload("lirch-gui" LIRCH_CLIENT_PRELOADS)
    configure_file(core/lirch_qtgui.cpp.in core/qlirch_qtgui.cpp)
  endif(BUILD_CLIENT_QTGUI)
endif(BUILD_CLIENT_BASIC OR BUILD_CLIENT_CURSES OR BUILD_CLIENT_QTGUI)

## LIBRARIES

# Figure out which Qt components to load, if any
if(BUILD_CLIENT_QTGUI OR BUILD_PLUGINS)
  # (Almost) everything (except core) needs QtCore
  list(APPEND QT_COMPONENTS "QtCore")
endif(BUILD_CLIENT_QTGUI OR BUILD_PLUGINS)
# Only the antenna plugin strictly needs QtNetwork
if(BUILD_PLUGINS)
  list(APPEND QT_COMPONENTS "QtNetwork")
endif(BUILD_PLUGINS)
# Only the Qt GUI needs QtGui (and QtTest for testing)
if(BUILD_CLIENT_QTGUI)
  list(APPEND QT_COMPONENTS "QtGui")
  if(BUILD_TESTING)
    list(APPEND QT_COMPONENTS "QtTest")
  endif(BUILD_TESTING)
endif(BUILD_CLIENT_QTGUI)
# Load them
if(QT_COMPONENTS)
  set(QT_MIN_VERSION 4.8.0)
  find_package(Qt4 ${QT_MIN_VERSION} COMPONENTS ${QT_COMPONENTS} REQUIRED)
endif(QT_COMPONENTS)

# At some point maybe we'll consider supporting compilers that aren't gcc
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -Wextra")
  # TODO when does this become unnecessary?
  add_definitions(-D_GLIBCXX_USE_NANOSLEEP)
else(CMAKE_COMPILER_IS_GNUCXX)
  message(WARNING "You need to use gcc 4.6.0+ to compile lirch.")
endif(CMAKE_COMPILER_IS_GNUCXX)
# Find the libraries we'll be needing (for core)
find_library(PTHREAD_LIBRARY_PATH pthread REQUIRED)
if(UNIX)
  # TODO is this is *nix only? Windows alternative?
  find_library(DL_LIBRARY_PATH dl REQUIRED)
endif(UNIX)

## COMPILATION

# Always build the core
add_subdirectory(core)

if(BUILD_PLUGINS)
  add_subdirectory(plugins)
endif(BUILD_PLUGINS)

if(BUILD_CLIENT_BASIC)
  add_subdirectory(ui/basic)
endif(BUILD_CLIENT_BASIC)

if(BUILD_CLIENT_CURSES)
  add_subdirectory(ui/curses)
endif(BUILD_CLIENT_CURSES)

if(BUILD_CLIENT_QTGUI)
  add_subdirectory(ui/qt)
endif(BUILD_CLIENT_QTGUI)

## TESTING

include(CTest)
if(BUILD_TESTING)
  set(CTEST_PROJECT_NAME ${CMAKE_PROJECT_NAME})
  add_subdirectory(test)
endif(BUILD_TESTING)

## PACKAGING

# TODO how to get make package and cpack both to work?
set(FROM_CMAKE_BUILD ON CACHE INTERNAL "Tag for make package" FORCE)
# Initialize a global configuration
set(CPACK_GENERATOR "STGZ;TGZ;TBZ2;TZ") # FIXME ZIP is broken for me
set(CPACK_PROJECT_CONFIG_FILE "${CMAKE_SOURCE_DIR}/CPackConfig.cmake")
if(UNIX)
  list(APPEND CPACK_GENERATOR "DEB") # TODO also RPM/arch?
endif(UNIX)
if(WIN32 AND NOT UNIX)
  list(APPEND CPACK_GENERATOR "NSIS") # TODO doesn't work
endif(WIN32 AND NOT UNIX)
include("${CPACK_PROJECT_CONFIG_FILE}")
# Initialize source packaging (for future use)
#set(CPACK_SOURCE_PROJECT_CONFIG_FILE "${CMAKE_SOURCE_DIR}/CPackSourceConfig.cmake")
#include("${CPACK_SOURCE_PROJECT_CONFIG_FILE}")
# Include the proper modules for packaging
include(CPack)
