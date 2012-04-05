# Use only QtCore and QtNetwork
set(QT_DONT_USE_QTGUI TRUE)
set(QT_USE_QTNETWORK TRUE)
include(${QT_USE_FILE})
include_directories(
  ${QT_INCLUDE_DIR}
  ${QT_QTNETWORK_INCLUDE_DIR}
)

# Antenna plugin
# add_library(antenna SHARED antenna.cpp)
# target_link_libraries(antenna envelope ${QT_LIBRARIES} ${QT_NETWORK_LIBRARIES})

# Logger plugin
 add_library(logger SHARED logger.cpp)
 target_link_libraries(logger envelope ${QT_LIBRARIES})

# Meatgrinder plugin
add_library(meatgrinder SHARED meatgrinder.cpp)
target_link_libraries(meatgrinder envelope ${QT_LIBRARIES})

# Timer plugin
add_library(timer SHARED timer.cpp)
target_link_libraries(timer envelope)
