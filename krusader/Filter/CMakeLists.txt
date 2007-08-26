
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set( Filter_SRCS
	advancedfilter.cpp
	generalfilter.cpp
	filtertabs.cpp
	filterdialog.cpp )

kde4_add_library(Filter ${Filter_SRCS} )


########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libFilter.a
#
#INCLUDES = $(all_includes)
#
#libFilter_a_METASOURCES = AUTO
#
#libFilter_a_SOURCES = \
#	advancedfilter.cpp \
#	generalfilter.cpp \
#	filtertabs.cpp \
#	filterdialog.cpp
