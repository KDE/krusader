include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(Panel_SRCS
    kritemview.cpp
    quickfilter.cpp
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
    krviewitem.cpp
    panelfunc.cpp
    krpanel.cpp
    listpanel.cpp
    listpanelframe.cpp
    panelpopup.cpp
    krselectionmode.cpp
    krvfsmodel.cpp
    krinterview.cpp
    krinterbriefview.cpp
    krinterdetailedview.cpp
    krinterviewitemdelegate.cpp
    krviewfactory.cpp
    krmousehandler.cpp)

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

if(LIBKONQ_FOUND)
    target_link_libraries(Panel ${LIBKONQ_LIBRARY})
endif(LIBKONQ_FOUND)

