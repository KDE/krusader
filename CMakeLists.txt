project(krusader)

INCLUDE(CPack)

set(VERSION "2.4.0-beta3" )
set(RELEASE_NAME "Single Step")

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories( ${KDE4_INCLUDES} ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )

set(CMAKE_VERBOSE_MAKEFILE OFF)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

add_definitions( "-DKRARC_ENABLED" )

if(ENABLE_SYNCHRONIZER)
    add_definitions( "-DENABLE_SYNCHRONIZER" )
endif(ENABLE_SYNCHRONIZER)

find_package(ACL)
if(ACL_FOUND)
    add_definitions(-DHAVE_POSIX_ACL)
endif(ACL_FOUND)

#include(ConfigureChecks.cmake)

add_subdirectory(krusader)
add_subdirectory(pics)
add_subdirectory(doc-extras)
add_subdirectory(krArc)
add_subdirectory(iso)
add_subdirectory(doc)
#add_subdirectory(krfiledialog) - not used
#add_subdirectory(virt) - not used
#add_subdirectory(tar) - not used

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(TARDIR)")
