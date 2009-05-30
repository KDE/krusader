include_directories(${KDE4_INCLUDES})

configure_file(krusaderversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/krusaderversion.h)

add_subdirectory(ActionMan)
add_subdirectory(BookMan)
add_subdirectory(Dialogs)
add_subdirectory(DiskUsage)
add_subdirectory(Filter)
add_subdirectory(GUI)
add_subdirectory(Konfigurator)
add_subdirectory(KViewer)
add_subdirectory(Locate)
add_subdirectory(MountMan)
add_subdirectory(Queue)
add_subdirectory(Panel)
### add_subdirectory(RemoteMan) - deprecated ###
add_subdirectory(Search)
add_subdirectory(Splitter)
add_subdirectory(Synchronizer)
add_subdirectory(UserAction)
add_subdirectory(UserMenu)
add_subdirectory(VFS)

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(KRJSDIR)")

set(krusader_SRCS 
    paneltabbar.cpp 
    panelmanager.cpp 
    krservices.cpp 
    main.cpp 
    krusaderview.cpp 
    krusader.cpp 
    krslots.cpp 
    kicons.cpp 
    krusaderapp.cpp
    krtrashhandler.cpp)

qt4_add_dbus_adaptor(krusader_SRCS org.krusader.Instance.xml krusader.h Krusader)

kde4_add_app_icon(krusader_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/hi*-app-krusader_user.png")

kde4_add_executable(krusader ${krusader_SRCS})

target_link_libraries(krusader
                      BookMan
                      Dialogs
                      DiskUsage
                      GUI
                      Konfigurator
                      KViewer
                      MountMan
                      Panel
                      Queue
                      VFS
                      Search
                      Splitter
                      Synchronizer
                      UserMenu
                      Locate
                      UserAction
                      ActionMan
                      KViewer
                      Filter
                      Dialogs
                      GUI
                      ${KDE4_KPARTS_LIBS}
                      ${KDE4_KHTML_LIBS}
                      ${KDE4_KDEUI_LIBS}
                      ${KDE4_KFILE_LIBS}
                      ${QT_QT3SUPPORT_LIBRARY}
                      ${KDE4_KDE3SUPPORT_LIBS}
                      ${KDE4_KDECORE_LIBS})

install(TARGETS krusader DESTINATION ${BIN_INSTALL_DIR})
install(FILES krusader.desktop
              krusader_root-mode.desktop
        DESTINATION ${XDG_APPS_INSTALL_DIR} )
install(FILES krusaderui.rc
              midnight_commander.color
              total_commander.color
              total_commander.keymap
              total_commander.keymap.info
              useraction_examples.xml
              splash.png
        DESTINATION ${DATA_INSTALL_DIR}/krusader)

kde4_install_icons( ${ICON_INSTALL_DIR} )
