set(Locate_SRCS
    locate.cpp)

add_library(Locate STATIC ${Locate_SRCS})

target_link_libraries(Locate
    KF6::Archive
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::Completion
    KF6::KIOCore
    KF6::I18n
    KF6::IconThemes
    KF6::Parts
    KF6::TextWidgets
    KF6::WidgetsAddons
    KF6::Service
    Qt6::Core5Compat
)
