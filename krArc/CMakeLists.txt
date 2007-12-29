
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(kio_krarc_PART_SRCS krarc.cpp )

kde4_add_plugin(kio_krarc ${kio_krarc_PART_SRCS})

target_link_libraries(kio_krarc  ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS} ${KDE4_KFILE_LIBS} ${KDE4_KDE3SUPPORT_LIBS} )

install(TARGETS kio_krarc DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############




#original Makefile.am contents follow:

######## kdevelop will overwrite this part!!! (begin)##########
#kde_module_LTLIBRARIES = kio_krarc.la
#
#
#INCLUDES = $(all_includes)
#
#
##LDFLAGS =    
#
#kio_krarc_la_METASOURCES=AUTO
#
#kio_krarc_la_SOURCES = krarc.cpp
#kio_krarc_la_LIBADD = $(LIB_QT) $(LIB_KDECORE) $(LIB_KIO)
## $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT)
#
#kio_krarc_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
#
#noinst_HEADERS = krarc.h
#
#EXTRA_DIST = krarc.protocol
#
#install-data-local:
#	$(mkinstalldirs) $(DESTDIR)$(kde_servicesdir)
#	$(INSTALL_DATA) $(srcdir)/krarc.protocol $(DESTDIR)$(kde_servicesdir)/krarc.protocol
#
#uninstall-local:
#	rm -f  $(DESTDIR)$(kde_servicesdir)/krarc.protocol
#
######## kdevelop will overwrite this part!!! (end)############
## These paths are KDE specific. Use them:
## kde_appsdir         Where your application's menu entry (.desktop) should go to.
## kde_icondir         Where your icon should go to - better use KDE_ICON.
## kde_sounddir        Where your sounds should go to.
## kde_htmldir         Where your docs should go to. (contains lang subdirs)
## kde_datadir         Where you install application data. (Use a subdir)
## kde_locale          Where translation files should go to. (contains lang subdirs)
## kde_cgidir          Where cgi-bin executables should go to.
## kde_confdir         Where config files should go to (system-wide ones with default values).
## kde_mimedir         Where mimetypes .desktop files should go to.
## kde_servicesdir     Where services .desktop files should go to.
## kde_servicetypesdir Where servicetypes .desktop files should go to.
## kde_toolbardir      Where general toolbar icons should go to (deprecated, use KDE_ICON).
## kde_wallpaperdir    Where general wallpapers should go to.
## kde_templatesdir    Where templates for the "New" menu (Konqueror/KDesktop) should go to.
## kde_bindir          Where executables should go to. Use bin_PROGRAMS or bin_SCRIPTS.
## kde_libdir          Where shared libraries should go to. Use lib_LTLIBRARIES.
## kde_moduledir       Where modules (e.g. parts) should go to. Use kde_module_LTLIBRARIES.
## kde_styledir        Where Qt/KDE widget styles should go to (new in KDE 3).
## kde_designerdir     Where Qt Designer plugins should go to (new in KDE 3).
#
#
## make messages.po. Move this one to ../po/ and "make merge" in po
## the -x is for skipping messages already translated in kdelibs
#messages:
#	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
#	if test -n "$$LIST"; then \
#		 $(XGETTEXT) -C -ki18n -x $(kde_includes)/kde.pot $$LIST -o ../po/krarc.pot; \
#	fi
#
