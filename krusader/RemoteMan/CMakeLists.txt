
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set( RemoteMan_SRCS
	remoteman.cpp
	remotemanbase.cpp )

kde4_add_library(RemoteMan ${RemoteMan_SRCS} )


########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libRemoteMan.a
#
#INCLUDES = $(all_includes)
#
#libRemoteMan_a_METASOURCES = AUTO
#
#libRemoteMan_a_SOURCES = \
#	remoteman.cpp \
#	remotemanbase.cpp 
