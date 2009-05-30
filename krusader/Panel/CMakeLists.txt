set(Panel_SRCS
    krcolorcache.cpp 
    krcalcspacedialog.cpp 
    krpopupmenu.cpp 
    krpreviewpopup.cpp 
    krview.cpp 
    krviewitem.cpp 
    panelfunc.cpp 
    listpanel.cpp 
    panelpopup.cpp 
    krselectionmode.cpp 
    krdetailedviewitem.cpp 
    krdetailedview.cpp 
    krbriefview.cpp 
    krbriefviewitem.cpp
    krvfsmodel.cpp
    krinterbriefview.cpp
    krinterdetailedview.cpp
    krinterviewitemdelegate.cpp
    krviewfactory.cpp
    krmousehandler.cpp)

kde4_add_library(Panel STATIC ${Panel_SRCS})

target_link_libraries(Panel Dialogs GUI KViewer ${KDE4_KFILE_LIBS} ${KDE4_KDE3SUPPORT_LIBRARY})
