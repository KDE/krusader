set(PanelView_SRCS
    krinterbriefview.cpp
    krinterdetailedview.cpp
    krinterview.cpp
    krmousehandler.cpp
    krselectionmode.cpp
    krsort.cpp
    krview.cpp
    krviewfactory.cpp
    krviewitem.cpp
    krviewitemdelegate.cpp
    krviewproperties.cpp
    listmodel.cpp
)

add_library(PanelView STATIC ${PanelView_SRCS})

target_link_libraries(PanelView
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
)
