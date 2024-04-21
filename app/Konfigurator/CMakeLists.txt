set(Konfigurator_SRCS
    kgcolors.cpp
    kgdependencies.cpp
    konfiguratorpage.cpp
    konfiguratoritems.cpp
    konfigurator.cpp
    kgstartup.cpp
    kgpanel.cpp
    kggeneral.cpp
    kgarchives.cpp
    kgadvanced.cpp
    kguseractions.cpp
    kgprotocols.cpp
    krresulttable.cpp
    krresulttabledialog.cpp
    searchobject.cpp)

add_library(Konfigurator STATIC ${Konfigurator_SRCS})

target_link_libraries(Konfigurator
    KF5::Archive
    KF5::I18n
    KF5::IconThemes
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::ConfigCore
    KF5::ConfigWidgets
    KF5::WidgetsAddons
    KF5::GuiAddons
    Qt5::Xml
)
