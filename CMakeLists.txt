
find_package(KDE4 REQUIRED)

include(CPack)

SET(CPACK_PACKAGE_VERSION_MAJOR 2)
SET(CPACK_PACKAGE_VERSION_MINOR 0)
SET(CPACK_PACKAGE_VERSION_PATCH 0)
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Krusader is a twin panel file manager for KDE.")
SET(CPACK_PACKAGE_VENDOR "Krusader Team : http://www.krusader.org")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "krusader_kde4_${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "krusader-kde4-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
   

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions( -DVERSION='"2.0.0-beta1"' )
add_definitions( -DRELEASE_NAME='"Phoenix` Egg"' )

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
#add_subdirectory( virt ) - not used

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

########### experimental cpack support ###########

# FIXME: still creates empty packages
# FIXME: CPACK_GENERATOR/CPACK_SOURCE_GENERATOR aren't honoured when "make package" is used

INCLUDE(InstallRequiredSystemLibraries)
#SET(CPACK_SET_DESTDIR ON)
SET(CPACK_GENERATOR "TBZ2")
#SET(CPACK_INSTALL_CMAKE_PROJECTS "/usr/local/src/krusader_kde4;krusader;ALL;/")
#SET(CPACK_PACKAGING_INSTALL_PREFIX "/")
#SET(CPACK_NSIS_DISPLAY_NAME "Krusader 2.0.0-beta1")
SET(CPACK_OUTPUT_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/CPackConfig.cmake")
SET(CPACK_PACKAGE_CONTACT "krusader-devel <krusader-devel@googlegroups.com>")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Krusader is an advanced twin panel (commander style) file manager for KDE and other desktops in the *nix world")
SET(CPACK_PACKAGE_EXECUTABLES "krusader")
SET(CPACK_PACKAGE_FILE_NAME "krusader-2.0.0-beta1")
#SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Krusader 2.0.0-beta1")
#SET(CPACK_PACKAGE_INSTALL_REGISTRY_KEY "Krusader 2.0.0-beta1")
SET(CPACK_PACKAGE_NAME "krusader")
SET(CPACK_PACKAGE_VENDOR "Krusader Krew")
SET(CPACK_PACKAGE_VERSION "2.0.0-beta1 \"Phoenix Egg\"")
SET(CPACK_PACKAGE_VERSION_MAJOR "2")
SET(CPACK_PACKAGE_VERSION_MINOR "0")
SET(CPACK_PACKAGE_VERSION_PATCH "0-beta1")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
SET(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README")
SET(CPACK_RESOURCE_FILE_WELCOME "/usr/share/cmake/Templates/CPack.GenericWelcome.txt")
SET(CPACK_SOURCE_GENERATOR "TBZ2")
SET(CPACK_SOURCE_OUTPUT_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/CPackSourceConfig.cmake")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "krusader-2.0.0-beta1")
#SET(CPACK_SOURCE_IGNORE_FILES "")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Krusader 2.0.0-beta1")
INCLUDE(CPack)
