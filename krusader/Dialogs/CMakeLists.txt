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
    krkeydialog.cpp)

kde4_add_library(Dialogs STATIC ${Dialogs_SRCS})
