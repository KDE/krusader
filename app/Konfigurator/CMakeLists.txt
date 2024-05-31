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
    KF6::Archive
    KF6::I18n
    KF6::IconThemes
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::ConfigCore
    KF6::ConfigWidgets
    KF6::WidgetsAddons
    KF6::GuiAddons
    Qt6::Xml
    Qt6::Core5Compat
)
