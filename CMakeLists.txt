cmake_minimum_required(VERSION 2.8.0)
project(Lirch C CXX)

# Version info
set(LIRCH_VERSION_MAJOR 0)
set(LIRCH_VERSION_MINOR 1)
set(LIRCH_VERSION_PATCH 0)
set(LIRCH_VERSION_TWEAK 1)
set(LIRCH_VERSION_STRING "${LIRCH_VERSION_MAJOR}.${LIRCH_VERSION_MINOR}.${LIRCH_VERSION_PATCH}.${LIRCH_VERSION_TWEAK}")
configure_file(client.ui "${CMAKE_BINARY_DIR}/client.ui")
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

if(BUILD_QT_CLIENT)
	# We need to locate Qt4
	find_package(Qt4 REQUIRED)

	# It's likely we'll use these modules
	set(QT_USE_QTNETWORK TRUE)
	set(QT_USE_QTTEST TRUE)
	include(${QT_USE_FILE})

	# Source, headers to moc, and UI files
	set(client_SOURCES main.cpp client.cpp)
	set(client_HEADERS client.h)
	set(client_FORMS "${CMAKE_BINARY_DIR}/client.ui")

	# Requisites for building with Qt4
	qt4_wrap_cpp(client_HEADERS_MOC ${client_HEADERS})
	qt4_wrap_ui(client_FORMS_HEADERS ${client_FORMS})
	add_definitions(${QT_DEFINITIONS})

	# Add details for the client target
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
