project(krusader)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

set(VERSION "2.6.1-beta" )
set(RELEASE_NAME "Stiff Challenges")

# Manages plugin loading. For more details it can be seen https://www.kde.org/announcements/kde-frameworks-5.5.0.php
# https://github.com/KDE/partitionmanager/blob/master/CMakeLists.txt http://commits.kde.org/kimtoy/1e62777affb0e072f3462e80bc271f3cba661104
set(KDE_INSTALL_USE_QT_SYS_PATHS ON CACHE BOOL "Install mkspecs files, Plugins and Imports to the Qt 5 install dir" FORCE)

find_package(ECM 1.7.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(ECMOptionalAddSubdirectory)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateHeaders)
include(GenerateExportHeader)

include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(WriteBasicConfigVersionFile)
include(CheckFunctionExists)
include(CheckIncludeFiles)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddAppIcon)

find_package(Qt5 5.5 CONFIG REQUIRED
  Concurrent
  Core
  Gui
  DBus
  Widgets
  PrintSupport
  Xml
)

# determine kf5 version is at least 5.23
message(STATUS "Searching for KF5KIO version 5.23 or newer...")
find_package(KF5 5.23 QUIET COMPONENTS
  KIO
)
if(KF5_FOUND)
  set(KRARC_QUERY_ENABLED TRUE)
  add_definitions( "-DKRARC_QUERY_ENABLED" )
else(KF5_FOUND)
  message(STATUS "KF5KIO version 5.23 or newer not found, some krarc:/ functionality will be disabled.")
  set(KRARC_QUERY_ENABLED FALSE)
endif(KF5_FOUND)

find_package(KF5 REQUIRED COMPONENTS
  Archive
  Bookmarks
  Codecs
  Completion
  CoreAddons
  Config
  DocTools
  I18n
  IconThemes
  ItemViews
  KIO
  Notifications
  Parts
  Solid
  TextWidgets
  Wallet
  WidgetsAddons
  WindowSystem
  XmlGui
  GuiAddons
)

# Synchronizer is enabled by default, unless disabled from the command line (-DENABLE_SYNCHRONIZER=OFF)
option(ENABLE_SYNCHRONIZER "Enable Synchronizer" ON)

# if an old setting is being used, use the setting that complies with the structure of other Krusader settings
if(ENABLE_SYNCHRONIZER)
    set(SYNCHRONIZER_ENABLED TRUE)
    add_definitions( "-DSYNCHRONIZER_ENABLED" )
else()
    set(SYNCHRONIZER_ENABLED FALSE)
endif()

# For security reasons, absolute kdesu path is set at build time and is not
# configurable.
if(NOT KDESU_PATH)
    if(EXISTS "${KDE_INSTALL_FULL_LIBDIR}/kf5/kdesu")
      # Used by Arch distribution
      set(KDESU_PATH "${KDE_INSTALL_FULL_LIBDIR}/kf5/kdesu")
    else()
      set(KDESU_PATH "${KDE_INSTALL_FULL_LIBEXECDIR_KF5}/kdesu")
    endif()
endif()
message("kdesu path set: ${KDESU_PATH}")
add_definitions( -DKDESU_PATH="${KDESU_PATH}" )

add_definitions(${QT_DEFINITIONS} ${KF5_DEFINITIONS})
add_definitions( "-DKRARC_ENABLED" )
add_definitions( "-DQT_NO_URL_CAST_FROM_STRING" )

check_include_files(attr/libattr.h HAVE_ATTR_LIBATTR_H)
check_include_files(sys/xattr.h HAVE_SYS_XATTR_H)
check_include_files(sys/acl.h HAVE_SYS_ACL_H)
check_include_files(acl/libacl.h HAVE_ACL_LIBACL_H)

if (HAVE_ATTR_LIBATTR_H AND HAVE_SYS_XATTR_H AND HAVE_SYS_ACL_H AND HAVE_ACL_LIBACL_H)
    set(ACL_HEADERS_FOUND TRUE)
endif(HAVE_ATTR_LIBATTR_H AND HAVE_SYS_XATTR_H AND HAVE_SYS_ACL_H AND HAVE_ACL_LIBACL_H)

if (ACL_HEADERS_FOUND)
    find_library(ACL_LIBS NAMES acl )
    find_library(ATTR_LIBS NAMES attr )
endif(ACL_HEADERS_FOUND)

if (ACL_HEADERS_FOUND AND ACL_LIBS AND ATTR_LIBS)
    set(ACL_FOUND TRUE)
    set(ACL_LIBS ${ACL_LIBS} ${ATTR_LIBS})
    message(STATUS "Found ACL support: ${ACL_LIBS}")
    add_definitions(-DHAVE_POSIX_ACL)
endif(ACL_HEADERS_FOUND AND ACL_LIBS AND ATTR_LIBS)

#include(ConfigureChecks.cmake)

add_subdirectory(krusader)
add_subdirectory(pics)
add_subdirectory(doc-extras)
add_subdirectory(krArc)
add_subdirectory(iso)
add_subdirectory(doc)
#add_subdirectory(virt) - not used

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
