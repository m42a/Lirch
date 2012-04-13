# Start with only QtCore (maybe QtTest)
set(QT_DONT_USE_QTCORE FALSE)
set(QT_DONT_USE_QTGUI TRUE)
set(QT_USE_QTNETWORK FALSE)
if(BUILD_TESTING)
  set(QT_USE_QTTEST TRUE)
endif(BUILD_TESTING)
include("${QT_USE_FILE}")
include_directories("${QT_INCLUDE_DIR}")
if(BUILD_TESTING)
  include_directories("${QT_QTTEST_INCLUDE_DIR}")
endif(BUILD_TESTING)
# Build the shared client code (TODO shared lib? is this a kludge?)
qt4_wrap_cpp(lirch_client_pipe_MOC_SOURCES ../lirch_client_pipe.h)
add_library(lirch-client ../lirch_client_pipe.cpp ${lirch_client_pipe_MOC_SOURCES})
target_link_libraries(lirch-client ${QT_LIBRARIES})
set_target_properties(lirch-client PROPERTIES COMPILE_FLAGS -fPIC)
# Build the actual plugin
add_library(lirch-qt SHARED lirch_qt_plugin.cpp)
target_link_libraries(lirch-qt envelope lirch-client ${QT_LIBRARIES})
install(TARGETS lirch-qt DESTINATION lib)

# The actual GUI needs the UI files included from here:
set(LIRCH_QT_DIR "${CMAKE_BINARY_DIR}/ui/qt")
include_directories("${LIRCH_QT_DIR}")
# Enable QtGui for the static lib
set(QT_DONT_USE_QTGUI FALSE)
include("${QT_USE_FILE}")
include_directories("${QT_INCLUDE_DIR}")

# Source, headers to moc, UI, and resource files need prep
set(lirch_qt_interface_HEADERS lirch_qt_interface.h lirch_qlineedit_dialog.h)
set(lirch_qt_interface_SOURCES lirch_qt_interface.cpp lirch_qlineedit_dialog.cpp)
# Needs to have the version string replaced
configure_file(lirch_qt_interface.ui "${LIRCH_QT_DIR}/lirch_qt_interface.ui")
set(lirch_qt_interface_UI "${LIRCH_QT_DIR}/lirch_qt_interface.ui" lirch_qlineedit_dialog.ui)
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
