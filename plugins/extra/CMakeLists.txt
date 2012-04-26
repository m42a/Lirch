## These plugins use no Qt:

# Timer plugin
add_library(timer SHARED timer.cpp)
target_link_libraries(timer envelope)

## These plugins use QtCore only:
set(QT_DONT_USE_QTCORE FALSE)
set(QT_DONT_USE_QTGUI TRUE)
set(QT_USE_QTNETWORK FALSE)
set(QT_USE_QTTEST FALSE)
include(${QT_USE_FILE})
include_directories(${QT_INCLUDE_DIR})

if(BUILD_TESTING)
  # Settings tester plugin
  add_library(settingstest SHARED settingstest.cpp)
  target_link_libraries(settingstest envelope ${QT_LIBRARIES})
endif(BUILD_TESTING)

## These plugins use QtNetwork, too:
set(QT_USE_QTNETWORK TRUE)
include(${QT_USE_FILE})
include_directories(${QT_QTNETWORK_INCLUDE_DIR})

# Tester plugin
if(UNIX AND BUILD_TESTING)
  add_library(linuxtester SHARED linuxtester.cpp)
  target_link_libraries(linuxtester envelope ${QT_LIBRARIES} ${QT_QTNETWORK_LIBRARIES})
endif(UNIX AND BUILD_TESTING)
