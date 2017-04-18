include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(Panel_SRCS
    viewactions.cpp
    listpanelactions.cpp
    dirhistoryqueue.cpp
    krsort.cpp
    krlayoutfactory.cpp
    krerrordisplay.cpp
    krpreviews.cpp
    krpreviewjob.cpp
    krcolorcache.cpp
    krcalcspacedialog.cpp
    krpopupmenu.cpp
    krpreviewpopup.cpp
    krview.cpp
    krviewproperties.cpp
    krviewitem.cpp
    panelfunc.cpp
    krpanel.cpp
    listpanel.cpp
    listpanelframe.cpp
    panelpopup.cpp
    krselectionmode.cpp
    listmodel.cpp
    krinterview.cpp
    krinterbriefview.cpp
    krinterdetailedview.cpp
    krviewfactory.cpp
    krviewitemdelegate.cpp
    krmousehandler.cpp
    krsearchbar.cpp
    krfiletreeview.cpp)

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
