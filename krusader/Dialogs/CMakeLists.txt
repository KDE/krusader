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

target_link_libraries(Dialogs KF5::Completion KF5::ConfigCore KF5::CoreAddons KF5::WidgetsAddons KF5::KDELibs4Support)
