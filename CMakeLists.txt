cmake_minimum_required(VERSION 3.16)

project(tagviewer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Multimedia)
find_package(Qt6 REQUIRED COMPONENTS MultimediaWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(json)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/tagviewer.rc")

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        include/mainwindow.h
        ui/mainwindow.ui
)




if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(tagviewer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        resources/icons.qrc

        include/pixmaplabel.h src/pixmaplabel.cpp
        include/taglistwidgetitem.h src/taglistwidgetitem.cpp

        include/tagtreeitem.h src/tagtreeitem.cpp
        include/videoplayer.h src/videoplayer.cpp ui/videoplayer.ui

        include/taglistwidget.h src/taglistwidget.cpp
        include/filelistwidget.h src/filelistwidget.cpp ui/filelistwidget.ui

        include/jsonnode.h src/jsonnode.cpp
        src/tagtree.cpp include/tagtree.h

        include/icondialog.h src/icondialog.cpp ui/icondialog.ui
        include/newtagdialog.h src/newtagdialog.cpp ui/newtagdialog.ui

        include/iconpanel.h src/iconpanel.cpp ui/iconpanel.ui
        include/icontable.h src/icontable.cpp
        ${app_icon_resource_windows}
        include/mediadisplay.h src/mediadisplay.cpp ui/mediadisplay.ui
        include/tageditor.h src/tageditor.cpp ui/tageditor.ui
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET tagviewer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(tagviewer SHARED
            ${PROJECT_SOURCES}

        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(tagviewer
            ${PROJECT_SOURCES}
            ${app_icon_resource_windows}
        )
    endif()
endif()


target_include_directories(tagviewer PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(tagviewer PRIVATE
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    nlohmann_json::nlohmann_json
)
target_link_libraries(tagviewer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.tagviewer)
endif()
set_target_properties(tagviewer PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS tagviewer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(tagviewer)
endif()
