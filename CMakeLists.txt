cmake_minimum_required(VERSION 3.16)

project(krusader)

set(VERSION "2.10.0-dev")
set(RELEASE_NAME "Bleeding Edge")

set(CMAKE_CXX_STANDARD 17)

set(MIN_QT_VERSION "6.4.0")
set(KF6_MIN_VERSION "5.240.0")


# ===== Packages =====

find_package(ECM ${KF6_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
set(KDE_COMPILERSETTINGS_LEVEL "5.200")

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(KDEGitCommitHooks)

include(ECMInstallIcons)
include(ECMAddAppIcon)
include(ECMQtDeclareLoggingCategory)

include(FeatureSummary)
include(CheckIncludeFiles)

find_package(Qt6 ${MIN_QT_VERSION} CONFIG REQUIRED COMPONENTS
  Concurrent
  Core
  Gui
  DBus
  Widgets
  PrintSupport
  Xml
  Core5Compat
)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
  Archive
  Bookmarks
  Codecs
  Completion
  CoreAddons
  Config
  Crash
  DocTools
  GlobalAccel
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
  StatusNotifierItem
  ColorScheme
)


# ===== Get GIT revision =====

execute_process(COMMAND git log --pretty=format:%h -n 1
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_REVISION
                ERROR_QUIET)

# ===== Definitions =====

# Synchronizer is enabled by default, unless disabled from the command line (-DENABLE_SYNCHRONIZER=OFF)
option(ENABLE_SYNCHRONIZER "Enable Synchronizer" ON)

# If an old setting is being used, use the setting that complies with the structure of other Krusader settings.
if(ENABLE_SYNCHRONIZER)
    set(SYNCHRONIZER_ENABLED TRUE)
    add_definitions(-DSYNCHRONIZER_ENABLED)
else()
    set(SYNCHRONIZER_ENABLED FALSE)
endif()

# For security reasons, absolute kdesu path is set at build time and is not configurable.
if(NOT KDESU_PATH)
    set(KDESU_PATH "${KDE_INSTALL_FULL_LIBEXECDIR_KF}/kdesu")
endif()
add_definitions(-DKDESU_PATH="${KDESU_PATH}")
message(STATUS "kdesu path has been hard-coded: ${KDESU_PATH}")

add_definitions(-DKRARC_ENABLED)
add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

# Enable Krarc query, since MIN_KF_VERSION >= 5.23
add_definitions(-DKRARC_QUERY_ENABLED)


# ===== Additional checks =====

if (NOT CMAKE_INSTALL_PREFIX MATCHES "^/usr(/local)?/?$" AND NOT DEFINED KDE_INSTALL_QTPLUGINDIR
        AND NOT DEFINED KDE_INSTALL_USE_QT_SYS_PATHS)
    message(WARNING "CMAKE_INSTALL_PREFIX is not set to a standard location (/usr or /usr/local), \
                     krarc:/ protocol may not work. To force installing plugins into standard location \
                     use -DKDE_INSTALL_USE_QT_SYS_PATHS=true")
endif()


# ===== ACL support =====

check_include_files(attr/libattr.h HAVE_ATTR_LIBATTR_H)
check_include_files(sys/xattr.h HAVE_SYS_XATTR_H)
check_include_files(sys/acl.h HAVE_SYS_ACL_H)
check_include_files(acl/libacl.h HAVE_ACL_LIBACL_H)

if (HAVE_ATTR_LIBATTR_H AND HAVE_SYS_XATTR_H AND HAVE_SYS_ACL_H AND HAVE_ACL_LIBACL_H)
    set(ACL_HEADERS_FOUND TRUE)
endif(HAVE_ATTR_LIBATTR_H AND HAVE_SYS_XATTR_H AND HAVE_SYS_ACL_H AND HAVE_ACL_LIBACL_H)

if (ACL_HEADERS_FOUND)
    find_library(ACL_LIBS NAMES acl)
    find_library(ATTR_LIBS NAMES attr)
endif(ACL_HEADERS_FOUND)

if (ACL_HEADERS_FOUND AND ACL_LIBS AND ATTR_LIBS)
    set(ACL_FOUND TRUE)
    set(ACL_LIBS ${ACL_LIBS} ${ATTR_LIBS})
    message(STATUS "Found ACL support: ${ACL_LIBS}")
    add_definitions(-DHAVE_POSIX_ACL)
endif(ACL_HEADERS_FOUND AND ACL_LIBS AND ATTR_LIBS)


# ===== Compile options =====

if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fno-common -ansi -Wpedantic -Wconversion -Wpointer-arith \
                         -Wcast-qual -Wcast-align -Wwrite-strings -Wsuggest-override -Wfloat-equal \
                         -Woverloaded-virtual -Wundef -Wlogical-op -Wuninitialized -Wredundant-decls -Wnoexcept \
                         -Wnon-virtual-dtor -Wctor-dtor-privacy -Wzero-as-null-pointer-constant" CACHE STRING "" FORCE)
endif()

remove_definitions(-DQT_NO_KEYWORDS  -DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII)

# ===== Subdirectories =====

add_subdirectory(app)
add_subdirectory(doc/handbook)
add_subdirectory(plugins/iso)
add_subdirectory(plugins/krarc)


# ===== Translations =====

ki18n_install(po)
kdoctools_install(po)

# ==== clang-format target and commit hook ====

file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

# ===== Output configuration =====

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
