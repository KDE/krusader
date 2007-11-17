
add_subdirectory( en )
#add_subdirectory( ru )
add_subdirectory( pics )

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############




#original Makefile.am contents follow:

######## kdevelop will overwrite this part!!! (begin)##########
#
#SUBDIRS = en ru pics 
#
#EXTRA_DIST = krusader.1 
#
#install-data-local:
#	$(mkinstalldirs) $(DESTDIR)$(mandir)/man1/
#	$(INSTALL_DATA) $(srcdir)/krusader.1 $(DESTDIR)$(mandir)/man1/krusader.1
#
#uninstall-local:
#	-rm -f $(DESTDIR)$(mandir)/man1/krusader.1
#
######## kdevelop will overwrite this part!!! (end)############
#
