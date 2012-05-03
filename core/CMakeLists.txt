# Should envelope should be a static lib?
# 1) It's small and unlikely to grow large
# 2) If it changes, we want plugins (shared libs) to link the new version
# 3) We can enable PIC with the second line, so plugins will build
# 4) Everything else we put in lib that's shared is a plugin
# Downside is that every plugin will contain it (~MB gain overall)
add_library(envelope message.cpp)
set_target_properties(envelope PROPERTIES COMPILE_FLAGS -fPIC)

# Build a shared library that practically everything uses
#add_library(envelope SHARED message.cpp)

# Build the core library
add_library(lirch-core core.cpp plugin_loader.cpp)
target_link_libraries(lirch-core envelope dl pthread)

# Prepare the core's QtCore mediator
set(QT_DONT_USE_QTCORE FALSE)
set(QT_DONT_USE_QTGUI TRUE)
set(QT_USE_QTNETWORK FALSE)
set(QT_USE_QTTEST FALSE)
if(BUILD_TESTING)
  set(QT_USE_QTTEST TRUE)
endif(BUILD_TESTING)
include("${QT_USE_FILE}")
include_directories("${QT_INCLUDE_DIR}" "${CMAKE_SOURCE_DIR}/tclap/include")
qt4_wrap_cpp(LIRCH_QTCORE_MOC lirch_qtcore.h)

# Build the core executable with QtCore and no UI
add_executable(lirch alirch_qtcore.cpp ${LIRCH_QTCORE_MOC})
target_link_libraries(lirch lirch-core ${QT_LIBRARIES})
install(TARGETS lirch DESTINATION bin)

if(BUILD_CLIENT_BASIC)
  # Build the core executable with QtCore and basic UI
  add_executable(blirch blirch_qtcore.cpp ${LIRCH_QTCORE_MOC})
  target_link_libraries(blirch lirch-core ${QT_LIBRARIES})
  install(TARGETS blirch DESTINATION bin)
endif(BUILD_CLIENT_BASIC)

if(BUILD_CLIENT_CURSES)
  # Build the core executable with QtCore and curses UI
  add_executable(clirch clirch_qtcore.cpp ${LIRCH_QTCORE_MOC})
  target_link_libraries(clirch lirch-core ${QT_LIBRARIES})
  install(TARGETS clirch DESTINATION bin)
endif(BUILD_CLIENT_CURSES)

if(BUILD_CLIENT_QTGUI)
  # Use QtCore, QtGui, and QtNetwork
  set(QT_DONT_USE_QTGUI FALSE)
  set(QT_USE_QTNETWORK TRUE)
  include("${QT_USE_FILE}")
  include_directories("${QT_INCLUDE_DIR}" "${QT_QTNETWORK_INCLUDE_DIR}")
  # Build the core executable with QtGUI and qt UI
  add_executable(qlirch qlirch_qtgui.cpp)
  target_link_libraries(qlirch lirch-core lirch-qt-interface)
  install(TARGETS qlirch DESTINATION bin)
endif(BUILD_CLIENT_QTGUI)
