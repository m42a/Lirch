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

# Logger plugin
add_library(logger SHARED logger.cpp)
target_link_libraries(logger envelope ${QT_LIBRARIES})
 
# Masseuse plugin
add_library(masseuse SHARED masseuse.cpp)
target_link_libraries(masseuse envelope ${QT_LIBRARIES})

# Meatgrinder plugin
add_library(meatgrinder SHARED meatgrinder.cpp)
target_link_libraries(meatgrinder envelope ${QT_LIBRARIES})

## These plugins use QtNetwork, too:
set(QT_USE_QTNETWORK TRUE)
include(${QT_USE_FILE})
include_directories(${QT_QTNETWORK_INCLUDE_DIR})

# Antenna plugin
add_library(antenna SHARED antenna.cpp)
target_link_libraries(antenna envelope ${QT_LIBRARIES} ${QT_QTNETWORK_LIBRARIES})

# Userlist plugin
add_library(userlist SHARED userlist.cpp)
target_link_libraries(userlist envelope ${QT_LIBRARIES} ${QT_QTNETWORK_LIBRARIES})

# Tester plugin
if(UNIX)
  add_library(linuxtester SHARED linuxtester.cpp)
  target_link_libraries(linuxtester envelope ${QT_LIBRARIES} ${QT_QTNETWORK_LIBRARIES})
endif(UNIX)

# For install (packaging)
install(TARGETS antenna logger masseuse meatgrinder userlist DESTINATION lib/lirch)
