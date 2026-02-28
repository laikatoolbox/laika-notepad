cmake_minimum_required(VERSION 3.16)

project(laika-notepad VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Concurrent)

set(TS_FILES laika-notepad_en_US.ts)

set(PROJECT_SOURCES
    Main.cpp
        MainWindow.cpp
        MainWindow.h
        MainWindow.ui
        ${TS_FILES}
)

qt_add_executable(laika-notepad
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    TextEdit.h TextEdit.cpp
    LineNumberArea.h
    settings/SettingsStore.h
    settings/Theme.h settings/Theme.cpp
    AboutDialog.h AboutDialog.cpp AboutDialog.ui
    Images.qrc
    SettingsDialog.h SettingsDialog.cpp SettingsDialog.ui
    TextStats.h
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
qt_add_translations(laika-notepad)
qt_add_lupdate(
    SOURCE_TARGETS laika-notepad
    TS_FILES  ${TS_FILES}
)
add_dependencies(laika-notepad update_translations)

target_link_libraries(laika-notepad PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(laika-notepad PRIVATE Qt${QT_VERSION_MAJOR}::Concurrent)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.laika-notepad)
endif()
set_target_properties(laika-notepad PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS laika-notepad
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(laika-notepad)
endif()
