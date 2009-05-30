


########### next target ###############

set( Search_SRCS
	krsearchmod.cpp
	krsearchdialog.cpp )

kde4_add_library(Search STATIC ${Search_SRCS} )

target_link_libraries(Search Dialogs )

########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libSearch.a
#
#INCLUDES = $(all_includes)
#
#libSearch_a_METASOURCES = AUTO
#
#libSearch_a_SOURCES = \
#	krsearchmod.cpp \
#	krsearchdialog.cpp
