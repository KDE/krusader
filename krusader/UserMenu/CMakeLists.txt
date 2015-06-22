include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(UserMenu_SRCS
    usermenu.cpp)

add_library(UserMenu STATIC ${UserMenu_SRCS})

target_link_libraries(UserMenu
    KF5::CoreAddons
    KF5::ConfigCore
    KF5::I18n
    KF5::KIOWidgets
    KF5::TextWidgets
    KF5::WidgetsAddons
)
