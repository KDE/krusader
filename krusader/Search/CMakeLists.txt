include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(Search_SRCS
    krsearchdialog.cpp)

add_library(Search STATIC ${Search_SRCS})

target_link_libraries(Search
    Dialogs
    KF5::I18n
    KF5::KIOCore
    KF5::WidgetsAddons
)
