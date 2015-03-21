set(kio_virt_PART_SRCS virt.cc)

add_library(kio_virt ${kio_virt_PART_SRCS})

# TODO porting : new variables needed?
target_link_libraries(kio_virt
# 		      ${KDE4_KDECORE_LIBS}
# 		      ${KDE4_KIO_LIBS}
		      )

install(TARGETS kio_virt DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES virt.protocol DESTINATION ${SERVICES_INSTALL_DIR})
