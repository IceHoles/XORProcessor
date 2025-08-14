cmake_minimum_required(VERSION 3.19)
project(FileXorProcessor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Concurrent)

qt_add_executable(FileXorProcessor
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    fileprocessor.h
    fileprocessor.cpp
    mainwindow.h
    mainwindow.ui
)

qt_standard_project_setup()

target_link_libraries(FileXorProcessor
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Concurrent
)

include(GNUInstallDirs)

install(TARGETS FileXorProcessor
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET FileXorProcessor
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
