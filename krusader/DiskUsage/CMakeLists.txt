
add_subdirectory( radialMap )
add_subdirectory( filelightParts )

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############


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
