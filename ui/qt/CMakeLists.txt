# Start with only QtCore and QtNetwork (maybe QtTest)
set(QT_DONT_USE_QTCORE FALSE)
set(QT_DONT_USE_QTGUI TRUE)
set(QT_USE_QTNETWORK TRUE)
set(QT_USE_QTTEST FALSE)
if(BUILD_TESTING)
  set(QT_USE_QTTEST TRUE)
endif(BUILD_TESTING)
include("${QT_USE_FILE}")
include_directories("${QT_INCLUDE_DIR}" "${QT_QTNETWORK_INCLUDE_DIR}")
if(BUILD_TESTING)
  include_directories("${QT_QTTEST_INCLUDE_DIR}")
endif(BUILD_TESTING)

# Build the common client code (static lib)
qt4_wrap_cpp(lirch_client_pipe_MOC_SOURCES ../lirch_client_pipe.h)
add_library(lirch-client ../lirch_client_pipe.cpp ${lirch_client_pipe_MOC_SOURCES})
target_link_libraries(lirch-client pthread ${QT_LIBRARIES} ${QT_QTNETWORK_LIBRARIES})
set_target_properties(lirch-client PROPERTIES COMPILE_FLAGS -fPIC)

# Build the actual plugin (shared lib)
add_library(lirch-gui SHARED lirch_qt_plugin.cpp)
target_link_libraries(lirch-gui envelope lirch-client)
install(TARGETS lirch-gui DESTINATION lib/lirch)

# Build actual GUI elements (needs the UI files included)
set(LIRCH_QT_DIR "${CMAKE_BINARY_DIR}/ui/qt")
include_directories("${LIRCH_QT_DIR}")
# Enable QtGui for the static lib
set(QT_DONT_USE_QTGUI FALSE)
set(QT_USE_QTNETWORK FALSE)
include("${QT_USE_FILE}")
include_directories("${QT_INCLUDE_DIR}")
# Source, headers to moc, UI, and resource files need prep
set(lirch_qt_interface_HEADERS
  lirch_channel.h
  lirch_qt_interface.h
  lirch_qlineedit_dialog.h
  lirch_qtabwidget.h
  lirch_setup_wizard.h)
set(lirch_qt_interface_SOURCES
  lirch_channel.cpp
  lirch_qt_interface.cpp
  lirch_qlineedit_dialog.cpp
  lirch_qtabwidget.cpp
  lirch_setup_wizard.cpp)
# These need to have some strings replaced
configure_file(lirch_qt_interface.ui "${LIRCH_QT_DIR}/lirch_qt_interface.ui")
configure_file(lirch_setup_wizard.ui "${LIRCH_QT_DIR}/lirch_setup_wizard.ui")
set(lirch_qt_interface_UI
  "${LIRCH_QT_DIR}/lirch_qt_interface.ui"
  lirch_qlineedit_dialog.ui
  "${LIRCH_QT_DIR}/lirch_setup_wizard.ui")
set(lirch_qt_interface_QRC lirch_qt_interface.qrc)
# Prepare each type of code
qt4_wrap_cpp(lirch_qt_interface_MOC_SOURCES ${lirch_qt_interface_HEADERS})
qt4_wrap_ui(lirch_qt_interface_UI_SOURCES ${lirch_qt_interface_UI})
qt4_add_resources(lirch_qt_interface_QRC_SOURCES ${lirch_qt_interface_QRC})
# Add details for this target
add_library(lirch-qt-interface
        ${lirch_qt_interface_SOURCES}
	${lirch_qt_interface_MOC_SOURCES}
	${lirch_qt_interface_UI_SOURCES}
	${lirch_qt_interface_QRC_SOURCES})
target_link_libraries(lirch-qt-interface lirch-client ${QT_LIBRARIES})
