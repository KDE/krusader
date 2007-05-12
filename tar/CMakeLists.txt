
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(kio_tar_PART_SRCS tar.cc )

kde4_automoc(${kio_tar_PART_SRCS})

kde4_add_plugin(kio_tar ${kio_tar_PART_SRCS})

target_link_libraries(kio_tar  ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} )

install(TARGETS kio_tar DESTINATION ${PLUGIN_INSTALL_DIR} )


########### next target ###############

set(ktartest_SRCS ktartest.cpp )

kde4_automoc(${ktartest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(ktartest ${ktartest_SRCS})

target_link_libraries(ktartest  ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############

install( FILES  tar.protocol ar.protocol zip.protocol DESTINATION ${SERVICES_INSTALL_DIR} )



#original Makefile.am contents follow:

### Makefile.am of kdebase/kioslave/tar
#
#INCLUDES = $(all_includes)
#AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#METASOURCES = AUTO
#
#kde_module_LTLIBRARIES = kio_tar.la
#
#kio_tar_la_SOURCES = tar.cc
#kio_tar_la_LIBADD = $(LIB_KSYCOCA)
#kio_tar_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#
#check_PROGRAMS = ktartest
#
#ktartest_SOURCES = ktartest.cpp
#ktartest_LDADD = $(LIB_KSYCOCA)
#
#noinst_HEADERS = tar.h
#
#kdelnk_DATA = tar.protocol ar.protocol zip.protocol
#kdelnkdir = $(kde_servicesdir)
#
