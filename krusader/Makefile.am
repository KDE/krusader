####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = krusader

## INCLUDES were found outside kdevelop specific part

if include_libkonq
LIB_KONQ = -lkonq
endif

krusader_SOURCES = paneltabbar.cpp panelmanager.cpp krservices.cpp main.cpp krusaderview.cpp krusader.cpp krslots.cpp kicons.cpp 
krusader_LDADD   = ./BookMan/libBookMan.a ./Dialogs/libDialogs.a ./DiskUsage/libDiskUsage.a ./DiskUsage/radialMap/libradialmap.a ./DiskUsage/filelightParts/libfilelightparts.a ./GUI/libGUI.a ./Konfigurator/libKonfigurator.a ./KViewer/libKViewer.a ./MountMan/libMountMan.a ./Panel/libPanel.a ./RemoteMan/libRemoteMan.a ./VFS/libVFS.a ./Search/libSearch.a ./Splitter/libSplitter.a ./Synchronizer/libSynchronizer.a ./UserMenu/libUserMenu.a ./Locate/libLocate.a UserAction/libUserAction.a VFS/libVFS.a KViewer/libKViewer.a Dialogs/libDialogs.a GUI/libGUI.a $(LIB_KONQ) $(LIB_KPARTS) $(LIB_KFILE) $(LIB_KHTML) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)

SUBDIRS = BookMan Dialogs DiskUsage GUI Konfigurator KViewer MountMan Panel RemoteMan VFS \
        Search Splitter Synchronizer Locate UserAction UserMenu

EXTRA_DIST = krusader.desktop defaults.h kicons.cpp kicons.h krslots.cpp krslots.h krusader.cpp krusader.h krusaderui.rc krusaderview.cpp krusaderview.h main.cpp resources.h krservices.cpp krservices.h panelmanager.cpp panelmanager.h paneltabbar.cpp paneltabbar.h 16-blue-krusader.png 22-blue-krusader.png 32-blue-krusader.png 48-blue-krusader.png 64-blue-krusader.png 16-red-krusader.png 22-red-krusader.png 32-red-krusader.png 48-red-krusader.png 64-red-krusader.png 

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(kde_appsdir)/Applications/
	$(INSTALL_DATA) $(srcdir)/krusader.desktop $(DESTDIR)$(kde_appsdir)/Applications/krusader.desktop
	$(mkinstalldirs) $(DESTDIR)$(kde_datadir)/krusader/
	$(INSTALL_DATA) $(srcdir)/krusaderui.rc $(DESTDIR)$(kde_datadir)/krusader/krusaderui.rc
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/16-blue-krusader.png $(DESTDIR)$(kde_icondir)/hicolor/16x16/apps/krusader.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/22x22/apps/
	$(INSTALL_DATA) $(srcdir)/22-blue-krusader.png $(DESTDIR)$(kde_icondir)/hicolor/22x22/apps/krusader.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/32-blue-krusader.png $(DESTDIR)$(kde_icondir)/hicolor/32x32/apps/krusader.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/48x48/apps/
	$(INSTALL_DATA) $(srcdir)/48-blue-krusader.png $(DESTDIR)$(kde_icondir)/hicolor/48x48/apps/krusader.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/64x64/apps/
	$(INSTALL_DATA) $(srcdir)/64-blue-krusader.png $(DESTDIR)$(kde_icondir)/hicolor/64x64/apps/krusader.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/16-red-krusader.png $(DESTDIR)$(kde_icondir)/hicolor/16x16/apps/krusader_red.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/22x22/apps/
	$(INSTALL_DATA) $(srcdir)/22-red-krusader.png $(DESTDIR)$(kde_icondir)/hicolor/22x22/apps/krusader_red.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/32-red-krusader.png $(DESTDIR)$(kde_icondir)/hicolor/32x32/apps/krusader_red.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/48x48/apps/
	$(INSTALL_DATA) $(srcdir)/48-red-krusader.png $(DESTDIR)$(kde_icondir)/hicolor/48x48/apps/krusader_red.png
	$(mkinstalldirs) $(DESTDIR)$(kde_icondir)/hicolor/64x64/apps/
	$(INSTALL_DATA) $(srcdir)/64-red-krusader.png $(DESTDIR)$(kde_icondir)/hicolor/64x64/apps/krusader_red.png

uninstall-local:
	-rm -f $(DESTDIR)$(kde_appsdir)/Applications/krusader.desktop
	-rm -f $(DESTDIR)$(kde_datadir)/krusader/krusaderui.rc
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/16x16/apps/krusader.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/22x22/apps/krusader.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/32x32/apps/krusader.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/48x48/apps/krusader.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/64x64/apps/krusader.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/16x16/apps/krusader_red.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/22x22/apps/krusader_red.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/32x32/apps/krusader_red.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/48x48/apps/krusader_red.png
	-rm -f $(DESTDIR)$(kde_icondir)/hicolor/64x64/apps/krusader_red.png

####### kdevelop will overwrite this part!!! (end)############
# These paths are KDE specific. Use them:
# kde_appsdir         Where your application's menu entry (.desktop) should go to.
# kde_icondir         Where your icon should go to - better use KDE_ICON.
# kde_sounddir        Where your sounds should go to.
# kde_htmldir         Where your docs should go to. (contains lang subdirs)
# kde_datadir         Where you install application data. (Use a subdir)
# kde_locale          Where translation files should go to. (contains lang subdirs)
# kde_cgidir          Where cgi-bin executables should go to.
# kde_confdir         Where config files should go to (system-wide ones with default values).
# kde_mimedir         Where mimetypes .desktop files should go to.
# kde_servicesdir     Where services .desktop files should go to.
# kde_servicetypesdir Where servicetypes .desktop files should go to.
# kde_toolbardir      Where general toolbar icons should go to (deprecated, use KDE_ICON).
# kde_wallpaperdir    Where general wallpapers should go to.
# kde_templatesdir    Where templates for the "New" menu (Konqueror/KDesktop) should go to.
# kde_bindir          Where executables should go to. Use bin_PROGRAMS or bin_SCRIPTS.
# kde_libdir          Where shared libraries should go to. Use lib_LTLIBRARIES.
# kde_moduledir       Where modules (e.g. parts) should go to. Use kde_module_LTLIBRARIES.
# kde_styledir        Where Qt/KDE widget styles should go to (new in KDE 3).
# kde_designerdir     Where Qt Designer plugins should go to (new in KDE 3).

# set the include path for X, qt and KDE
INCLUDES= $(all_includes)

METASOURCES = AUTO

# the library search path.
krusader_LDFLAGS = $(all_libraries) $(kde_RPATH)

# Uncomment the following two lines if you add a ui.rc file for your application to make use of
# KDEs XML GUI builing
rcdir = $(DESTDIR)$(kde_datadir)/krusader
rc_DATA = krusaderui.rc

#WARNING: if you use a ui.rc file above, use:

# messages: rc.cpp

# instead of

# messages:

messages: rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/krusader.pot; \
	fi

noinst_HEADERS = panelmanager.h paneltabbar.h
