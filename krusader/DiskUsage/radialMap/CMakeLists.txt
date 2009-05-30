


########### next target ###############

set( radialMap_SRCS
	radialMap/widget.cpp
	radialMap/builder.cpp
	radialMap/map.cpp
	radialMap/widgetEvents.cpp
	radialMap/labels.cpp
	radialMap/segmentTip.cpp )


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
