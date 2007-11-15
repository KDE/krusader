
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############
kde4_install_icons( ${ICON_INSTALL_DIR}/krusader/icons )

#install( FILES  DESTINATION ${DATA_INSTALL_DIR}/krusader/icons )



#original Makefile.am contents follow:

## with *_ICON = AUTO automake copies them automaticly into the right folders.
## the naming-convention which is nessesary for this is:
## <theme><size>-<type>-<name>.png (they will be installed as <name>.png)
##
## <theme> is "hi", "lo" or "cr" for hicolor, locolor or crystal
## <size> is 16, 22 or 32 for locolor; 16, 22, 32, 48, 64, 128 or "sc" for hicolor and Crystal.
##	"sc" stands for scalable and should be in SVG format compressed with GZip and with the
##	extention .svgz.
## <type> is "app" for an application icon,
##	"mime" for a mimetype icon,
##	"action" for a menu-item/toolbar-button,
##	"filesys" for some filesystem icons (dirs etc.) and
##	"device" for the cdrom/floppy/harddisk/etc. icons.
##
## This info is copied from http://developer.kde.org/documentation/other/makefile_am_howto.html
## Jonas 2005-04-15
#
#krusadericondir = $(kde_datadir)/krusader/icons
#krusadericon_ICON = AUTO
