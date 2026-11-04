cmake_minimum_required(VERSION 3.24)
project(omacap VERSION 0.1.0 LANGUAGES C CXX)
include(GNUInstallDirs)
include(CTest)
set(CMAKE_AUTOMOC ON)

find_package(PkgConfig REQUIRED)
find_package(Qt6 6.8 REQUIRED COMPONENTS Concurrent Core Gui Test Widgets)
find_package(LayerShellQt REQUIRED)
pkg_check_modules(WaylandClient REQUIRED IMPORTED_TARGET wayland-client)
pkg_check_modules(Libdeflate REQUIRED IMPORTED_TARGET libdeflate)
find_program(WAYLAND_SCANNER wayland-scanner REQUIRED)

set(PROTOCOL_XMLS
  # ext-image-capture-source references this interface even for output-only use.
  /usr/share/wayland-protocols/staging/ext-foreign-toplevel-list/ext-foreign-toplevel-list-v1.xml
  /usr/share/wayland-protocols/staging/ext-image-capture-source/ext-image-capture-source-v1.xml
  /usr/share/wayland-protocols/staging/ext-image-copy-capture/ext-image-copy-capture-v1.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/protocols/wlr-virtual-pointer-unstable-v1.xml
)
set(PROTOCOL_SOURCES)
foreach(PROTOCOL_XML IN LISTS PROTOCOL_XMLS)
  get_filename_component(PROTOCOL_NAME "${PROTOCOL_XML}" NAME_WE)
  set(PROTOCOL_HEADER "${CMAKE_CURRENT_BINARY_DIR}/${PROTOCOL_NAME}-client-protocol.h")
  set(PROTOCOL_CODE "${CMAKE_CURRENT_BINARY_DIR}/${PROTOCOL_NAME}-protocol.c")
  add_custom_command(
    OUTPUT "${PROTOCOL_HEADER}" "${PROTOCOL_CODE}"
    COMMAND "${WAYLAND_SCANNER}" client-header "${PROTOCOL_XML}" "${PROTOCOL_HEADER}"
    COMMAND "${WAYLAND_SCANNER}" private-code "${PROTOCOL_XML}" "${PROTOCOL_CODE}"
    DEPENDS "${PROTOCOL_XML}"
    VERBATIM
  )
  list(APPEND PROTOCOL_SOURCES "${PROTOCOL_HEADER}" "${PROTOCOL_CODE}")
endforeach()

add_library(omacap-core STATIC
  src/cli-path.cpp
  src/cli-path.hpp
  src/icons.cpp
  src/icons.hpp
  src/instance-lock.cpp
  src/instance-lock.hpp
  src/png.cpp
  src/png.hpp
  src/capture.cpp
  src/capture.hpp
  src/cut.cpp
  src/cut.hpp
  src/palette-config.cpp
  src/palette-config.hpp
  src/background-config.cpp
  src/background-config.hpp
  src/capture-handoff.cpp
  src/capture-handoff.hpp
  src/output-config.cpp
  src/output-config.hpp
  src/overlay-chrome.cpp
  src/overlay-chrome.hpp
  src/shortcut-guide.cpp
  src/shortcut-guide.hpp
  src/chrome-theme.cpp
  src/chrome-theme.hpp
  src/eyedropper.cpp
  src/eyedropper.hpp
  src/surface-capture.cpp
  src/stitch.cpp
  src/stitch.hpp
  src/text-band.cpp
  src/text-band.hpp
  src/auto-capture.cpp
  src/auto-capture.hpp
  src/scroll-capture.cpp
  src/scroll-capture.hpp
  src/scroll-inject.cpp
  src/scroll-inject.hpp
  src/startup-timing.cpp
  src/startup-timing.hpp
  src/stroke-smoothing.cpp
  src/stroke-smoothing.hpp
  ${PROTOCOL_SOURCES}
  src/editor.cpp
  src/editor-save-as.cpp
  src/editor.hpp
)
qt_add_resources(omacap-core "capture-fonts"
  PREFIX "/fonts"
  BASE "${CMAKE_CURRENT_SOURCE_DIR}/assets"
  FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/Neucha.ttf"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/JetBrainsMono-Regular.ttf"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/InterDisplay-SemiBold.ttf"
)

# CXX-only: omacap-core also compiles Wayland protocol .c files.
target_precompile_headers(omacap-core PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:<QtWidgets/QWidget$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtWidgets/QApplication$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtGui/QPainter$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtGui/QPainterPath$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtGui/QImage$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtGui/QPixmap$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtGui/QFontDatabase$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtGui/QMouseEvent$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtGui/QKeyEvent$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtGui/QWheelEvent$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtConcurrent/QtConcurrentRun$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtCore/QFutureWatcher$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtCore/QProcess$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtCore/QFile$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtCore/QDir$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtCore/QFileInfo$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtCore/QJsonObject$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtCore/QJsonArray$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtCore/QJsonDocument$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtCore/QTimer$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtCore/QString$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<QtCore/QVector$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<algorithm$<ANGLE-R>>"
  "$<$<COMPILE_LANGUAGE:CXX>:<cmath$<ANGLE-R>>"
)

target_include_directories(omacap-core PUBLIC
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
target_compile_features(omacap-core PUBLIC cxx_std_23)
target_compile_options(omacap-core PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(omacap-core PUBLIC
  Qt6::Concurrent
  Qt6::Core
  Qt6::Gui
  Qt6::Widgets
  LayerShellQt::Interface
  PkgConfig::WaylandClient
  PkgConfig::Libdeflate
)

qt_add_executable(omacap src/main.cpp)
qt_add_resources(omacap "application-icon"
  PREFIX "/"
  FILES assets/omacap.png
)
target_compile_options(omacap PRIVATE -Wall -Wextra -Wpedantic)
target_compile_definitions(omacap PRIVATE OMACAP_VERSION="${PROJECT_VERSION}")
target_link_libraries(omacap PRIVATE omacap-core)
target_precompile_headers(omacap REUSE_FROM omacap-core)

if(BUILD_TESTING)
qt_add_executable(omacap-smoke
  tests/scroll-focus-smoke.cpp
  tests/scroll-focus-smoke.hpp
  tests/chrome-theme-smoke.cpp
  tests/chrome-theme-smoke.hpp
  tests/editor-smoke.cpp
  tests/capture-handoff-smoke.cpp
  tests/capture-handoff-smoke.hpp
  tests/selection-repaint-smoke.cpp
  tests/selection-repaint-smoke.hpp
  tests/clipboard-smoke.cpp
  tests/clipboard-smoke.hpp
  tests/png-smoke.cpp
  tests/png-smoke.hpp
  tests/instance-lock-smoke.cpp
  tests/instance-lock-smoke.hpp
  tests/transform-smoke.cpp
  tests/transform-smoke.hpp
  tests/cut-smoke.cpp
  tests/cut-smoke.hpp
  tests/cut-mapping-smoke.cpp
  tests/cut-mapping-smoke.hpp
  tests/palette-config-smoke.cpp
  tests/palette-config-smoke.hpp
  tests/stroke-smoothing-smoke.cpp
  tests/stroke-smoothing-smoke.hpp
  src/cli-path.cpp
  src/capture-handoff.cpp
  src/capture-handoff.hpp
  src/cli-path.hpp
  src/capture.cpp
  src/capture.hpp
  src/overlay-chrome.cpp
  src/overlay-chrome.hpp
  tests/surface-capture-smoke.cpp
  tests/surface-capture-smoke.hpp
  src/stitch.cpp
  src/stitch.hpp
  src/auto-capture.cpp
  src/auto-capture.hpp
  tests/stitch-smoke.cpp
  tests/stitch-smoke.hpp
  tests/scroll-startup-smoke.cpp
  tests/scroll-startup-smoke.hpp
  tests/save-as-smoke.cpp
  tests/save-as-smoke.hpp
)
target_include_directories(omacap-smoke PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/tests"
)
target_compile_options(omacap-smoke PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(omacap-smoke PRIVATE omacap-core Qt6::Test)
target_precompile_headers(omacap-smoke REUSE_FROM omacap-core)
add_test(NAME omacap-smoke COMMAND omacap-smoke)
set_tests_properties(omacap-smoke PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()

qt_add_executable(stitch-replay
  src/stitch-replay.cpp
  src/stitch.cpp
  src/stitch.hpp
)
target_include_directories(stitch-replay PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
target_compile_features(stitch-replay PRIVATE cxx_std_23)
target_compile_options(stitch-replay PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(stitch-replay PRIVATE Qt6::Core Qt6::Gui)

install(TARGETS omacap
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
install(FILES assets/OFL.txt
  DESTINATION "${CMAKE_INSTALL_DATADIR}/licenses/omacap"
  RENAME Neucha-OFL.txt
)
install(FILES assets/JetBrainsMono-OFL.txt assets/Inter-OFL.txt assets/Lucide-ISC.txt
  DESTINATION "${CMAKE_INSTALL_DATADIR}/licenses/omacap"
)
install(FILES omacap.desktop
  DESTINATION "${CMAKE_INSTALL_DATADIR}/applications"
)
install(FILES assets/omacap.svg
  DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps"
)
