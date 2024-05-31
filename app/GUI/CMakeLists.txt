set(GUI_SRCS
    dirhistorybutton.cpp
    krusaderstatus.cpp
    kfnkeys.cpp
    kcmdline.cpp
    profilemanager.cpp
    krhistorycombobox.cpp
    krremoteencodingmenu.cpp
    krtreewidget.cpp
    krstyleproxy.cpp
    krlistwidget.cpp
    mediabutton.cpp
    kcmdmodebutton.cpp
    recentlyclosedtabsmenu.cpp
    terminaldock.cpp)

add_library(GUI STATIC ${GUI_SRCS})

target_link_libraries(GUI
    KF6::Archive
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::Codecs
    KF6::I18n
    KF6::IconThemes
    KF6::Parts
    KF6::Service
    KF6::Solid
    KF6::WidgetsAddons
    KF6::KIOFileWidgets
    Qt6::Widgets
    Qt6::Core5Compat
)
