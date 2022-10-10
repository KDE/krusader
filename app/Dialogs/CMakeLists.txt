set(Dialogs_SRCS
    krsqueezedtextlabel.cpp
    packgui.cpp
    packguibase.cpp
    newftpgui.cpp
    krspwidgets.cpp
    krspecialwidgets.cpp
    krpleasewait.cpp
    krmaskchoice.cpp
    krdialogs.cpp
    kurllistrequester.cpp
    popularurls.cpp
    checksumdlg.cpp
    percentalsplitter.cpp
)

add_library(Dialogs STATIC ${Dialogs_SRCS})

target_link_libraries(Dialogs
    KF5::Completion
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::I18n
    KF5::IconThemes
    KF5::ItemViews
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::Notifications
    KF5::Parts
    KF5::WidgetsAddons
)
