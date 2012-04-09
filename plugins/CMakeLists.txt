# Meatgrinder plugin
add_library(meatgrinder SHARED meatgrinder.cpp)
target_link_libraries(meatgrinder envelope)

# Timer plugin
add_library(timer SHARED timer.cpp)
target_link_libraries(timer envelope)

# Logger plugin (uses QtCore)
set(QT_DONT_USE_QTCORE FALSE)
set(QT_DONT_USE_QTGUI TRUE)
set(QT_USE_QTNETWORK FALSE)
set(QT_USE_QTTEST FALSE)
include(${QT_USE_FILE})
include_directories(${QT_INCLUDE_DIR})
add_library(logger SHARED logger.cpp)
target_link_libraries(logger envelope ${QT_LIBRARIES})

# Antenna plugin (uses QtNetwork)
set(QT_USE_QTNETWORK TRUE)
include(${QT_USE_FILE})
include_directories(${QT_QTNETWORK_INCLUDE_DIR})
add_library(antenna SHARED antenna.cpp)
target_link_libraries(antenna envelope ${QT_LIBRARIES} ${QT_NETWORK_LIBRARIES})
