
include( radialMap/CMakeLists.txt )
include( filelightParts/CMakeLists.txt )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/filelightParts)


########### next target ###############

set( DiskUsage_SRCS
	diskusagegui.cpp
	diskusage.cpp
	dulistview.cpp
	dulines.cpp
	dufilelight.cpp )

kde4_add_library(DiskUsage STATIC ${DiskUsage_SRCS} ${radialMap_SRCS} ${filelightParts_SRCS} )


########### install files ###############




#original Makefile.am contents follow:

#SUBDIRS = \
#	radialMap \
#	filelightParts
#
#
#noinst_LIBRARIES = libDiskUsage.a
#
#INCLUDES = $(all_includes)
#
#libDiskUsage_a_METASOURCES = AUTO
#
#libDiskUsage_a_SOURCES = \
#	diskusagegui.cpp \
#	diskusage.cpp \
#	dulistview.cpp \
#	dulines.cpp \
#	dufilelight.cpp
