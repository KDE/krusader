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

kde4_add_library(VFS STATIC ${VFS_SRCS})
if(ACL_FOUND)
    target_link_libraries(VFS ${ACL_LIBS})
endif(ACL_FOUND)
