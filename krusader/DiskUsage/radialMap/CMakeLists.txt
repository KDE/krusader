
include_directories( ${CMAKE_SOURCE_DIR}/krusader/DiskUsage/filelightParts ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############


########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libradialmap.a
#
#INCLUDES = -I$(top_srcdir)/krusader/DiskUsage/filelightParts \
#	 $(all_includes)
#
#METASOURCES = AUTO
#
#libradialmap_a_SOURCES = \
#	widget.cpp \
#	builder.cpp \
#	map.cpp \
#	widgetEvents.cpp \
#	labels.cpp \
#	segmentTip.cpp
