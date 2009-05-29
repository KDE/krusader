
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )
install( FILES bookmanadd.png bookmanedit.png cmdline.png fnkeys.png kgstartup.png krusader1.png listpanel.png mainwindow.png mountman.png actionman.png search_advanced.png search_general.png terminalEmu.png toolbar.png tabbed_browsing.png krusader_title.png syncdir.png DESTINATION ${HTML_INSTALL_DIR}/en/krusader)


########### install files ###############




#original Makefile.am contents follow:

######## kdevelop will overwrite this part!!! (begin)##########
#
#
#EXTRA_DIST = bookmanadd.png bookmanedit.png cmdline.png fnkeys.png kgstartup.png krusader1.png listpanel.png mainwindow.png mountman.png actionman.png search_advanced.png search_general.png terminalEmu.png toolbar.png tabbed_browsing.png krusader_title.png syncdir.png 
#
#install-data-local:
#	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/krusader/
#	$(INSTALL_DATA) $(srcdir)/bookmanadd.png $(DESTDIR)$(kde_htmldir)/en/krusader/bookmanadd.png
#	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/krusader/
#	$(INSTALL_DATA) $(srcdir)/bookmanedit.png $(DESTDIR)$(kde_htmldir)/en/krusader/bookmanedit.png
#	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/krusader/
#	$(INSTALL_DATA) $(srcdir)/cmdline.png $(DESTDIR)$(kde_htmldir)/en/krusader/cmdline.png
#	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/krusader/
#	$(INSTALL_DATA) $(srcdir)/fnkeys.png $(DESTDIR)$(kde_htmldir)/en/krusader/fnkeys.png
#	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/krusader/
#	$(INSTALL_DATA) $(srcdir)/kgstartup.png $(DESTDIR)$(kde_htmldir)/en/krusader/kgstartup.png
#	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/krusader/
#	$(INSTALL_DATA) $(srcdir)/krusader1.png $(DESTDIR)$(kde_htmldir)/en/krusader/krusader1.png
#	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/krusader/
#	$(INSTALL_DATA) $(srcdir)/listpanel.png $(DESTDIR)$(kde_htmldir)/en/krusader/listpanel.png
#	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/krusader/
#	$(INSTALL_DATA) $(srcdir)/mainwindow.png $(DESTDIR)$(kde_htmldir)/en/krusader/mainwindow.png
#	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/krusader/
#	$(INSTALL_DATA) $(srcdir)/mountman.png $(DESTDIR)$(kde_htmldir)/en/krusader/mountman.png
#	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/krusader/
#	$(INSTALL_DATA) $(srcdir)/actionman.png $(DESTDIR)$(kde_htmldir)/en/krusader/actionman.png
#	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/krusader/
#	$(INSTALL_DATA) $(srcdir)/search_advanced.png $(DESTDIR)$(kde_htmldir)/en/krusader/search_advanced.png
#	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/krusader/
#	$(INSTALL_DATA) $(srcdir)/search_general.png $(DESTDIR)$(kde_htmldir)/en/krusader/search_general.png
#	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/krusader/
#	$(INSTALL_DATA) $(srcdir)/terminalEmu.png $(DESTDIR)$(kde_htmldir)/en/krusader/terminalEmu.png
#	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/krusader/
#	$(INSTALL_DATA) $(srcdir)/toolbar.png $(DESTDIR)$(kde_htmldir)/en/krusader/toolbar.png
#	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/krusader/
#	$(INSTALL_DATA) $(srcdir)/tabbed_browsing.png $(DESTDIR)$(kde_htmldir)/en/krusader/tabbed_browsing.png
#	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/krusader/
#	$(INSTALL_DATA) $(srcdir)/krusader_title.png $(DESTDIR)$(kde_htmldir)/en/krusader/krusader_title.png
#	$(mkinstalldirs) $(DESTDIR)$(kde_htmldir)/en/krusader/
#	$(INSTALL_DATA) $(srcdir)/syncdir.png $(DESTDIR)$(kde_htmldir)/en/krusader/syncdir.png
#
#uninstall-local:
#	-rm -f $(DESTDIR)$(kde_htmldir)/en/krusader/bookmanadd.png
#	-rm -f $(DESTDIR)$(kde_htmldir)/en/krusader/bookmanedit.png
#	-rm -f $(DESTDIR)$(kde_htmldir)/en/krusader/cmdline.png
#	-rm -f $(DESTDIR)$(kde_htmldir)/en/krusader/fnkeys.png
#	-rm -f $(DESTDIR)$(kde_htmldir)/en/krusader/kgstartup.png
#	-rm -f $(DESTDIR)$(kde_htmldir)/en/krusader/krusader1.png
#	-rm -f $(DESTDIR)$(kde_htmldir)/en/krusader/listpanel.png
#	-rm -f $(DESTDIR)$(kde_htmldir)/en/krusader/mainwindow.png
#	-rm -f $(DESTDIR)$(kde_htmldir)/en/krusader/mountman.png
#	-rm -f $(DESTDIR)$(kde_htmldir)/en/krusader/actionman.png
#	-rm -f $(DESTDIR)$(kde_htmldir)/en/krusader/search_advanced.png
#	-rm -f $(DESTDIR)$(kde_htmldir)/en/krusader/search_general.png
#	-rm -f $(DESTDIR)$(kde_htmldir)/en/krusader/terminalEmu.png
#	-rm -f $(DESTDIR)$(kde_htmldir)/en/krusader/toolbar.png
#	-rm -f $(DESTDIR)$(kde_htmldir)/en/krusader/tabbed_browsing.png
#	-rm -f $(DESTDIR)$(kde_htmldir)/en/krusader/krusader_title.png
#	-rm -f $(DESTDIR)$(kde_htmldir)/en/krusader/syncdir.png
######## kdevelop will overwrite this part!!! (end)############
