
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set( GUI_SRCS
	dirhistoryqueue.cpp 
	dirhistorybutton.cpp 
	krusaderstatus.cpp 
	kfnkeys.cpp 
	kcmdline.cpp 
	syncbrowsebutton.cpp 
	profilemanager.cpp 
	krremoteencodingmenu.cpp 
	krtreewidget.cpp
	krstyleproxy.cpp
	krlistwidget.cpp
	mediabutton.cpp 
	kcmdmodebutton.cpp )

kde4_add_library(GUI ${GUI_SRCS} )

########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libGUI.a
#
#INCLUDES = $(all_includes)
#
#libGUI_a_METASOURCES = AUTO
#
#libGUI_a_SOURCES = \
#	dirhistoryqueue.cpp \
#	dirhistorybutton.cpp \
#	krusaderstatus.cpp \
#	kfnkeys.cpp \
#	kcmdline.cpp \
#	syncbrowsebutton.cpp \
#	profilemanager.cpp \
#	krremoteencodingmenu.cpp \
#	mediabutton.cpp \
#	kcmdmodebutton.cpp
#
