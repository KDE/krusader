include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(KViewer_SRCS
    krviewer.cpp
    kimagefilepreview.cpp
    panelviewer.cpp
    diskusageviewer.cpp
    lister.cpp)

add_library(KViewer STATIC ${KViewer_SRCS})

target_link_libraries(KViewer
    KF5::ConfigCore
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::IconThemes
    KF5::KIOCore
    KF5::KIOFileWidgets
    KF5::Parts
    KF5::WidgetsAddons
    KF5::XmlGui
)
