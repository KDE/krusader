
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set( filelightParts_SRCS
	Config.cpp
	fileTree.cpp )

kde4_add_library(filelightParts ${filelightParts_SRCS} )


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
