project(krusader)

INCLUDE(CPack)

set(VERSION "2.2.0-beta1" )
set(RELEASE_NAME "DeKade")

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories( ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )

set(CMAKE_VERBOSE_MAKEFILE OFF)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

#include(ConfigureChecks.cmake)

add_subdirectory(krusader)
add_subdirectory(pics)
add_subdirectory(doc-extras)
add_subdirectory(krArc)
add_subdirectory(iso)
#add_subdirectory(krfiledialog) - not used
#add_subdirectory(virt) - not used
#add_subdirectory(tar) - not used

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(TARDIR)")
