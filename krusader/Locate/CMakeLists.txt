include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(Locate_SRCS
    locate.cpp)

add_library(Locate STATIC ${Locate_SRCS})

target_link_libraries(Locate KF5::Completion KF5::WidgetsAddons KF5::IconThemes KF5::CoreAddons KF5::KDELibs4Support)