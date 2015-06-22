project(krusader)

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)
set (QT_MIN_VERSION "5.2.0")

set(VERSION "15.08-beta" )
set(RELEASE_NAME "Clear Skies")

find_package(ECM 1.1.0 REQUIRED NO_MODULE)
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

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(Qt5 CONFIG REQUIRED
  Core
  Gui
  DBus
  Widgets
  PrintSupport
  Xml
)

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
)

add_definitions(${QT_DEFINITIONS} ${KF5_DEFINITIONS})
add_definitions( "-DKRARC_ENABLED" )
add_definitions( "-DQT_NO_URL_CAST_FROM_STRING" )

if(ENABLE_SYNCHRONIZER)
    add_definitions( "-DENABLE_SYNCHRONIZER" )
endif(ENABLE_SYNCHRONIZER)

find_package(ACL)
if(ACL_FOUND)
    add_definitions(-DHAVE_POSIX_ACL)
endif(ACL_FOUND)

#include(ConfigureChecks.cmake)

add_subdirectory(krusader)
add_subdirectory(pics)
add_subdirectory(doc-extras)
add_subdirectory(krArc)
add_subdirectory(iso)
add_subdirectory(doc)
#add_subdirectory(virt) - not used

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
