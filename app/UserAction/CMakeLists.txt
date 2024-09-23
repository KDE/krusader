set(UserAction_SRCS
    kraction.cpp
    expander.cpp
    useractionpopupmenu.cpp
    kractionbase.cpp
    useraction.cpp)

add_library(UserAction STATIC ${UserAction_SRCS})

target_link_libraries(UserAction
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::IconThemes
    KF6::Notifications
    KF6::Parts
    KF6::WidgetsAddons
    KF6::XmlGui
    KF6::TextWidgets
    KF6::KIOFileWidgets
    KF6::StatusNotifierItem
    Qt6::Xml
    Qt6::Core5Compat
)
