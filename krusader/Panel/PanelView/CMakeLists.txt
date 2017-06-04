include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

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
