####### kdevelop will overwrite this part!!! (begin)##########

# the condition for this is found in configure.in.in
if with_tar
TARDIR = tar
endif

SUBDIRS = krusader po pics doc krArc iso $(TARDIR)

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
