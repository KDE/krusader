include_directories(${KF5_INCLUDES_DIRS} ${QT_INCLUDES})

set(FileSystem_SRCS
    defaultfilesystem.cpp
    dirlisterinterface.cpp
    fileitem.cpp
    filesystem.cpp
    filesystemprovider.cpp
    krpermhandler.cpp
    krquery.cpp
    krtrashhandler.cpp
    sizecalculator.cpp
    virtualfilesystem.cpp
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
