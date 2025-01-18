cmake_minimum_required(VERSION 3.16)

project(LANChat VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

# Adding Boost with conan
#####################################################################
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_BINARY_DIR}/conan_toolchain.cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Boost REQUIRED)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found. Please install Boost.")
endif()
#####################################################################

# Adding documentation with doxygen
#####################################################################
add_subdirectory(docs)
#####################################################################


# Source files
# The source files include header files so we can view them in QT Creator.
file(GLOB serverFiles Server/*.cpp Server/*.h)
file(GLOB clientFiles Client/*.cpp Client/*.h)

set(SERVER_SOURCES
    server_main.cpp
    QtWindow/server_mainwindow.cpp
    QtWindow/server_mainwindow.h
    ${serverFiles}
    )

set(CLIENT_SOURCES
    client_main.cpp
    QtWindow/client_mainwindow.cpp
    QtWindow/client_mainwindow.h
    ${clientFiles}
    )

set(SERVER_DIRECTORIES
    Server)
set(CLIENT_DIRECTORIES
    Client)

function(configure_target target_name sources include_directories)
    if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
        qt_add_executable(${target_name}
            MANUAL_FINALIZATION
            ${sources}
        )
    # Define target properties for Android with Qt 6 as:
    #    set_property(TARGET ServerChat APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    #                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
    # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
    else()
        if(ANDROID)
            add_library(${target_name} SHARED
                ${sources}
            )

    # Define properties for Android with Qt 5 after find_package() calls as:
    #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
        else()
            add_executable(${target_name}
                ${sources}
            )
        endif()
    endif()

    target_include_directories(${target_name} PRIVATE ${Boost_INCLUDE_DIRS}
                                                      QtWindow
                                                      ${include_directories})

    target_link_libraries(${target_name} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
                                                 Qt${QT_VERSION_MAJOR}::Network
                                                 boost::boost)

    # Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
    # If you are developing for iOS or macOS you should consider setting an
    # explicit, fixed bundle identifier manually though.
    if(${QT_VERSION} VERSION_LESS 6.1.0)
      set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.${target_name})
    endif()

    set_target_properties(${target_name} PROPERTIES
        ${BUNDLE_ID_OPTION}
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
        MACOSX_BUNDLE TRUE
        WIN32_EXECUTABLE TRUE
    )
endfunction()

configure_target(ServerChat "${SERVER_SOURCES}" "${SERVER_DIRECTORIES}")
configure_target(ClientChat "${CLIENT_SOURCES}" "${CLIENT_DIRECTORIES}")

add_dependencies(ServerChat documentation)

# Installation
include(GNUInstallDirs)
install(TARGETS ServerChat ClientChat
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ServerChat)
    qt_finalize_executable(ClientChat)
endif()


