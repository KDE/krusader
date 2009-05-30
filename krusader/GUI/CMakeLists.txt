set(GUI_SRCS
    dirhistoryqueue.cpp 
    dirhistorybutton.cpp 
    krusaderstatus.cpp 
    kfnkeys.cpp 
    kcmdline.cpp 
    syncbrowsebutton.cpp 
    profilemanager.cpp 
    krremoteencodingmenu.cpp 
    krtreewidget.cpp
    krstyleproxy.cpp
    krlistwidget.cpp
    mediabutton.cpp 
    kcmdmodebutton.cpp
    terminaldock.cpp)

kde4_add_library(GUI STATIC ${GUI_SRCS})
