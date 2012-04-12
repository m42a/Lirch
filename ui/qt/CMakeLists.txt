# Use QtCore and QtGui, and QtTest (eventually)
set(QT_DONT_USE_QTCORE FALSE)
set(QT_DONT_USE_QTGUI FALSE)
set(QT_USE_QTNETWORK FALSE)
if(BUILD_TESTING)
  set(QT_USE_QTTEST TRUE)
endif(BUILD_TESTING)
include("${QT_USE_FILE}")

# Set include directories 
set(LIRCH_QT_DIR "${CMAKE_BINARY_DIR}/ui/qt")
include_directories(
  "${QT_INCLUDE_DIR}"
  "${LIRCH_QT_DIR}"
)
if(BUILD_TESTING)
  include_directories(
    "${QT_QTTEST_INCLUDE_DIR}"
  )
endif(BUILD_TESTING)

# Source, headers to moc, UI, and resource files need prep
set(lirch_qt_interface_HEADERS lirch_qt_interface.h lirch_qlineedit_dialog.h)
set(lirch_qt_interface_SOURCES lirch_qt_interface.cpp lirch_qlineedit_dialog.cpp)
# Needs to have the version string replaced
configure_file(lirch_qt_interface.ui "${LIRCH_QT_DIR}/lirch_qt_interface.ui")
configure_file(lirch_qlineedit_dialog.ui "${LIRCH_QT_DIR}/lirch_qlineedit_dialog.ui")
set(lirch_qt_interface_UI "${LIRCH_QT_DIR}/lirch_qt_interface.ui" "${LIRCH_QT_DIR}/lirch_qlineedit_dialog.ui")
set(lirch_qt_interface_QRC lirch_qt_interface.qrc)

# Prepare the code
qt4_wrap_cpp(lirch_qt_interface_MOC_HEADERS ${lirch_qt_interface_HEADERS})
qt4_wrap_ui(lirch_qt_interface_UI_HEADERS ${lirch_qt_interface_UI})
qt4_add_resources(lirch_qt_interface_QRC_HEADERS ${lirch_qt_interface_QRC})

# Add details for this target
add_library(lirch-qt SHARED
	${lirch_qt_interface_SOURCES}
	${lirch_qt_interface_MOC_HEADERS}
	${lirch_qt_interface_UI_HEADERS}
	${lirch_qt_interface_QRC_HEADERS})
target_link_libraries(lirch-qt envelope ${QT_LIBRARIES})
