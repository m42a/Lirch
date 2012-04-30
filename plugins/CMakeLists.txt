## Optional plugins
if(BUILD_PLUGINS_EXTRA OR BUILD_TESTING)
  add_subdirectory(extra)
endif(BUILD_PLUGINS_EXTRA OR BUILD_TESTING)

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

# Quip plugin
add_library(quip SHARED quip.cpp)
target_link_libraries(quip envelope ${QT_LIBRARIES})

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

# For install (packaging)
install(TARGETS antenna logger masseuse meatgrinder quip userlist DESTINATION lib/lirch)
