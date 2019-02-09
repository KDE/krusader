include(radialMap/CMakeLists.txt)
include(filelightParts/CMakeLists.txt)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/filelightParts)

set(DiskUsage_SRCS
    diskusagegui.cpp
    diskusage.cpp
    dulistview.cpp
    dulines.cpp
    dufilelight.cpp )

add_library(DiskUsage STATIC ${DiskUsage_SRCS} ${radialMap_SRCS} ${filelightParts_SRCS})

target_link_libraries(DiskUsage
    KF5::ConfigWidgets
    KF5::I18n
    KF5::IconThemes
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::WidgetsAddons
)
