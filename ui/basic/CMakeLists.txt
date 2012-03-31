# Use only QtCore
set(QT_DONT_USE_QTGUI TRUE)
# set(QT_USE_QTNETWORK TRUE)
include("${QT_USE_FILE}")
include_directories(
  "${QT_INCLUDE_DIR}"
  "${QT_QTNETWORK_INCLUDE_DIR}"
)

add_library(lirch-basic SHARED basic.cpp)
target_link_libraries(lirch-basic envelope ${QT_LIBRARIES})
