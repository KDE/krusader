####### kdevelop will overwrite this part!!! (begin)##########

SUBDIRS = krusader po pics doc krArc iso

EXTRA_DIST = krusader.kdevprj admin AUTHORS COPYING ChangeLog INSTALL README TODO krusader.lsm krusader.spec 

####### kdevelop will overwrite this part!!! (end)############
# not a GNU package. You can remove this line, if 
# have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign

MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 

package-messages:
	$(MAKE) -f admin/Makefile.common package-messages
	$(MAKE) -C po merge

dist-hook:
	cd $(top_distdir) && perl admin/am_edit -padmin
	cd $(top_distdir) && $(MAKE) -f admin/Makefile.common subdirs

include admin/deps.am
