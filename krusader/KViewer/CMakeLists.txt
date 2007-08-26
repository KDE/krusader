
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set( KViewer_SRCS
	krviewer.cpp
	kimagefilepreview.cpp
	panelviewer.cpp
	diskusageviewer.cpp )

kde4_add_library(KViewer ${KViewer_SRCS} )


########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libKViewer.a
#
#INCLUDES = $(all_includes)
#
#libKViewer_a_METASOURCES = AUTO
#
#libKViewer_a_SOURCES = \
#	krviewer.cpp \
#	kimagefilepreview.cpp \
#	panelviewer.cpp \
#	diskusageviewer.cpp
