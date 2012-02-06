cmake_minimum_required(VERSION 2.8.0)
project(Lirch C CXX)

# Force a build type to be declared
set(CMAKE_BUILD_TYPES "(None|Debug|Release|RelWithDebInfo|MinSizeRel)")
if(NOT CMAKE_BUILD_TYPE MATCHES "${CMAKE_BUILD_TYPES}")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "${CMAKE_BUILD_TYPES}" FORCE)
endif(NOT CMAKE_BUILD_TYPE MATCHES "${CMAKE_BUILD_TYPES}")

# Basic build options
option(BUILD_QT_CLIENT      "Build the Qt4 interface" OFF)
option(BUILD_NCURSES_CLIENT "Build the ncurses interface" OFF)
option(MAKE_MESSAGE_TEST    "Make the message testing executable" OFF)

if(BUILD_QT_CLIENT)
	# We need to locate Qt4
	find_package(Qt4 REQUIRED)

	# It's likely we'll use these modules
	set(QT_USE_QTNETWORK TRUE)
	set(QT_USE_QTTEST TRUE)
	include(${QT_USE_FILE})
	# Source, headers, and UI
	set(client_SOURCES main.cpp client.cpp)
	set(client_HEADERS client.h)
	set(client_FORMS client.ui)

	# Requisites for building with Qt4
	qt4_wrap_cpp(client_HEADERS_MOC ${client_HEADERS})
	qt4_wrap_ui(client_FORMS_HEADERS ${client_FORMS})
	add_definitions(${QT_DEFINITIONS})

	# Add details for the client target
	include_directories(${CMAKE_BINARY_DIR})
	add_executable(client	${client_SOURCES}
				${client_HEADERS_MOC}
				${client_FORMS_HEADERS})
	target_link_libraries(client ${QT_LIBRARIES})
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
	add_test(testingWorks message_test)
	add_test(sendToPlugin message_test toplugin)
	add_test(sendToCore message_test tocore)
endif(MAKE_MESSAGE_TEST)
