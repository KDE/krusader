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
    sidebar.cpp
    viewactions.cpp
)

add_library(Panel STATIC ${Panel_SRCS})

target_link_libraries(Panel
    Dialogs
    GUI
    KViewer
    KF6::Archive
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::IconThemes
    KF6::KIOFileWidgets
    KF6::KIOWidgets
    KF6::Service
    KF6::WidgetsAddons
    KF6::XmlGui
    Qt6::Xml
    Qt6::Core5Compat
)
