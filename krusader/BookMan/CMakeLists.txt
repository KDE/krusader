
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############


set( BookMan_SRCS
	krbookmark.cpp
	krbookmarkbutton.cpp
	krbookmarkhandler.cpp
	kraddbookmarkdlg.cpp )

kde4_add_library(BookMan ${BookMan_SRCS} )


########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libBookMan.a
#
#INCLUDES = $(all_includes)
#
#libBookMan_a_METASOURCES = AUTO
#
#libBookMan_a_SOURCES = \
#	krbookmark.cpp \
#	krbookmarkbutton.cpp \
#	krbookmarkhandler.cpp \
#	kraddbookmarkdlg.cpp
