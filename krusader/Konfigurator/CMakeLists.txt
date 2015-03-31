include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

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

target_link_libraries(Konfigurator KF5::KIOWidgets KF5::ConfigWidgets KF5::KDELibs4Support KF5::Wallet)
