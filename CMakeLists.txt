cmake_minimum_required(VERSION 2.8.0 FATAL_ERROR)
project(Lirch C CXX)

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

# Basic build options
option(BUILD_QT_CLIENT      "Build the Qt4 interface"     ON)
option(BUILD_NCURSES_CLIENT "Build the ncurses interface" OFF)

# Configure relevant files and include build dir
configure_file(lirch_qt_interface.ui "${CMAKE_BINARY_DIR}/lirch_qt_interface.ui")
configure_file(lirch_constants.h.in "${CMAKE_BINARY_DIR}/lirch_constants.h")
include_directories(${CMAKE_BINARY_DIR})

if(BUILD_QT_CLIENT)
	# We need to locate Qt4
        find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork QtTest REQUIRED)

	# It's likely we'll use these modules
	set(QT_USE_QTNETWORK TRUE)
	set(QT_USE_QTTEST TRUE)
	include(${QT_USE_FILE})

	# Source, headers to moc, and UI files
	set(lirch_qt_interface_SOURCES lirch_qt_main.cpp lirch_qt_interface.cpp)
	set(lirch_qt_interface_HEADERS lirch_qt_interface.h)
	set(lirch_qt_interface_UI "${CMAKE_BINARY_DIR}/lirch_qt_interface.ui")
        set(lirch_qt_interface_QRC "${CMAKE_SOURCE_DIR}/lirch_qt_icons.qrc")

	# Requisites for building with Qt4
	qt4_wrap_cpp(lirch_qt_interface_MOC_HEADERS ${lirch_qt_interface_HEADERS})
	qt4_wrap_ui(lirch_qt_interface_UI_HEADERS ${lirch_qt_interface_UI})
	qt4_add_resources(lirch_qt_interface_QRC_HEADERS ${lirch_qt_interface_QRC})
	add_definitions(${QT_DEFINITIONS})

	# Add details for this target
	add_executable(lirch-qt
		${lirch_qt_interface_SOURCES}
		${lirch_qt_interface_MOC_HEADERS}
		${lirch_qt_interface_UI_HEADERS}
                ${lirch_qt_interface_QRC_HEADERS})
	target_link_libraries(lirch-qt ${QT_LIBRARIES})
endif(BUILD_QT_CLIENT)

if(BUILD_NCURSES_CLIENT)
        find_library(NCURSES_LIBRARY_PATH ncurses REQUIRED)
        find_library(Qt4 COMPONENTS QtCore QtNetwork REQUIRED)

        # It's likely we'll use these modules
        set(QT_USE_QTNETWORK TRUE)
        include(${QT_USE_FILE})
endif(BUILD_NCURSES_CLIENT)

# TODO does the .pro file need to be configured to load/unload Qt4Components?
# Both clients use QtCore and QtNetwork, only the Qt client uses QtGui and QtTest
# To unload QtGui, need QT -= gui, QtCore is implicit, QT += network testlib

# Testing

include(CTest)
if(ENABLE_TESTING)
  set(CTEST_PROJECT_NAME ${CMAKE_PROJECT_NAME})
endif(ENABLE_TESTING)

# Packaging

include(CPack)
