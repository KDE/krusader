include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES})

set(kio_tar_PART_SRCS tar.cc)

kde4_add_plugin(kio_tar ${kio_tar_PART_SRCS})

target_link_libraries(kio_tar ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS})

install(TARGETS kio_tar DESTINATION ${PLUGIN_INSTALL_DIR})

set(ktartest_SRCS ktartest.cpp)

if(KDE4_BUILD_TESTS)
    kde4_add_executable(ktartest ${ktartest_SRCS})
    target_link_libraries(ktartest ${KDE4_KDECORE_LIBS} ${KDE4_KIO_LIBS})
endif(KDE4_BUILD_TESTS)

install(FILES tar.protocol
              ar.protocol
              zip.protocol
        DESTINATION ${SERVICES_INSTALL_DIR})
