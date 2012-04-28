# TODO find out if is this only for windows
include(InstallRequiredSystemLibraries)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 1)

# Make the packager work independently of the build
if(NOT FROM_CMAKE_BUILD)
  # FIXME what to do about this:
  set(CPACK_PACKAGE_VERSION "beta-ubuntu1")
  if(UNIX)
    set(CPACK_CMAKE_GENERATOR "Unix Makefiles")
  endif(UNIX)
  set(CMAKE_SOURCE_DIR "..")
  set(CMAKE_BINARY_DIR ".")
  set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
  set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
  # TODO find out if this fixes it:
  #set(CPACK_INSTALL_CMAKE_PROJECTS ".;${CPACK_PACKAGE_FILE_NAME};ALL;/")
endif(NOT FROM_CMAKE_BUILD)

set(CPACK_PACKAGE_URL                  "https://www.github.com/m42a/Lirch/wiki")
set(CPACK_PACKAGE_VENDOR               "The Addams Family")
set(CPACK_PACKAGE_CONTACT              "Tor E. Hagemann <hagemt@rpi.edu>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY  "A local IRC host (lirch)")

set(CPACK_PACKAGE_DESCRIPTION_FILE     "${CMAKE_SOURCE_DIR}/NEWS")
set(CPACK_RESOURCE_FILE_LICENSE        "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README         "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_WELCOME        "${CMAKE_SOURCE_DIR}/NEWS")

set(CPACK_PACKAGE_INSTALL_DIRECTORY    "${CPACK_PACKAGE_FILE_NAME} ${CPACK_PACKAGE_VERSION}")
SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "${CPACK_PACKAGE_FILE_NAME} ${CPACK_PACKAGE_VERSION}")

set(CPACK_PACKAGE_VERSION_MAJOR       "${LIRCH_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR       "${LIRCH_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH       "${LIRCH_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION_TWEAK       "${LIRCH_VERSION_TWEAK}")

if(UNIX)
  # TODO make sure this is what we need
  #set(CPACK_DEBIAN_PACKAGE_DEPENDS      "package_name (>= min_version), ...")
  # Obvious requirements: libdl libpthread libQtGui libQtNetwork libQtCore libQtTest? libc(6) libgcc libstdc++ libm
  # Pulled from objdump -p on bin,lib: libpng libSM libICE libfreetype libXext libX11 libz libgthread libglib libgobject librt
  # Referenced elsewhere: GLIBCXX_3.4.14 CXXABI_1.3.5 GCC_3.0 GLIBC_2.2.5
  set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqtcore4 (>= 4:4.8.1-0ubuntu4), libqtgui4 (4:4.8.1-0ubuntu4), libqt4-network (4:4.8.1-0ubuntu4)")
  # TODO work out optional/recommended
  #set(CPACK_DEBIAN_PACKAGE_OPTIONAL? fortune)
  # This defaults to i386
  set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
  # TODO find a neater way
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
  endif(CMAKE_SIZEOF_VOID_P EQUAL 8)
  # Similar projects are in 'net'
  set(CPACK_DEBIAN_PACKAGE_SECTION "net")

  # TODO is any of this necessary for us? (if so, make it portable)
  #list(APPEND CPACK_SOURCE_STRIP_FILES "${CMAKE_BUILD_DIR}/lirch_constants.h")
  #set(CPACK_SOURCE_IGNORE_FILES "\\.swp$;.*~")
  #set(CPACK_SOURCE_PACKAGE_FILE_NAME "lirch-src")

  # Strip binaries
  list(APPEND CPACK_STRIP_FILES "${EXECUTABLE_OUTPUT_PATH}/lirch")
  if(BUILD_PLUGINS)
    list(APPEND CPACK_STRIP_FILES "${LIBRARY_OUTPUT_PATH}/libantenna.so")
    list(APPEND CPACK_STRIP_FILES "${LIBRARY_OUTPUT_PATH}/liblogger.so")
    list(APPEND CPACK_STRIP_FILES "${LIBRARY_OUTPUT_PATH}/libmasseuse.so")
    list(APPEND CPACK_STRIP_FILES "${LIBRARY_OUTPUT_PATH}/libmeatgrinder.so")
    list(APPEND CPACK_STRIP_FILES "${LIBRARY_OUTPUT_PATH}/libuserlist.so")
  endif(BUILD_PLUGINS)
  if(BUILD_BASIC_CLIENT)
    list(APPEND CPACK_STRIP_FILES "${LIBRARY_OUTPUT_PATH}/liblirch-basic.so")
    list(APPEND CPACK_STRIP_FILES "${EXECUTABLE_OUTPUT_PATH}/blirch")
  endif(BUILD_BASIC_CLIENT)
  if(BUILD_CURSES_CLIENT)
    list(APPEND CPACK_STRIP_FILES "${LIBRARY_OUTPUT_PATH}/liblirch-curses.so")
    list(APPEND CPACK_STRIP_FILES "${EXECUTABLE_OUTPUT_PATH}/clirch")
  endif(BUILD_CURSES_CLIENT)
  if(BUILD_QT_CLIENT)
    list(APPEND CPACK_STRIP_FILES "${LIBRARY_OUTPUT_PATH}/liblirch-gui.so")
    list(APPEND CPACK_STRIP_FILES "${EXECUTABLE_OUTPUT_PATH}/qlirch")
  endif(BUILD_QT_CLIENT)
endif(UNIX)

# For NSIS on native Windows
if(WIN32 AND NOT UNIX)
  set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/ui/qt/images/lirch.ico")
  set(CPACK_NSIS_INSTALLED_ICON_NAME "${CMAKE_BUILD_DIR}/bin/lirch.exe")
  set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} ${CPACK_DESCRIPTION_SUMMARY}")
  set(CPACK_NSIS_HELP_LINK "${CPACK_PACKAGE_URL}")
  set(CPACK_NSIS_URL_INFO_ABOUT "${CPACK_PACKAGE_URL}")
  set(CPACK_NSIS_CONTACT "${CPACK_PACKAGE_CONTACT}")
  set(CPACK_NSIS_MODIFY_PATH ON)
endif(WIN32 AND NOT UNIX)

# Shortcuts
if(BUILD_CURSES_CLIENT)
  list(APPEND CPACK_PACKAGE_EXECUTABLES "clirch" "Lirch (console)")
endif(BUILD_CURSES_CLIENT)
if(BUILD_QT_CLIENT)
  list(APPEND CPACK_PACKAGE_EXECUTABLES "qlirch" "Lirch (gui)")
endif(BUILD_QT_CLIENT)
