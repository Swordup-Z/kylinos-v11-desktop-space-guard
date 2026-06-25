cmake_minimum_required(VERSION 3.16)

project(KylinSpaceCleaner VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(GNUInstallDirs)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Charts)

add_executable(kylin-space-cleaner
    src/space_cleaner.cpp
    resources.qrc
)
target_link_libraries(kylin-space-cleaner PRIVATE Qt5::Core Qt5::Widgets Qt5::Charts)

add_executable(kylin-space-cleaner-helper
    src/space_helper.cpp
)
target_link_libraries(kylin-space-cleaner-helper PRIVATE Qt5::Core)

install(TARGETS kylin-space-cleaner
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(TARGETS kylin-space-cleaner-helper
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}
)
install(PROGRAMS bin/kylin-space-guard
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES packaging/kylin-space-guard.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)
install(FILES packaging/kylin-space-guard.desktop
    RENAME kylin-space-cleaner.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)
install(FILES assets/kylin-space-guard.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)
install(FILES assets/kylin-space-guard.svg
    RENAME kylin-space-cleaner.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
)
install(FILES assets/kylin-space-guard-256.png
    RENAME kylin-space-guard.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
)
install(FILES assets/kylin-space-guard-256.png
    RENAME kylin-space-cleaner.png
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps
)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "kylin-space-guard")
set(CPACK_PACKAGE_VENDOR "Kylin Space Guard")
set(CPACK_PACKAGE_CONTACT "Swordup-Z <swordup.zeng@gmail.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "KylinOS V11 desktop space cleaner")
set(CPACK_PACKAGE_DESCRIPTION "KylinOS V11 desktop space cleaner for root usage visibility, Kaiming old container cleanup planning, KARE/ostree visibility, and autostart entry optimization.")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5core5a, libqt5widgets5, libqt5charts5")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
    "${CMAKE_CURRENT_SOURCE_DIR}/packaging/postinst;${CMAKE_CURRENT_SOURCE_DIR}/packaging/postrm"
)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
include(CPack)
