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
#install(TARGETS envelope DESTINATION lib)

# Build the core library
add_library(lirch-core core.cpp plugin_loader.cpp)
target_link_libraries(lirch-core envelope dl pthread)

if(BUILD_CLIENT_NON_QT)
  # Build the client executable
  add_executable(lirch lirch_non_qt.cpp)
  target_link_libraries(lirch lirch-core)
  install(TARGETS lirch DESTINATION bin)
endif(BUILD_CLIENT_NON_QT)

if(BUILD_CLIENT_QT)
  # Use QtCore and QtGui
  set(QT_DONT_USE_QTCORE FALSE)
  set(QT_DONT_USE_QTGUI FALSE)
  set(QT_USE_QTNETWORK FALSE)
  set(QT_USE_QTTEST FALSE)
  include(${QT_USE_FILE})
  include_directories(${QT_INCLUDE_DIR})
  # Build the core executable with Qt GUI
  add_executable(qlirch lirch_qt.cpp)
  target_link_libraries(qlirch lirch-core lirch-qt-interface ${QT_LIBRARIES})
  install(TARGETS qlirch DESTINATION bin)
endif(BUILD_CLIENT_QT)
