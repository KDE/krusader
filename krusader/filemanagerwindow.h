/*****************************************************************************
 * Copyright (C) 2010 Jan Lepper <dehtris@yahoo.de>                          *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef __FILEMANAGERWINDOW_H__
#define __FILEMANAGERWINDOW_H__

#include "krmainwindow.h"
#include "abstractpanelmanager.h"

class ListPanelActions;
class KrPanel;

class FileManagerWindow : public KrMainWindow
{
public:
    virtual AbstractPanelManager *activeManager() = 0;
    virtual AbstractPanelManager *leftManager() = 0;
    virtual AbstractPanelManager *rightManager() = 0;
    virtual ListPanelActions *listPanelActions() = 0;
    virtual void plugActionList(const char *name, QList<QAction*> &list) = 0;

    KrPanel *activePanel() {
        return activeManager()->currentPanel();
    }
    KrPanel *leftPanel() {
        return leftManager()->currentPanel();
    }
    KrPanel *rightPanel() {
        return rightManager()->currentPanel();
    }
};

#endif // __FILEMANAGERWINDOW_H__
