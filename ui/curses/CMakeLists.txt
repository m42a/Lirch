# Use only QtCore
set(QT_DONT_USE_QTCORE FALSE)
set(QT_DONT_USE_QTGUI TRUE)
set(QT_USE_QTNETWORK FALSE)
set(QT_USE_QTTEST FALSE)
include("${QT_USE_FILE}")
include_directories(
  "${QT_INCLUDE_DIR}"
)

find_library(CURSES_LIBRARY_PATH curses REQUIRED)

add_library(lirch-curses SHARED lirch_curses_interface.cpp)
target_link_libraries(lirch-curses envelope curses ${QT_LIBRARIES})
install(TARGETS lirch-curses DESTINATION lib)
