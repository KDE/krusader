include(FindZLIB)

include(libisofs/CMakeLists.txt)

include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(kio_iso_PART_SRCS
    kisodirectory.cpp
    kisofile.cpp
    qfilehack.cpp
    kiso.cpp
    iso.cpp
    ../krusader/krdebuglogger.cpp
)

add_library(kio_iso MODULE ${kio_iso_PART_SRCS} ${libisofs_SRCS})

#this is a library so it needs to enforce it's translation domain, not use the application's domain.
add_definitions(-DTRANSLATION_DOMAIN="krusader")

target_link_libraries(kio_iso
    KF5::Archive
    KF5::ConfigCore
    KF5::KIOCore
    ${ZLIB_LIBRARY}
)

install(TARGETS kio_iso DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES iso.protocol DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kio_isorc DESTINATION ${CONFIG_INSTALL_DIR})
