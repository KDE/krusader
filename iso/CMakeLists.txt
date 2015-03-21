include(FindZLIB)

include(libisofs/CMakeLists.txt)

include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(kio_iso_PART_SRCS 
    kisodirectory.cpp 
    kisofile.cpp 
    qfilehack.cpp 
    kiso.cpp 
    iso.cpp)

add_library(kio_iso ${kio_iso_PART_SRCS} ${libisofs_SRCS})

# TODO porting : new variables needed?
target_link_libraries(kio_iso
# 		      ${KDE4_KDECORE_LIBS}
# 		      ${KDE4_KIO_LIBS}
		      ${ZLIB_LIBRARY})

install(TARGETS kio_iso DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES iso.protocol DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES kio_isorc DESTINATION ${CONFIG_INSTALL_DIR})
