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
    KF6::Archive
    KF6::Completion
    KF6::ConfigWidgets
    KF6::CoreAddons
    KF6::I18n
    KF6::IconThemes
    KF6::ItemViews
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::Notifications
    KF6::Parts
    KF6::WidgetsAddons
    KF6::StatusNotifierItem
)
