set(kio_krarc_PART_SRCS
    krarc.cpp
    krarcbasemanager.cpp
    krlinecountingprocess.cpp
    ../../app/krdebuglogger.cpp
)

kcoreaddons_add_plugin(kio_krarc SOURCES ${kio_krarc_PART_SRCS} INSTALL_NAMESPACE "kf6/kio")

#this is a library so it needs to enforce it's translation domain, not use the application's domain.
add_definitions(-DTRANSLATION_DOMAIN="krusader")

# TODO porting : new variables needed?
target_link_libraries(kio_krarc
    Qt6::Gui
    Qt6::Core5Compat
    KF6::Archive
    KF6::Completion
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::KIOCore
)
