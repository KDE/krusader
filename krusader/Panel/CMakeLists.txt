include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

add_subdirectory(PanelView)

set(Panel_SRCS
    dirhistoryqueue.cpp
    krcalcspacedialog.cpp
    krcolorcache.cpp
    krerrordisplay.cpp
    krfiletreeview.cpp
    krlayoutfactory.cpp
    krpanel.cpp
    krpreviewjob.cpp
    krpreviewpopup.cpp
    krpreviews.cpp
    krsearchbar.cpp
    listpanel.cpp
    listpanelactions.cpp
    listpanelframe.cpp
    panelcontextmenu.cpp
    panelfunc.cpp
    panelpopup.cpp
    viewactions.cpp
)

add_library(Panel STATIC ${Panel_SRCS})

target_link_libraries(Panel
    Dialogs
    GUI
    KViewer
    KF5::Archive
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::IconThemes
    KF5::KIOFileWidgets
    KF5::KIOWidgets
    KF5::Service
    KF5::WidgetsAddons
    KF5::XmlGui
)
