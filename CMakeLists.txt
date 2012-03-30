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
option(BUILD_TESTS          "Build the testing executable" OFF)

# Build the main program (always happens)
set(CMAKE_CXX_FLAGS -std=c++0x)
add_executable(lirch core.cpp message.cpp message_pipe.cpp plugin_loader.cpp)
target_link_libraries(lirch dl)

# Build the meat grinder (always happens)
find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
add_library(meatgrinder SHARED meatgrinder.cpp message.cpp message_pipe.cpp)
target_link_libraries(meatgrinder ${QT_LIBRARIES})

add_library(timer SHARED timer.cpp message.cpp message_pipe.cpp)

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

if(BUILD_TESTS)
	add_executable(lirch_tests test.cpp message_pipe.cpp message.cpp)
	# At some point maybe I'll consider supporting compilers that aren't gcc
	set(CMAKE_CXX_FLAGS -std=c++0x\ -lpthread)
	enable_testing()
	add_test(sendToPlugin lirch_tests toplugin)
	add_test(sendToCore lirch_tests tocore)
	add_test(readEmptyMessage lirch_tests readempty)
	add_test(cycleMessage lirch_tests cycle)
	add_test(exhaustMessages lirch_tests exhaust)
	add_test(share1Pipe lirch_tests sharing)
	add_test(messageOrder lirch_tests order)
	add_test(multiPipe lirch_tests copying)
	add_test(threadSequencing lirch_tests threadseq)
	add_test(immediateBlockingRead lirch_tests 0bread)
	add_test(threadedBlockingRead lirch_tests 1bread)
	add_test(doubleBlockingRead lirch_tests 2bread)
	add_test(sharedDoubleBlockingRead lirch_tests shared2bread)
	add_test(registration lirch_tests register)
	add_test(initializingRegistration lirch_tests registryInitList)
	add_test(registrationSize lirch_tests registryEmpty)
	add_test(registryDuplicateHandling lirch_tests registryDups)
endif(BUILD_TESTS)
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
