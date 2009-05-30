project(krusader)

INCLUDE(CPack)

set(VERSION "2.0.0" )
set(RELEASE_NAME "Mars Pathfinder")

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

set(CMAKE_VERBOSE_MAKEFILE OFF)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

#include(ConfigureChecks.cmake)

add_subdirectory(krusader)
add_subdirectory(pics)
add_subdirectory(doc)
add_subdirectory(krArc)
add_subdirectory(iso)
#add_subdirectory(virt) - not used

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(TARDIR)")
