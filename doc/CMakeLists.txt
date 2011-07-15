########### install files ###############


kde4_create_handbook(index.docbook INSTALL_DESTINATION ${HTML_INSTALL_DIR}/en SUBDIR krusader)
kde4_create_manpage(man-krusader.1.docbook 1 INSTALL_DESTINATION ${MAN_INSTALL_DIR})

