
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set( MountMan_SRCS
	kmountmangui.cpp
	kmountman.cpp
)

kde4_add_library(MountMan ${MountMan_SRCS} )


########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libMountMan.a
#
#INCLUDES = $(all_includes)
#
#libMountMan_a_METASOURCES = AUTO
#
#libMountMan_a_SOURCES = \
#	kmountmangui.cpp \
#	kmountman.cpp \
#	kdiskfreesp.cpp
