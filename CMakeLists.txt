project(krusader)

INCLUDE(CPack)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

set(CMAKE_VERBOSE_MAKEFILE OFF)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
set(VERSION "2.0.0" )
set(RELEASE_NAME "Mars Pathfinder" )

# include(ConfigureChecks.cmake)

add_subdirectory( krusader )
add_subdirectory( pics )
add_subdirectory( doc )
add_subdirectory( krArc )
add_subdirectory( iso )
#add_subdirectory( virt ) - not used

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(TARDIR)")

########### install files ###############

#original Makefile.am contents follow:

######## kdevelop will overwrite this part!!! (begin)##########
#
## the condition for this is found in configure.in.in
#if with_tar
#TARDIR = tar
#endif
#
#SUBDIRS = krusader po pics doc krArc iso virt $(TARDIR)
#
#EXTRA_DIST = krusader.kdevprj admin AUTHORS COPYING ChangeLog INSTALL README TODO krusader.lsm krusader.spec 
#
######## kdevelop will overwrite this part!!! (end)############
## not a GNU package. You can remove this line, if 
## have all needed files, that a GNU package needs
#AUTOMAKE_OPTIONS = foreign
#
#MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 
#
#package-messages:
#	$(MAKE) -f admin/Makefile.common package-messages
#	$(MAKE) -C po merge
#
#dist-hook:
#	cd $(top_distdir) && perl admin/am_edit -padmin
#	cd $(top_distdir) && $(MAKE) -f admin/Makefile.common subdirs
#
#include admin/deps.am

