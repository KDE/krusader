include_directories(${KF5_INCLUDE_DIRS})

# TODO porting : don't remember anymore
#include_directories(
#  ${CMAKE_CURRENT_SOURCE_DIR}
#  ${CMAKE_CURRENT_SOURCE_DIR}/session
#)

configure_file(krusaderversion.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/krusaderversion.h)

#find_package(LibKonq)

#if(LIBKONQ_FOUND)
#    message(STATUS "libkonq library found")
#    add_definitions(-D__LIBKONQ__)
#    include_directories( ${LIBKONQ_INCLUDE_DIR} )
#else(LIBKONQ_FOUND)
#    message(STATUS "libkonq library not found")
#endif(LIBKONQ_FOUND)

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
add_subdirectory(UserAction)
add_subdirectory(UserMenu)
add_subdirectory(VFS)

if(ENABLE_SYNCHRONIZER)
    add_subdirectory(Synchronizer)
endif(ENABLE_SYNCHRONIZER)

message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}: skipped subdir $(KRJSDIR)")

set(krusader_SRCS
    krmainwindow.cpp
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
    krtrashhandler.cpp)

# TODO porting: missing kf5 equivalent
#kde4_add_app_icon(krusader_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/hi*-app-krusader_user.png")

qt5_add_resources(krusader_RC_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc" )

add_executable(krusader ${krusader_SRCS} ${krusader_RC_SRCS})

target_link_libraries(krusader
                      Panel
                      BookMan
                      Dialogs
                      DiskUsage
                      GUI
                      Konfigurator
                      KViewer
                      MountMan
                      Queue
                      VFS
                      Search
                      Splitter
                      UserMenu
                      Locate
                      UserAction
                      ActionMan
                      KViewer
                      Filter
                      Dialogs
                      GUI
#                       ${KDE4_KPARTS_LIBS}
#                       ${KDE4_KDEUI_LIBS}
#                       ${KDE4_KFILE_LIBS}
#                       ${KDE4_KDECORE_LIBS}
#                       ${KDE4_SOLID_LIBS}
                      )

if(ENABLE_SYNCHRONIZER)
    target_link_libraries( krusader Synchronizer )
endif(ENABLE_SYNCHRONIZER)

if(LIBKONQ_FOUND)
    target_link_libraries( krusader ${LIBKONQ_LIBRARY} )
endif(LIBKONQ_FOUND)

install(TARGETS krusader ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS krusader.desktop
              krusader_root-mode.desktop
        DESTINATION ${XDG_APPS_INSTALL_DIR} )
install(FILES krusaderui.rc
              krusaderlisterui.rc
              krviewer.rc
              midnight_commander.color
              total_commander.color
              total_commander.keymap
              total_commander.keymap.info
              useraction_examples.xml
              layout.xml
              splash.png
        DESTINATION ${DATA_INSTALL_DIR}/krusader)

ecm_install_icons(ICONS
hi16-apps-krusader_blue.png
hi16-apps-krusader_red.png
hi16-apps-krusader_root.png
hi16-apps-krusader_user.png
hi22-apps-krusader_blue.png
hi22-apps-krusader_red.png
hi22-apps-krusader_root.png
hi22-apps-krusader_shield.png
hi22-apps-krusader_user.png
hi32-apps-krusader_blue.png
hi32-apps-krusader_red.png
hi32-apps-krusader_root.png
hi32-apps-krusader_shield.png
hi32-apps-krusader_user.png
hi48-apps-krusader_blue.png
hi48-apps-krusader_red.png
hi48-apps-krusader_root.png
hi48-apps-krusader_shield.png
hi48-apps-krusader_user.png
hi64-apps-krusader_blue.png
hi64-apps-krusader_red.png
hi64-apps-krusader_root.png
hi64-apps-krusader_shield.png
hi64-apps-krusader_user.png
hi128-apps-krusader_root.png
hi128-apps-krusader_user.png
DESTINATION ${ICON_INSTALL_DIR})
