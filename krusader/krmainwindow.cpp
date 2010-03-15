#include "krmainwindow.h"

#include <QtGlobal>
#include <QAction>
#include <kactioncollection.h>


void KrMainWindow::enableAction(const char *name, bool enable)
{
    action(name)->setEnabled(enable);
}

QAction* KrMainWindow::action(const char *name)
{
    QAction *act = actions()->action(name);
    if(!act)
        qFatal("no such action: %s", name);
    return act;
}
