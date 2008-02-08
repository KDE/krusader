# the condition for this is found in configure.in.in
if include_libkonq
LIB_KONQ = -lkonq
endif

# the condition for this is found in configure.in.in
if include_libkjsembed
AM_CPPFLAGS = -D__KJSEMBED__
LIB_KJSEMBED = -lkjsembed
KRJSDIR = KrJS
LIB_KRJS = KrJS/libKrJS.a
endif


# these subdirs are also visited during the build-process
SUBDIRS = \
	ActionMan \
	BookMan \
	Dialogs \
	DiskUsage \
	Filter \
	GUI \
	Konfigurator \
	KViewer \
	Locate \
	MountMan \
	Queue \
	Panel \
	RemoteMan \
	Search \
	Splitter \
	Synchronizer \
	UserAction \
	$(KRJSDIR) \
	UserMenu \
	VFS


###########################################################
######################## BUILDING #########################
##########################################################


# the programm which is build and installed:
bin_PROGRAMS = krusader


krusader_SOURCES = \
	paneltabbar.cpp \
	panelmanager.cpp \
	krservices.cpp \
	main.cpp \
	krusaderview.cpp \
	krusader.cpp \
	krslots.cpp \
	kicons.cpp 


# this makes that .ui and .moc files are generated automaticly.
# to make this happen include filename.moc at the end of filename.cpp (assuming that filename.h is the header to use)
# and add filename.ui to the *_SOURCES
METASOURCES = AUTO


# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

# these are the libs which are liked together for krusader.
# the correct order is very important, also the double entries are necessary!
krusader_LDADD   = \
	BookMan/libBookMan.a \
	Dialogs/libDialogs.a \
	DiskUsage/libDiskUsage.a \
	DiskUsage/radialMap/libradialmap.a \
	DiskUsage/filelightParts/libfilelightparts.a \
	GUI/libGUI.a \
	Konfigurator/libKonfigurator.a \
	KViewer/libKViewer.a \
	MountMan/libMountMan.a \
	Panel/libPanel.a \
	Queue/libQueue.a \
	RemoteMan/libRemoteMan.a \
	VFS/libVFS.a \
	Search/libSearch.a \
	Splitter/libSplitter.a \
	Synchronizer/libSynchronizer.a \
	UserMenu/libUserMenu.a \
	Locate/libLocate.a \
	UserAction/libUserAction.a \
	ActionMan/libActionMan.a \
	$(LIB_KRJS) \
	KViewer/libKViewer.a \
	Filter/libFilter.a \
	Dialogs/libDialogs.a \
	GUI/libGUI.a \
	$(LIB_KONQ) \
	$(LIB_KJSEMBED) \
	$(LIB_KPARTS) \
	$(LIB_KFILE) \
	$(LIB_KHTML) \
	$(LIB_KDEUI) \
	$(LIB_KDECORE) \
	$(LIB_QT) \
	$(LIBSOCKET)


# the library search path.
krusader_LDFLAGS = $(all_libraries) $(kde_RPATH)


###########################################################
###################### INSTALLING #########################
##########################################################


# this installes alle the icons in the current dir.
# see the comment in krusader_kde3/icons/Makefile.am for more info
KDE_ICON = AUTO

# How it works (by jonas): 
# somenamedir = /path/what/ever		# a directory
# somename_DATA = file1 file2 file3	# the data whichshold be installed there

# These paths are KDE specific. Use them (jonas: taken from the old Makefile.am):
# kde_appsdir		Where your application's menu entry (.desktop) should go to.
# xdg_appsdir		Where your application's .desktop-files should go acording to freedesktop.org's standard
# kde_icondir		Where your icon should go to - better use KDE_ICON.
# kde_sounddir		Where your sounds should go to.
# kde_htmldir		Where your docs should go to. (contains lang subdirs)
# kde_datadir		Where you install application data. (Use a subdir)
# kde_locale		Where translation files should go to. (contains lang subdirs)
# kde_cgidir		Where cgi-bin executables should go to.
# kde_confdir		Where config files should go to (system-wide ones with default values).
# kde_mimedir		Where mimetypes .desktop files should go to.
# kde_servicesdir		Where services .desktop files should go to.
# kde_servicetypesdir	Where servicetypes .desktop files should go to.
# kde_wallpaperdir	Where general wallpapers should go to.
# kde_templatesdir	Where templates for the "New" menu (Konqueror/KDesktop) should go to.
# kde_bindir		Where executables should go to. Use bin_PROGRAMS or bin_SCRIPTS.
# kde_libdir		Where shared libraries should go to. Use lib_LTLIBRARIES.
# kde_moduledir		Where modules (e.g. parts) should go to. Use kde_module_LTLIBRARIES.
# kde_styledir		Where Qt/KDE widget styles should go to (new in KDE 3).
# kde_designerdir	Where Qt Designer plugins should go to (new in KDE 3).


# this installes the .desktop-files at hte right place
# use 'xdg' instead of 'kde' here to follow freedesktop.org's standard (jonas)
xdg_apps_DATA = \
	krusader.desktop \
	krusader_root-mode.desktop

# this installes all additional data.
krusaderlocaldir = $(kde_datadir)/krusader
krusaderlocal_DATA = \
	krusaderui.rc \
	alpa-yellow.color \
	bash.color \
	dos_navigator.color \
	midnight_commander.color \
	midnight_cmd_AHamann.color \
	total_commander.color \
	total_cmd_pleasent.color \
	midnight_commander.keymap \
	midnight_commander.keymap.info \
	total_commander.keymap \
	total_commander.keymap.info \
	useraction_examples.xml \
	splash.png

# this installs data for example javascript useractions.
krusaderuajsdir = $(kde_datadir)/krusader/js
krusaderuajs_DATA = \
       recode.js \
       recode.ui \
       mount.js \
       mount.ui \
       calc.js  \
       calc.ui \
       select_from_file.js

# Jonas: I've no idea how this thing works so I leave it like it was.

# WARNING: if you use a ui.rc file above, use:
# messages: rc.cpp
# instead of
# messages:

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/krusader.pot; \
	fi
