include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(VFS_SRCS
    vfilecontainer.cpp
    krvfshandler.cpp
    virt_vfs.cpp 
    vfs.cpp 
    vfile.cpp 
    normal_vfs.cpp 
    krpermhandler.cpp 
    krarchandler.cpp 
    preservingcopyjob.cpp 
    virtualcopyjob.cpp 
    ftp_vfs.cpp 
    krquery.cpp
    kiojobwrapper.cpp
    packjob.cpp
    abstractthreadedjob.cpp)

add_library(VFS STATIC ${VFS_SRCS})

if(ACL_FOUND)
    target_link_libraries(VFS
                          KF5::Archive
                          KF5::I18n
                          KF5::IconThemes
                          KF5::KIOCore
                          KF5::KIOWidgets
                          KF5::Parts
                          KF5::Solid
                          KF5::Wallet
                          KF5::WidgetsAddons
                          KF5::WindowSystem
                          ${ACL_LIBS}
                          )
else(ACL_NOT_FOUND)
    target_link_libraries(VFS
                          KF5::Archive
                          KF5::I18n
                          KF5::IconThemes
                          KF5::KIOCore
                          KF5::KIOWidgets
                          KF5::Parts
                          KF5::Solid
                          KF5::Wallet
                          KF5::WidgetsAddons
                          KF5::WindowSystem
                          )
endif(ACL_FOUND)
