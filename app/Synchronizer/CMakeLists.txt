set(Synchronizer_SRCS
    synchronizedialog.cpp
    synchronizer.cpp
    synchronizergui.cpp
    feedtolistboxdialog.cpp
    synchronizertask.cpp
    synchronizerdirlist.cpp)

add_library(Synchronizer STATIC ${Synchronizer_SRCS})

target_link_libraries(Synchronizer
    Dialogs
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOWidgets
    KF6::WidgetsAddons
    KF6::GuiAddons
    KF6::KIOFileWidgets
    KF6::Parts
    Qt6::Core5Compat
)
