set(KViewer_SRCS
    krviewer.cpp
    panelviewer.cpp
    diskusageviewer.cpp
    lister.cpp
    viewertabbar.cpp)

add_library(KViewer STATIC ${KViewer_SRCS})

target_link_libraries(KViewer
    Qt5::PrintSupport
    KF5::Archive
    KF5::ConfigCore
    KF5::ConfigWidgets
    KF5::CoreAddons
    KF5::IconThemes
    KF5::KIOCore
    KF5::KIOFileWidgets
    KF5::WindowSystem
    KF5::Parts
    KF5::WidgetsAddons
    KF5::XmlGui
)
