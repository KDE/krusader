


########### next target ###############

set( MountMan_SRCS
	kmountmangui.cpp
	kmountman.cpp
)

kde4_add_library(MountMan STATIC ${MountMan_SRCS} )

target_link_libraries(MountMan Dialogs )

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
