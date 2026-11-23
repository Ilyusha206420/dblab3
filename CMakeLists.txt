cmake_minimum_required(VERSION 3.16)
project(GuitarWorkshopApp VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

add_executable(GuitarWorkshopApp
    main.cpp
    database/DatabaseCredentialsStore.h
    database/DatabaseCredentialsStore.cpp
    database/DatabaseInitializer.h
    database/DatabaseInitializer.cpp
    ui/LoginDialog.h
    ui/LoginDialog.cpp
    ui/ReportsWidget.h
    ui/ReportsWidget.cpp
    ui/TableManagementWidget.h
    ui/TableManagementWidget.cpp
    ui/PostgresLoginDialog.h
    ui/PostgresLoginDialog.cpp
    ui/MainWindow.h
    ui/MainWindow.cpp
)

if(WIN32)
    set_target_properties(GuitarWorkshopApp PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

target_link_libraries(GuitarWorkshopApp PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
)

add_custom_command(TARGET GuitarWorkshopApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/database/schema.sql"
        "$<TARGET_FILE_DIR:GuitarWorkshopApp>/database.sql"
)

if(WIN32 AND VCPKG_INSTALLED_DIR AND VCPKG_TARGET_TRIPLET)
    set(QT_PLUGIN_DIR_RELEASE "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/Qt6/plugins")
    set(QT_PLUGIN_DIR_DEBUG "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/Qt6/plugins")
    set(VCPKG_BIN_DIR_RELEASE "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin")
    set(VCPKG_BIN_DIR_DEBUG "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/bin")

    if(EXISTS "${QT_PLUGIN_DIR_RELEASE}/platforms/qwindows.dll"
       AND EXISTS "${QT_PLUGIN_DIR_DEBUG}/platforms/qwindowsd.dll")
        add_custom_command(TARGET GuitarWorkshopApp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:GuitarWorkshopApp>/platforms"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<IF:$<CONFIG:Debug>,${QT_PLUGIN_DIR_DEBUG}/platforms/qwindowsd.dll,${QT_PLUGIN_DIR_RELEASE}/platforms/qwindows.dll>"
                "$<TARGET_FILE_DIR:GuitarWorkshopApp>/platforms/$<IF:$<CONFIG:Debug>,qwindowsd.dll,qwindows.dll>"
        )
    endif()

    if(EXISTS "${QT_PLUGIN_DIR_RELEASE}/sqldrivers/qsqlpsql.dll"
       AND EXISTS "${QT_PLUGIN_DIR_DEBUG}/sqldrivers/qsqlpsqld.dll"
       AND EXISTS "${VCPKG_BIN_DIR_RELEASE}/libpq.dll"
       AND EXISTS "${VCPKG_BIN_DIR_DEBUG}/libpq.dll"
       AND EXISTS "${VCPKG_BIN_DIR_RELEASE}/libssl-3-x64.dll"
       AND EXISTS "${VCPKG_BIN_DIR_DEBUG}/libssl-3-x64.dll"
       AND EXISTS "${VCPKG_BIN_DIR_RELEASE}/libcrypto-3-x64.dll"
       AND EXISTS "${VCPKG_BIN_DIR_DEBUG}/libcrypto-3-x64.dll")
        add_custom_command(TARGET GuitarWorkshopApp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:GuitarWorkshopApp>/sqldrivers"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<IF:$<CONFIG:Debug>,${QT_PLUGIN_DIR_DEBUG}/sqldrivers/qsqlpsqld.dll,${QT_PLUGIN_DIR_RELEASE}/sqldrivers/qsqlpsql.dll>"
                "$<TARGET_FILE_DIR:GuitarWorkshopApp>/sqldrivers/$<IF:$<CONFIG:Debug>,qsqlpsqld.dll,qsqlpsql.dll>"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<IF:$<CONFIG:Debug>,${VCPKG_BIN_DIR_DEBUG}/libpq.dll,${VCPKG_BIN_DIR_RELEASE}/libpq.dll>"
                "$<TARGET_FILE_DIR:GuitarWorkshopApp>/libpq.dll"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<IF:$<CONFIG:Debug>,${VCPKG_BIN_DIR_DEBUG}/libssl-3-x64.dll,${VCPKG_BIN_DIR_RELEASE}/libssl-3-x64.dll>"
                "$<TARGET_FILE_DIR:GuitarWorkshopApp>/libssl-3-x64.dll"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<IF:$<CONFIG:Debug>,${VCPKG_BIN_DIR_DEBUG}/libcrypto-3-x64.dll,${VCPKG_BIN_DIR_RELEASE}/libcrypto-3-x64.dll>"
                "$<TARGET_FILE_DIR:GuitarWorkshopApp>/libcrypto-3-x64.dll"
        )
    endif()
endif()
