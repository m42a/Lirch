# Use only QtCore
set(QT_DONT_USE_QTCORE FALSE)
set(QT_DONT_USE_QTGUI TRUE)
set(QT_USE_QTNETWORK FALSE)
set(QT_USE_QTTEST FALSE)
include("${QT_USE_FILE}")
include_directories(
  "${QT_INCLUDE_DIR}"
)

add_library(lirch-basic SHARED basic.cpp)
target_link_libraries(lirch-basic envelope ${QT_LIBRARIES})
install(TARGETS lirch-basic DESTINATION lirch)
