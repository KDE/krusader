include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

set(kio_krarc_PART_SRCS krarc.cpp)

kde4_add_plugin(kio_krarc ${kio_krarc_PART_SRCS})

target_link_libraries(kio_krarc  ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS})

install(TARGETS kio_krarc DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES krarc.protocol DESTINATION  ${SERVICES_INSTALL_DIR} )
