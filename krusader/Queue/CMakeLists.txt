


########### next target ###############

set( Queue_SRCS
	queue.cpp
	queue_mgr.cpp
	queuewidget.cpp
	queuedialog.cpp )

kde4_add_library(Queue STATIC ${Queue_SRCS} )

########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libQueue.a
#
#INCLUDES = $(all_includes)
#
#libQueue_a_METASOURCES = AUTO
#
#libQueue_a_SOURCES = \
#	queue.cpp \
#	queue_mgr.cpp \
#	queuewidget.cpp
