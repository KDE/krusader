
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############


kde4_create_handbook(index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en SUBDIR krusader)



#original Makefile.am contents follow:

######## kdevelop will overwrite this part!!! (begin)##########
#
#
#EXTRA_DIST = archives.docbook editors-note.docbook java.docbook profiles.docbook user-interface.docbook bookmarks.docbook more.docbook faq.docbook keyboard.docbook release-overview.docbook using-krusader.docbook checksum.docbook features.docbook konfigurator.docbook remote-connections.docbook vfs.docbook commands.docbook  glossary.docbook krusader-tools.docbook search.docbook viewer-editor.docbook compare.docbook help.docbook locate.docbook splitter.docbook configuration-files.docbook index.docbook mount.docbook synchronizer.docbook credits.docbook installation.docbook mouse.docbook useractions.docbook diskusage.docbook introduction.docbook  occupied-space.docbook useraction-xml.docbook
#
######## kdevelop will overwrite this part!!! (end)############
#
#KDE_LANG = en
#KDE_DOCS = krusader
#
#
