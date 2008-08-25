
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set( Dialogs_SRCS
	krsqueezedtextlabel.cpp
	packgui.cpp
	packguibase.cpp
	newftpgui.cpp
	krspwidgets.cpp
	krspecialwidgets.cpp
	krpleasewait.cpp
	krmaskchoice.cpp
	krdialogs.cpp
	kurllistrequester.cpp
	popularurls.cpp
	checksumdlg.cpp
	percentalsplitter.cpp
	krkeydialog.cpp )

kde4_add_library(Dialogs STATIC ${Dialogs_SRCS} )


########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libDialogs.a
#
#INCLUDES = $(all_includes)
#
#libDialogs_a_METASOURCES = AUTO
#
#libDialogs_a_SOURCES = \
#	krsqueezedtextlabel.cpp \
#	krprogress.cpp \
#	packgui.cpp \
#	packguibase.cpp \
#	newftpgui.cpp \
#	krspwidgets.cpp \
#	krspecialwidgets.cpp \
#	krpleasewait.cpp \
#	krmaskchoice.cpp \
#	krdialogs.cpp \
#	kurllistrequester.cpp \
#	popularurls.cpp \
#	checksumdlg.cpp \
#	percentalsplitter.cpp \
#	krkeydialog.cpp
