
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )

########### next target ###############

set( Konfigurator_SRCS
	kgcolors.cpp
	kgdependencies.cpp
	konfiguratorpage.cpp
	konfiguratoritems.cpp
	konfigurator.cpp
	kgstartup.cpp
	kglookfeel.cpp
	kggeneral.cpp
	kgarchives.cpp
	kgadvanced.cpp
	kguseractions.cpp
	kgprotocols.cpp
	krresulttable.cpp
	krresulttabledialog.cpp
	searchobject.cpp )

kde4_add_library(Konfigurator ${Konfigurator_SRCS} )

########### install files ###############




#original Makefile.am contents follow:

#noinst_LIBRARIES = libKonfigurator.a
#
#INCLUDES = -I$(top_builddir)/krusader/GUI $(all_includes)
#
#libKonfigurator_a_METASOURCES = AUTO
#
#libKonfigurator_a_SOURCES = \
#	kgcolors.cpp \
#	kgdependencies.cpp \
#	konfiguratorpage.cpp \
#	konfiguratoritems.cpp \
#	konfigurator.cpp \
#	kgwelcome.cpp \
#	kgstartup.cpp \
#	kglookfeel.cpp \
#	kggeneral.cpp \
#	kgarchives.cpp \
#	kgadvanced.cpp \
#	kguseractions.cpp \
#	kgprotocols.cpp \
#	krresulttable.cpp \
#	krresulttabledialog.cpp \
#	searchobject.cpp
