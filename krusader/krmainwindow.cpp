#include "krmainwindow.h"

#include <QtGlobal>
#include <QAction>
#include <kactioncollection.h>

#include <stdio.h>


void KrMainWindow::enableAction(const char *name, bool enable)
{
    QAction *act = actions()->action(name);
    if(act)
        act->setEnabled(enable);
    else
        fprintf(stderr, "no such action: \"%s\"\n", name);
}

QAction* KrMainWindow::action(const char *name)
{
    QAction *act = actions()->action(name);
    if(!act)
        qFatal("no such action: \"%s\"", name);
    return act;
}
