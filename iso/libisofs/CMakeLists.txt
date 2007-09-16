
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )

set(libisofs_SRCS 
   libisofs/isofs.c )

########### next target ###############


########### install files ###############




#original Makefile.am contents follow:

#noinst_LTLIBRARIES = libisofs.la
#
#
#INCLUDES = $(all_includes)
#
#
##LDFLAGS =
#
#libisofs_la_METASOURCES=AUTO
#
#libisofs_la_SOURCES = isofs.c
##libisofs_la_LIBADD = $(LIB_KIO)
#
##libisofs_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
#
#
#
#noinst_HEADERS = bswap.h el_torito.h iso_fs.h isofs.h rock.h
