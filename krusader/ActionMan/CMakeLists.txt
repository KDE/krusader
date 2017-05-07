set(ActionMan_SRCS
    actionman.cpp
    actionproperty.cpp
    addplaceholderpopup.cpp
    useractionlistview.cpp
    useractionpage.cpp)

ki18n_wrap_ui (
    ActionMan_SRCS
    actionproperty.ui
)

add_library(ActionMan STATIC ${ActionMan_SRCS})

target_link_libraries(ActionMan
    KF5::ConfigCore
    KF5::I18n
    KF5::IconThemes
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::Notifications
    KF5::Parts
    KF5::WidgetsAddons
)
