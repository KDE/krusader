set(JobMan_SRCS
    jobman.cpp
    krjob.cpp)

add_library(JobMan STATIC ${JobMan_SRCS})

target_link_libraries(JobMan
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOWidgets
    KF5::WidgetsAddons
)
