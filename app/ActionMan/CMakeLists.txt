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
    KF6::ConfigCore
    KF6::I18n
    KF6::IconThemes
    KF6::IconWidgets
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::Notifications
    KF6::Parts
    KF6::WidgetsAddons
    KF6::TextWidgets
    KF6::StatusNotifierItem
    Qt6::Xml
)
