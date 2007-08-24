
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############


kde4_create_handbook(index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en)



#original Makefile.am contents follow:

######## kdevelop will overwrite this part!!! (begin)##########
#
#
#EXTRA_DIST = index.docbook help.docbook editors-note.docbook introduction.docbook features.docbook user-interface.docbook using-krusader.docbook commands.docbook keyboard.docbook mouse.docbook krusader-tools.docbook konfigurator.docbook faq.docbook credits.docbook installation.docbook configuration-files.docbook useraction-xml.docbook release-overview.docbook glossary.docbook
#
######## kdevelop will overwrite this part!!! (end)############
#
#KDE_LANG = ru
#KDE_DOCS = krusader
#
#
