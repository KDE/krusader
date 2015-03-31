include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(GUI_SRCS
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

add_library(GUI STATIC ${GUI_SRCS})

target_link_libraries(GUI KF5::ConfigCore KF5::KDELibs4Support)
