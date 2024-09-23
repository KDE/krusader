include(FindZLIB)

include(libisofs/CMakeLists.txt)

set(kio_iso_PART_SRCS
    kisodirectory.cpp
    kisofile.cpp
    qfilehack.cpp
    kiso.cpp
    iso.cpp
    ../../app/krdebuglogger.cpp
)

kcoreaddons_add_plugin(kio_iso SOURCES ${kio_iso_PART_SRCS} ${libisofs_SRCS} INSTALL_NAMESPACE "kf6/kio")

#this is a library so it needs to enforce it's translation domain, not use the application's domain.
add_definitions(-DTRANSLATION_DOMAIN="krusader")

target_link_libraries(kio_iso
    KF6::Archive
    KF6::Completion
    KF6::ConfigCore
    KF6::KIOCore
    ${ZLIB_LIBRARY}
)

install(FILES kio_isorc DESTINATION ${KDE_INSTALL_CONFDIR})
