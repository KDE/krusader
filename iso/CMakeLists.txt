include(FindZLIB)

include(libisofs/CMakeLists.txt)

set(kio_iso_PART_SRCS
    kisodirectory.cpp
    kisofile.cpp
    qfilehack.cpp
    kiso.cpp
    iso.cpp
    ../krusader/krdebuglogger.cpp
)

kcoreaddons_add_plugin(kio_iso SOURCES ${kio_iso_PART_SRCS} ${libisofs_SRCS} INSTALL_NAMESPACE "kf5/kio")

#this is a library so it needs to enforce it's translation domain, not use the application's domain.
add_definitions(-DTRANSLATION_DOMAIN="krusader")

target_link_libraries(kio_iso
    KF5::Archive
    KF5::ConfigCore
    KF5::KIOCore
    ${ZLIB_LIBRARY}
)

install(FILES kio_isorc DESTINATION ${KDE_INSTALL_CONFDIR})
