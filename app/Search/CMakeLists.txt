set(Search_SRCS
    krsearchmod.cpp
    krsearchdialog.cpp)

add_library(Search STATIC ${Search_SRCS})

target_link_libraries(Search
    Dialogs
    KF6::I18n
    KF6::KIOCore
    KF6::WidgetsAddons
    Qt6::Core5Compat
)
