cmake_minimum_required(VERSION 3.21)
project(fedora-pm VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(CTest)
find_package(Threads REQUIRED)

option(FEDORAPM_BUILD_GUI "Build the Qt 6 Widgets GUI when Qt is available" ON)
option(FEDORAPM_BUILD_TESTS "Build unit tests" ON)

add_library(fedorapm_core
    src/core/process_runner.cpp
    src/core/validation.cpp
    src/system/system_info.cpp
    src/system/repository_manager.cpp
    src/system/kernel_manager.cpp
    src/system/disk_space_manager.cpp
    src/system/security_manager.cpp
    src/system/config_manager.cpp
    src/system/doctor.cpp
    src/system/driver_manager.cpp
    src/system/gaming_manager.cpp
    src/system/rollback_manager.cpp
    src/system/cachyos_manager.cpp
    src/system/history_manager.cpp
    src/package_manager/package_manager.cpp
    src/package_manager/flatpak_manager.cpp
    src/package_manager/export_manager.cpp
    src/package_manager/download_manager.cpp
    src/package_manager/changelog_manager.cpp
)
target_include_directories(fedorapm_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(fedorapm_core PUBLIC Threads::Threads)
target_compile_options(fedorapm_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(fedora-pm src/cli/main.cpp src/cli/cachyos_commands.cpp)
target_link_libraries(fedora-pm PRIVATE fedorapm_core)

if(FEDORAPM_BUILD_GUI)
    find_package(Qt6 COMPONENTS Widgets QUIET)
    if(Qt6_FOUND)
        set(CMAKE_AUTOMOC ON)
        add_executable(fedora-pm-gui src/gui/main.cpp src/gui/main_window.cpp)
        target_link_libraries(fedora-pm-gui PRIVATE fedorapm_core Qt6::Widgets)
        target_include_directories(fedora-pm-gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    else()
        message(STATUS "Qt 6 Widgets not found; skipping fedora-pm-gui (install qt6-qtbase-devel on Fedora)")
    endif()
endif()

if(FEDORAPM_BUILD_TESTS AND BUILD_TESTING)
    add_executable(fedorapm-tests tests/test_validation.cpp)
    target_link_libraries(fedorapm-tests PRIVATE fedorapm_core)
    add_test(NAME fedorapm-validation COMMAND fedorapm-tests)
endif()

install(TARGETS fedora-pm RUNTIME DESTINATION bin)
if(TARGET fedora-pm-gui)
    install(TARGETS fedora-pm-gui RUNTIME DESTINATION bin)
endif()
