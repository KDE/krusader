
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set( UserAction_SRCS
	kraction.cpp
	expander.cpp
	useractionpopupmenu.cpp
	kractionbase.cpp 
	useraction.cpp )

kde4_add_library(UserAction STATIC ${UserAction_SRCS} )


########### install files ###############




#original Makefile.am contents follow:

#if include_libkjsembed
#AM_CPPFLAGS = -D__KJSEMBED__
#endif
#
#noinst_LIBRARIES = libUserAction.a
#
#INCLUDES = $(all_includes)
#
#libUserAction_a_METASOURCES = AUTO
#
#libUserAction_a_SOURCES = useraction.cpp \
#	kraction.cpp \
#	expander.cpp \
#	useractionpopupmenu.cpp \
#	kractionbase.cpp
#
#noinst_HEADERS = tstring.h
