set(kio_krarc_PART_SRCS
    krarc.cpp
    krarcbasemanager.cpp
    krlinecountingprocess.cpp
    ../../app/krdebuglogger.cpp
)

kcoreaddons_add_plugin(kio_krarc SOURCES ${kio_krarc_PART_SRCS} INSTALL_NAMESPACE "kf5/kio")

#this is a library so it needs to enforce it's translation domain, not use the application's domain.
add_definitions(-DTRANSLATION_DOMAIN="krusader")

# TODO porting : new variables needed?
target_link_libraries(kio_krarc
    Qt5::Gui
    KF5::Archive
    KF5::ConfigCore
    KF5::CoreAddons
    KF5::I18n
    KF5::KIOCore
)
