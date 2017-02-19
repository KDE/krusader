include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(FileSystem_SRCS
    vfilecontainer.cpp
    krvfshandler.cpp
    virt_vfs.cpp
    vfs.cpp
    vfile.cpp
    default_vfs.cpp
    krpermhandler.cpp
    krquery.cpp
    krtrashhandler.cpp
    ../../krArc/krlinecountingprocess.cpp
)

add_library(FileSystem STATIC ${FileSystem_SRCS})

target_link_libraries(FileSystem
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
)

if(ACL_FOUND)
    target_link_libraries(FileSystem ${ACL_LIBS})
endif(ACL_FOUND)
