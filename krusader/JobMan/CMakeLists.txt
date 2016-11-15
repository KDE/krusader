include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(JobMan_SRCS
    jobman.cpp)

add_library(JobMan STATIC ${JobMan_SRCS})

target_link_libraries(JobMan
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOWidgets
    KF5::WidgetsAddons
)
