# Should envelope should be a static lib?
# 1) It's small and unlikely to grow large
# 2) If it changes, we want plugins (shared libs) to link the new version
# 3) We can enable PIC with the second line, so plugins will build
# Downside is that every plugin will contain it (~MB gain overall)
#add_library(envelope message.cpp)
#set_target_properties(envelope PROPERTIES COMPILE_FLAGS -fPIC)

# Build a shared library that practically everything uses
add_library(envelope SHARED message.cpp)

set(CORE_LIBRARIES envelope ; dl ; pthread)

if(BUILD_QT_CLIENT)
  # Use QtCore and QtGui
  set(QT_DONT_USE_QTCORE FALSE)
  set(QT_DONT_USE_QTGUI FALSE)
  set(QT_USE_QTNETWORK FALSE)
  set(QT_USE_QTTEST FALSE)
  include(${QT_USE_FILE})
  include_directories(${QT_INCLUDE_DIR})
  add_definitions(-DLIRCH_CORE_USE_QT)
  list(APPEND CORE_LIBRARIES ${QT_LIBRARIES})
endif(BUILD_QT_CLIENT)

# Build the core executable
add_executable(lirch core.cpp plugin_loader.cpp)
target_link_libraries(lirch ${CORE_LIBRARIES})
