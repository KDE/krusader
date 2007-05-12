
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############




#original Makefile.am contents follow:

## This should not be nessesary because this subdir is only visited if KJSEmbed should be used...
##if include_libkjsembed
##AM_CPPFLAGS = -D__KJSEMBED__
##endif
#
#noinst_LIBRARIES = libKrJS.a
#
#INCLUDES = $(all_includes)
#
#libKrJS_a_METASOURCES = AUTO
#
#libKrJS_a_SOURCES = \
#	krjs.cpp
