noinst_LTLIBRARIES = libisofs.la


INCLUDES = $(all_includes)


LDFLAGS =      

libisofs_la_METASOURCES=AUTO

libisofs_la_SOURCES = isofs.c 
libisofs_la_LIBADD = $(LIB_KIO) $(LIB_KDECORE) $(LIB_QT)

libisofs_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)



EXTRA_DIST = bswap.h el_torito.h iso_fs.h isofs.h rock.h isofs.c 
