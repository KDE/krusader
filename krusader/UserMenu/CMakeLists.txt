
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set( UserMenu_SRCS
	usermenu.cpp )

kde4_add_library(UserMenu ${UserMenu_SRCS} )

########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libUserMenu.a
#
#INCLUDES = $(all_includes)
#
#libUserMenu_a_METASOURCES = AUTO
#
#libUserMenu_a_SOURCES = usermenu.cpp
