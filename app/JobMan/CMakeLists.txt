set(JobMan_SRCS
    jobman.cpp
    krjob.cpp)

add_library(JobMan STATIC ${JobMan_SRCS})

target_link_libraries(JobMan
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOWidgets
    KF6::WidgetsAddons
)
