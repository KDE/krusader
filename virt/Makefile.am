## Makefile.am of kdebase/kioslave/tar

INCLUDES = $(all_includes)
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)
METASOURCES = AUTO

kde_module_LTLIBRARIES = kio_virt.la

kio_virt_la_SOURCES = virt.cc
kio_virt_la_LIBADD = $(LIB_KSYCOCA)
kio_virt_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

noinst_HEADERS = virt.h

kdelnk_DATA = virt.protocol
kdelnkdir = $(kde_servicesdir)

