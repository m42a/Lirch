# Use QtCore and QtGui, QtNetwork and QtTest (eventually)
set(QT_DONT_USE_QTCORE FALSE)
set(QT_DONT_USE_QTGUI FALSE)
set(QT_USE_QTNETWORK TRUE)
set(QT_USE_QTTEST TRUE)
include("${QT_USE_FILE}")
set(LIRCH_QT_DIR "${CMAKE_BINARY_DIR}/ui/qt")
include_directories(
  "${QT_INCLUDE_DIR}"
  "${QT_QTNETWORK_INCLUDE_DIR}"
  "${QT_QTTEST_INCLUDE_DIR}"
  "${LIRCH_QT_DIR}"
)

# Needs to have the version string replaced
configure_file(lirch_qt_interface.ui "${LIRCH_QT_DIR}/lirch_qt_interface.ui")
# Source, headers to moc, UI, and resource files need prep
set(lirch_qt_interface_SOURCES lirch_qt_main.cpp lirch_qt_interface.cpp)
set(lirch_qt_interface_HEADERS lirch_qt_interface.h)
set(lirch_qt_interface_UI "${LIRCH_QT_DIR}/lirch_qt_interface.ui")
set(lirch_qt_interface_QRC lirch_qt_interface.qrc)
qt4_wrap_cpp(lirch_qt_interface_MOC_HEADERS ${lirch_qt_interface_HEADERS})
qt4_wrap_ui(lirch_qt_interface_UI_HEADERS ${lirch_qt_interface_UI})
qt4_add_resources(lirch_qt_interface_QRC_HEADERS ${lirch_qt_interface_QRC})
# Add details for this target
add_executable(lirch-qt
	${lirch_qt_interface_SOURCES}
	${lirch_qt_interface_MOC_HEADERS}
	${lirch_qt_interface_UI_HEADERS}
	${lirch_qt_interface_QRC_HEADERS})
target_link_libraries(lirch-qt envelope ${QT_LIBRARIES})

