set(Konfigurator_SRCS
    kgcolors.cpp
    kgdependencies.cpp
    konfiguratorpage.cpp
    konfiguratoritems.cpp
    konfigurator.cpp
    kgstartup.cpp
    kglookfeel.cpp
    kggeneral.cpp
    kgarchives.cpp
    kgadvanced.cpp
    kguseractions.cpp
    kgprotocols.cpp
    krresulttable.cpp
    krresulttabledialog.cpp
    searchobject.cpp)

kde4_add_library(Konfigurator STATIC ${Konfigurator_SRCS})
