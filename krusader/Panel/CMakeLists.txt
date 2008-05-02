add_definitions( -DQT3_SUPPORT=1 -DKDE3_SUPPORT=1)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############


set( Panel_SRCS
	krcolorcache.cpp 
	krcalcspacedialog.cpp 
	krpopupmenu.cpp 
	krpreviewpopup.cpp 
	krview.cpp 
	krviewitem.cpp 
	krdetailedviewitem.cpp 
	krdetailedview.cpp 
	panelfunc.cpp 
	listpanel.cpp 
	panelpopup.cpp 
	krselectionmode.cpp 
	krbriefview.cpp 
	krbriefviewitem.cpp
        krviewfactory.cpp )

kde4_add_library(Panel ${Panel_SRCS} )


########### install files ###############




#original Makefile.am contents follow:

#if include_libkonq
#AM_CPPFLAGS = -D__LIBKONQ__
#endif
#
#
#noinst_LIBRARIES = libPanel.a
#
#INCLUDES = $(all_includes)
#
#libPanel_a_METASOURCES = AUTO
#
#libPanel_a_SOURCES = \
#	krcolorcache.cpp \
#	krcalcspacedialog.cpp \
#	krpopupmenu.cpp \
#	krpreviewpopup.cpp \
#	krview.cpp \
#	krviewitem.cpp \
#	krdetailedviewitem.cpp \
#	krdetailedview.cpp \
#	panelfunc.cpp \
#	listpanel.cpp \
#	panelpopup.cpp \
#	krdrag.cpp \
#	krselectionmode.cpp \
#	krbriefview.cpp \
#	krbriefviewitem.cpp
#
#
