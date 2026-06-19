cmake_minimum_required(VERSION 3.16)

project(KylinSpaceCleaner VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Charts)

add_executable(kylin-space-cleaner
    src/space_cleaner.cpp
)
target_link_libraries(kylin-space-cleaner PRIVATE Qt5::Core Qt5::Widgets Qt5::Charts)

add_executable(kylin-space-cleaner-helper
    src/space_helper.cpp
)
target_link_libraries(kylin-space-cleaner-helper PRIVATE Qt5::Core)
