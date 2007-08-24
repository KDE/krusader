
find_package(KDE4 REQUIRED)

set(CMAKE_VERBOSE_MAKEFILE ON)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS} -DQT3_SUPPORT=1)
add_definitions( -DVERSION='"2.0-alpha"' )
add_definitions( -DRELEASE_NAME='"Initial version."' )

link_directories(${KDE4_LIB_DIR})

# include(ConvenienceLibs.cmake)

# include(ManualStuff.cmake)

# include(ConfigureChecks.cmake)

add_subdirectory( krusader )
add_subdirectory( po )
add_subdirectory( pics )
add_subdirectory( doc )
add_subdirectory( krArc )
add_subdirectory( iso )
add_subdirectory( virt )

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(TARDIR)")
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


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
