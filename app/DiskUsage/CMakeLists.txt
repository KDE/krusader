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
    KF6::Archive
    KF6::ConfigWidgets
    KF6::I18n
    KF6::IconThemes
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::WidgetsAddons
    KF6::ColorScheme
)
