set(UserAction_SRCS
    kraction.cpp
    expander.cpp
    useractionpopupmenu.cpp
    kractionbase.cpp
    useraction.cpp)

add_library(UserAction STATIC ${UserAction_SRCS})

target_link_libraries(UserAction
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::IconThemes
    KF5::Notifications
    KF5::Parts
    KF5::WidgetsAddons
    KF5::XmlGui
    KF5::KIOFileWidgets
    Qt5::Xml
)
