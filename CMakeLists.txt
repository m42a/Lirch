cmake_minimum_required(VERSION 2.8.0)
project(Lirch C CXX)

# Version info
set(LIRCH_VERSION_MAJOR 0)
set(LIRCH_VERSION_MINOR 1)
set(LIRCH_VERSION_PATCH 0)
set(LIRCH_VERSION_TWEAK 1)
set(LIRCH_VERSION_STRING "${LIRCH_VERSION_MAJOR}.${LIRCH_VERSION_MINOR}.${LIRCH_VERSION_PATCH}.${LIRCH_VERSION_TWEAK}")
configure_file(lirch_qt_interface.ui "${CMAKE_BINARY_DIR}/lirch_qt_interface.ui")
configure_file(lirch_constants.h.in "${CMAKE_BINARY_DIR}/lirch_constants.h")
include_directories(${CMAKE_BINARY_DIR})

# Force a build type to be declared
set(CMAKE_BUILD_TYPES "(None|Debug|Release|RelWithDebInfo|MinSizeRel)")
if(NOT CMAKE_BUILD_TYPE MATCHES "${CMAKE_BUILD_TYPES}")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "${CMAKE_BUILD_TYPES}" FORCE)
endif(NOT CMAKE_BUILD_TYPE MATCHES "${CMAKE_BUILD_TYPES}")

# Basic build options
option(BUILD_QT_CLIENT      "Build the Qt4 interface"     ON)
option(BUILD_NCURSES_CLIENT "Build the ncurses interface" OFF)
option(MAKE_MESSAGE_TEST    "Make the message testing executable" OFF)

if(BUILD_QT_CLIENT)
	# We need to locate Qt4
	find_package(Qt4 REQUIRED)

	# It's likely we'll use these modules
	set(QT_USE_QTNETWORK TRUE)
	set(QT_USE_QTTEST TRUE)
	include(${QT_USE_FILE})

	# Source, headers to moc, and UI files
	set(lirch_qt_interface_SOURCES main.cpp lirch_qt_interface.cpp)
	set(lirch_qt_interface_HEADERS lirch_qt_interface.h)
	set(lirch_qt_interface_FORMS "${CMAKE_BINARY_DIR}/lirch_qt_interface.ui")

	# Requisites for building with Qt4
	qt4_wrap_cpp(lirch_qt_interface_HEADERS_MOC ${lirch_qt_interface_HEADERS})
	qt4_wrap_ui(lirch_qt_interface_FORMS_HEADERS ${lirch_qt_interface_FORMS})
	add_definitions(${QT_DEFINITIONS})

	# Add details for this target
	add_executable(qlirch
		${lirch_qt_interface_SOURCES}
		${lirch_qt_interface_HEADERS_MOC}
		${lirch_qt_interface_FORMS_HEADERS})
	target_link_libraries(qlirch ${QT_LIBRARIES})
endif(BUILD_QT_CLIENT)

if(BUILD_NCURSES_CLIENT)
	find_library(NCURSES_LIBRARY_PATH ncurses)
	if(NCURSES_LIBRARY_PATH)
		# TODO ncurses build info
	else(NCURSES_LIBRARY_PATH)
		message(FATAL_ERROR "Cannot find libncurses!")
	endif(NCURSES_LIBRARY_PATH)
endif(BUILD_NCURSES_CLIENT)

if(MAKE_MESSAGE_TEST)
	add_executable(message_test message_test.cpp message_pipe.cpp message.cpp)
	# Hack to enable C++11 support
	set(CMAKE_CXX_FLAGS -std=c++0x)
	enable_testing()
	add_test(sendToPlugin message_test toplugin)
	add_test(sendToCore message_test tocore)
	add_test(readEmptyMessage message_test readempty)
	add_test(cycleMessage message_test cycle)
	add_test(exhaustMessages message_test exhaust)
	add_test(share1Pipe message_test sharing)
	add_test(messageOrder message_test order)
	add_test(multiPipe message_test copying)
endif(MAKE_MESSAGE_TEST)
