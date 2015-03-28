include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(kio_krarc_PART_SRCS krarc.cpp)

add_library(kio_krarc ${kio_krarc_PART_SRCS})

# TODO porting : new variables needed?
target_link_libraries(kio_krarc KF5::Archive KF5::KIOCore KF5::WidgetsAddons KF5::KDELibs4Support
# 		      ${KDE4_KDECORE_LIBS}
# 		      ${KDE4_KIO_LIBS}
# 		      ${KDE4_KFILE_LIBS}
		      )

install(TARGETS kio_krarc DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES krarc.protocol DESTINATION  ${SERVICES_INSTALL_DIR})
