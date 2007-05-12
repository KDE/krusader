
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(kio_virt_PART_SRCS virt.cc )

kde4_automoc(${kio_virt_PART_SRCS})

kde4_add_plugin(kio_virt ${kio_virt_PART_SRCS})

target_link_libraries(kio_virt  ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} )

install(TARGETS kio_virt DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install( FILES  virt.protocol DESTINATION ${SERVICES_INSTALL_DIR} )



#original Makefile.am contents follow:

### Makefile.am of kdebase/kioslave/tar
#
#INCLUDES = $(all_includes)
#AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = kio_virt.la
#
#kio_virt_la_SOURCES = virt.cc
#kio_virt_la_LIBADD = $(LIB_KSYCOCA)
#kio_virt_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#
#noinst_HEADERS = virt.h
#
#kdelnk_DATA = virt.protocol
#kdelnkdir = $(kde_servicesdir)
#
