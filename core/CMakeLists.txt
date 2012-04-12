# Should envelope should be a static lib?
# 1) It's small and unlikely to grow large
# 2) If it changes, we want plugins (shared libs) to link the new version
# 3) We can enable PIC with the second line, so plugins will build
# Downside is that every plugin will contain it (~MB gain overall)
#add_library(envelope message.cpp)
#set_target_properties(envelope PROPERTIES COMPILE_FLAGS -fPIC)

# Build a shared library that practically everything uses
add_library(envelope SHARED message.cpp)

if(BUILD_QT_CLIENT)
  # Use QtCore and QtGui
  set(QT_DONT_USE_QTCORE FALSE)
  set(QT_DONT_USE_QTGUI FALSE)
  set(QT_USE_QTNETWORK FALSE)
  set(QT_USE_QTTEST FALSE)
  include(${QT_USE_FILE})
  include_directories(${QT_INCLUDE_DIR})
  # Build the core executable with Qt GUI
  add_executable(qlirch lirch_qt.cpp core.cpp plugin_loader.cpp)
  target_link_libraries(qlirch envelope lirch-qt ${QT_LIBRARIES})
endif(BUILD_QT_CLIENT)

if(BUILD_NON_QT_CLIENT)
  # Build the core executable
  add_executable(lirch lirch_non_qt.cpp core.cpp plugin_loader.cpp)
  target_link_libraries(lirch envelope dl pthread)
endif(BUILD_NON_QT_CLIENT)
