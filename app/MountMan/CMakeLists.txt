set(MountMan_SRCS
    kmountmangui.cpp
    kmountman.cpp)

add_library(MountMan STATIC ${MountMan_SRCS})

target_link_libraries(MountMan
    Dialogs
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
    KF5::WidgetsAddons
    KF5::Solid
)
