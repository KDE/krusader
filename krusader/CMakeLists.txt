include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

configure_file(krusaderversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/krusaderversion.h)

add_subdirectory(ActionMan)
add_subdirectory(Archive)
add_subdirectory(BookMan)
add_subdirectory(Dialogs)
add_subdirectory(DiskUsage)
add_subdirectory(FileSystem)
add_subdirectory(Filter)
add_subdirectory(GUI)
add_subdirectory(Konfigurator)
add_subdirectory(KViewer)
add_subdirectory(JobMan)
add_subdirectory(Locate)
add_subdirectory(MountMan)
add_subdirectory(Panel)
add_subdirectory(Search)
add_subdirectory(Splitter)
add_subdirectory(UserAction)

if(SYNCHRONIZER_ENABLED)
    add_subdirectory(Synchronizer)
endif(SYNCHRONIZER_ENABLED)

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(KRJSDIR)")

set(krusader_SRCS
    krglobal.cpp
    actionsbase.cpp
    tabactions.cpp
    kractions.cpp
    paneltabbar.cpp
    panelmanager.cpp
    krservices.cpp
    main.cpp
    krusaderview.cpp
    krusader.cpp
    krslots.cpp
    kicons.cpp
    krdebuglogger.cpp
)

file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/icons/*-apps-krusader_user.png")
ecm_add_app_icon(krusader_SRCS ICONS ${ICONS_SRCS})

qt5_add_resources(krusader_RC_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc" )

add_executable(krusader ${krusader_SRCS} ${krusader_RC_SRCS})

target_link_libraries(krusader
                      Panel
                      PanelView
                      BookMan
                      Dialogs
                      DiskUsage
                      GUI
                      Konfigurator
                      KViewer
                      MountMan
                      FileSystem
                      Search
                      Splitter
                      Locate
                      UserAction
                      ActionMan
                      KViewer
                      Filter
                      Dialogs
                      GUI
                      Archive
                      JobMan
                      KF5::Notifications
                      KF5::Parts
                      KF5::WindowSystem
                      Qt5::PrintSupport
                      Qt5::Concurrent
)

if(SYNCHRONIZER_ENABLED)
    target_link_libraries( krusader Synchronizer )
endif(SYNCHRONIZER_ENABLED)

install(TARGETS krusader ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.kde.krusader.desktop
        DESTINATION ${XDG_APPS_INSTALL_DIR} )
install(FILES krusaderui.rc
              krusaderlisterui.rc
              krviewer.rc
        DESTINATION ${KXMLGUI_INSTALL_DIR}/krusader)
install(FILES midnight_commander.color
              total_commander.color
              total_commander.keymap
              total_commander.keymap.info
              useraction_examples.xml
              layout.xml
              splash.png
        DESTINATION ${DATA_INSTALL_DIR}/krusader)
install(FILES org.kde.krusader.appdata.xml
        DESTINATION ${KDE_INSTALL_METAINFODIR})

ecm_install_icons(ICONS
    icons/16-apps-krusader_blue.png
    icons/16-apps-krusader_red.png
    icons/16-apps-krusader_root.png
    icons/16-apps-krusader_user.png
    icons/22-apps-krusader_blue.png
    icons/22-apps-krusader_red.png
    icons/22-apps-krusader_root.png
    icons/22-apps-krusader_shield.png
    icons/22-apps-krusader_user.png
    icons/32-apps-krusader_blue.png
    icons/32-apps-krusader_red.png
    icons/32-apps-krusader_root.png
    icons/32-apps-krusader_shield.png
    icons/32-apps-krusader_user.png
    icons/48-apps-krusader_blue.png
    icons/48-apps-krusader_red.png
    icons/48-apps-krusader_root.png
    icons/48-apps-krusader_shield.png
    icons/48-apps-krusader_user.png
    icons/64-apps-krusader_blue.png
    icons/64-apps-krusader_red.png
    icons/64-apps-krusader_root.png
    icons/64-apps-krusader_shield.png
    icons/64-apps-krusader_user.png
    icons/128-apps-krusader_root.png
    icons/128-apps-krusader_user.png
    DESTINATION ${ICON_INSTALL_DIR}
)
