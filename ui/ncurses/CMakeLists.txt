# Use only QtCore
set(QT_DONT_USE_QTCORE FALSE)
set(QT_DONT_USE_QTGUI TRUE)
set(QT_USE_QTNETWORK FALSE)
set(QT_USE_QTTEST FALSE)
include("${QT_USE_FILE}")
include_directories(
  "${QT_INCLUDE_DIR}"
)

find_library(NCURSES_LIBRARY_PATH ncurses REQUIRED)

add_library(lirch-ncurses SHARED lirch_ncurses_interface.cpp)
target_link_libraries(lirch-ncurses envelope ncurses ${QT_LIBRARIES})
install(TARGETS lirch-ncurses DESTINATION lib)
