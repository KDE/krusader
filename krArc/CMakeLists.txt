include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(kio_krarc_PART_SRCS
    krarc.cpp
    krarcbasemanager.cpp
    krlinecountingprocess.cpp
    ../krusader/krdebuglogger.cpp
)

add_library(kio_krarc MODULE ${kio_krarc_PART_SRCS})

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

install(TARGETS kio_krarc DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES krarc.protocol DESTINATION  ${SERVICES_INSTALL_DIR})
