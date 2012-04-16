cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)
project(Lirch C CXX)

## BASIC OPTIONS

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

# Basic build options
option(BUILD_CLIENT_BASIC   "Build the basic interface"       OFF)
option(BUILD_CLIENT_NCURSES "Build the ncurses interface"     OFF)
option(BUILD_CLIENT_QT      "Build the Qt4 interface"         ON)
option(BUILD_PLUGINS        "Build the standard plugin suite" ON)

if(BUILD_CLIENT_BASIC OR BUILD_CLIENT_NCURSES)
  set(BUILD_CLIENT_NON_QT ON)
endif(BUILD_CLIENT_BASIC OR BUILD_CLIENT_NCURSES)

# Preload options (advanced)
macro(preload_option NAME OPTION_STATE)
  string(TOUPPER "PRELOAD_${NAME}" OPTION_NAME)
  set(OPTION_DOC "Preload the ${NAME} plugin")
  # Preload defaults to OFF
  option(${OPTION_NAME} ${OPTION_DOC} OFF)
  mark_as_advanced(${OPTION_NAME} FORCE)
  # Disabling the second argument triggers disabling the preload
  if (NOT ${OPTION_STATE})
    set(${OPTION_NAME} OFF CACHE BOOL ${OPTION_DOC} FORCE)
  endif(NOT ${OPTION_STATE})
  # Preloads modify the configuration of lirch_constants.h
  if(${OPTION_NAME})
    if(UNIX)
      list(APPEND PRELOADS "{ \"${NAME}\", \"${LIBRARY_OUTPUT_PATH}/lib${NAME}.so\" },")
    endif(UNIX)
    # FIXME add WIN32 support
  endif(${OPTION_NAME})
endmacro(preload_option)
# The active state of each is determined above
preload_option("lirch-basic"   BUILD_CLIENT_BASIC)
preload_option("lirch-ncurses" BUILD_CLIENT_NCURSES)
preload_option("lirch-qt"      BUILD_CLIENT_QT)
preload_option("antenna"       BUILD_PLUGINS)
preload_option("logger"        BUILD_PLUGINS)
preload_option("masseuse"      BUILD_PLUGINS)
preload_option("meatgrinder"   BUILD_PLUGINS)
preload_option("userlist"      BUILD_PLUGINS)
# Propagate the preloads for configuration
if(PRELOADS)
  foreach(PRELOAD IN LISTS PRELOADS)
    set(LIRCH_PRELOADS "${LIRCH_PRELOADS}
	${PRELOAD}")
  endforeach(PRELOAD IN LISTS PRELOADS)
endif(PRELOADS)

# Configure some string literals and include build/source dir
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/log")
configure_file(lirch_constants.h.in "${CMAKE_BINARY_DIR}/lirch_constants.h")
include_directories("${CMAKE_BINARY_DIR}")
include_directories("${CMAKE_SOURCE_DIR}")

## LIBRARIES

# Figure out which Qt components to load, if any
if(BUILD_CLIENT_QT OR BUILD_PLUGINS)
  # (Almost) everything (except core) needs QtCore
  list(APPEND QT_COMPONENTS "QtCore")
endif(BUILD_CLIENT_QT OR BUILD_PLUGINS)
# Only the antenna plugin strictly needs QtNetwork
if(BUILD_PLUGINS)
  list(APPEND QT_COMPONENTS "QtNetwork")
endif(BUILD_PLUGINS)
# Only the Qt GUI needs QtGui (and QtTest for testing)
if(BUILD_CLIENT_QT)
  list(APPEND QT_COMPONENTS "QtGui")
  if(BUILD_TESTING)
    list(APPEND QT_COMPONENTS "QtTest")
  endif(BUILD_TESTING)
endif(BUILD_CLIENT_QT)
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

if(BUILD_CLIENT_NCURSES)
  add_subdirectory(ui/ncurses)
endif(BUILD_CLIENT_NCURSES)

if(BUILD_CLIENT_QT)
  add_subdirectory(ui/qt)
endif(BUILD_CLIENT_QT)

# TESTING

include(CTest)
if(BUILD_TESTING)
  set(CTEST_PROJECT_NAME ${CMAKE_PROJECT_NAME})
  add_subdirectory(test)
endif(BUILD_TESTING)

# PACKAGING

# TODO dependencies and such
include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_VENDOR              "The Addams Family")
set(CPACK_PACKAGE_CONTACT             "Tor E. Hagemann <hagemt@rpi.edu>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A local IRC host (lirch)")
set(CPACK_PACKAGE_DESCRIPTION_FILE    "${CMAKE_SOURCE_DIR}/NEWS")

set(CPACK_RESOURCE_FILE_LICENSE       "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README        "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_WELCOME       "${CMAKE_SOURCE_DIR}/NEWS")

set(CPACK_PACKAGE_VERSION_MAJOR       "${LIRCH_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR       "${LIRCH_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH       "${LIRCH_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION_TWEAK       "${LIRCH_VERSION_TWEAK}")

# TODO is any of this necessary for us? (if so, make it portable)
#list(APPEND CPACK_SOURCE_STRIP_FILES "${CMAKE_BUILD_DIR}/lirch_constants.h")
#list(APPEND CPACK_STRIP_FILES "${LIBRARY_OUTPUT_PATH}/libenvelop.so")
#list(APPEND CPACK_STRIP_FILES "${EXECUTABLE_OUTPUT_PATH}/lirch")
#if(BUILD_PLUGINS)
#  list(APPEND CPACK_STRIP_FILES "${LIBRARY_OUTPUT_PATH}/libantenna.so")
#  list(APPEND CPACK_STRIP_FILES "${LIBRARY_OUTPUT_PATH}/liblogger.so")
#  list(APPEND CPACK_STRIP_FILES "${LIBRARY_OUTPUT_PATH}/libmasseuse.so")
#  list(APPEND CPACK_STRIP_FILES "${LIBRARY_OUTPUT_PATH}/libmeatgrinder.so")
#  list(APPEND CPACK_STRIP_FILES "${LIBRARY_OUTPUT_PATH}/libuserlist.so")
#endif(BUILD_PLUGINS)
#if(BUILD_BASIC_CLIENT)
#  list(APPEND CPACK_STRIP_FILES "${LIBRARY_OUTPUT_PATH}/liblirch-basic.so")
#endif(BUILD_BASIC_CLIENT)
#if(BUILD_NCURSES_CLIENT)
#  list(APPEND CPACK_STRIP_FILES "${LIBRARY_OUTPUT_PATH}/liblirch-ncurses.so")
#endif(BUILD_NCURSES_CLIENT)
#if(BUILD_QT_CLIENT)
#  list(APPEND CPACK_STRIP_FILES "${LIBRARY_OUTPUT_PATH}/liblirch-qt.so")
#endif(BUILD_QT_CLIENT)

set(CPACK_PACKAGE_EXECUTABLES "${CMAKE_EXECUTABLE_OUTPUT_PATH}/lirch" "Lirch (core)")

include(CPack)
