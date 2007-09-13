
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set( filelightParts_SRCS
	filelightParts/Config.cpp
	filelightParts/fileTree.cpp )


########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libfilelightparts.a
#
#INCLUDES = $(all_includes)
#
#METASOURCES = AUTO
#
#libfilelightparts_a_SOURCES = \
#	Config.cpp \
#	fileTree.cpp
