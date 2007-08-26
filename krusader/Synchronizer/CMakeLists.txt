
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set( Synchronizer_SRCS
	synchronizedialog.cpp
	synchronizer.cpp 
	synchronizergui.cpp 
	feedtolistboxdialog.cpp 
	synchronizertask.cpp 
	synchronizerdirlist.cpp )

kde4_add_library(Synchronizer ${Synchronizer_SRCS} )


########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libSynchronizer.a
#
#INCLUDES = $(all_includes)
#
#libSynchronizer_a_METASOURCES = AUTO
#
#libSynchronizer_a_SOURCES = \
#	synchronizedialog.cpp \
#	synchronizer.cpp \
#	synchronizergui.cpp \
#	feedtolistboxdialog.cpp \
#	synchronizertask.cpp \
#	synchronizerdirlist.cpp
