cmake_minimum_required(VERSION 3.20)

project(FourPillarsOfDestiny
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)
find_package(KF6Kirigami REQUIRED)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appFourPillarsOfDestiny
    main.cpp
    AppController.cpp
    AppController.h
)

qt_add_qml_module(appFourPillarsOfDestiny
    URI org.example.fourpillarsofdestiny
    VERSION 1.0
    QML_FILES
        Main.qml
        pages/HomePage.qml
        pages/InputPage.qml
)

target_link_libraries(appFourPillarsOfDestiny PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    KF6::Kirigami
)

include(GNUInstallDirs)
install(TARGETS appFourPillarsOfDestiny
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
