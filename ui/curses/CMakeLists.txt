# Use only QtCore
set(QT_DONT_USE_QTCORE FALSE)
set(QT_DONT_USE_QTGUI TRUE)
set(QT_USE_QTNETWORK FALSE)
set(QT_USE_QTTEST FALSE)
include("${QT_USE_FILE}")
include_directories(
  "${QT_INCLUDE_DIR}"
)

# We need to check a variety of things
find_library(CURSES_LIBRARY_PATH ncursesw cursesw curses REQUIRED)

# FIXME hack to work around Debian
include_directories("/usr/include/ncursesw")

add_library(lirch-curses SHARED lirch_curses_interface.cpp)
target_link_libraries(lirch-curses envelope ${CURSES_LIBRARY_PATH} ${QT_LIBRARIES})
install(TARGETS lirch-curses DESTINATION lib)
