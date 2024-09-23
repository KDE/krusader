set(MountMan_SRCS
    kmountmangui.cpp
    kmountman.cpp)

add_library(MountMan STATIC ${MountMan_SRCS})

target_link_libraries(MountMan
    Dialogs
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOCore
    KF6::WidgetsAddons
    KF6::Solid
)
