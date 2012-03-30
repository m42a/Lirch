# Use only QtCore and QtNetwork
set(QT_DONT_USE_QTGUI TRUE)
set(QT_USE_QTNETWORK TRUE)
include("${QT_USE_FILE}")
include_directories(
  "${QT_INCLUDE_DIR}"
  "${QT_QTNETWORK_INCLUDE_DIR}"
)

find_library(NCURSES_LIBRARY_PATH ncurses REQUIRED)
# add_executable(lirch-ncurses lirch_ncurses_interface.cpp)
# target_link_libraries(lirch-ncurses envelope ncurses ${QT_LIBRARIES})
