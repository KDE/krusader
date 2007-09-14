
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

### IMPORTANT: actionpropertybase.ui is not used anymore, because of qt3 to qt4 issues
###            instead, actionpropertybase.h was generated and put into svn :-(

set( ActionMan_SRCS
	actionman.cpp
#	actionpropertybase.ui
	actionproperty.cpp
	addplaceholderpopup.cpp
	useractionlistview.cpp
	useractionpage.cpp )

kde4_add_library(ActionMan ${ActionMan_SRCS} )

########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libActionMan.a
#
#INCLUDES = $(all_includes)
#
#libActionMan_a_METASOURCES = AUTO
#
#libActionMan_a_SOURCES = \
#	actionman.cpp \
#	actionpropertybase.ui \
#	actionproperty.cpp \
#	addplaceholderpopup.cpp \
#	useractionlistview.cpp \
#	useractionpage.cpp
#noinst_HEADERS = useractionpage.h
