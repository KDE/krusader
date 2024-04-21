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
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOWidgets
    KF5::WidgetsAddons
    KF5::GuiAddons
    KF5::KIOFileWidgets
)
