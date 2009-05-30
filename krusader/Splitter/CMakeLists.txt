


########### next target ###############

set( Splitter_SRCS
	crc32.cpp
	splittergui.cpp
	splitter.cpp
	combiner.cpp )

kde4_add_library(Splitter STATIC ${Splitter_SRCS} )


########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libSplitter.a
#
#INCLUDES = $(all_includes)
#
#libSplitter_a_METASOURCES = AUTO
#
#libSplitter_a_SOURCES = \
#	crc32.cpp \
#	splittergui.cpp \
#	splitter.cpp \
#	combiner.cpp 
