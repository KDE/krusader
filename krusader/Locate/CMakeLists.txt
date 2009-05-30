


########### next target ###############

set( Locate_SRCS
	locate.cpp )

kde4_add_library(Locate STATIC ${Locate_SRCS} )


########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libLocate.a
#
#INCLUDES = $(all_includes)
#
#libLocate_a_METASOURCES = AUTO
#
#libLocate_a_SOURCES = locate.cpp 
