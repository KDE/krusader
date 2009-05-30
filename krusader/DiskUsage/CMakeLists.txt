include(radialMap/CMakeLists.txt)
include(filelightParts/CMakeLists.txt)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/filelightParts)

set(DiskUsage_SRCS
    diskusagegui.cpp
    diskusage.cpp
    dulistview.cpp
    dulines.cpp
    dufilelight.cpp )

kde4_add_library(DiskUsage STATIC ${DiskUsage_SRCS} ${radialMap_SRCS} ${filelightParts_SRCS})
