## Makefile.am of kdebase/kioslave/tar

INCLUDES = $(all_includes)
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)
METASOURCES = AUTO

kde_module_LTLIBRARIES = kio_tar.la

kio_tar_la_SOURCES = tar.cc
kio_tar_la_LIBADD = $(LIB_KSYCOCA)
kio_tar_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

check_PROGRAMS = ktartest

ktartest_SOURCES = ktartest.cpp
ktartest_LDADD = $(LIB_KSYCOCA)

noinst_HEADERS = tar.h

kdelnk_DATA = tar.protocol ar.protocol zip.protocol
kdelnkdir = $(kde_servicesdir)

