/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
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

#ifndef KRADDBOOKMARKDLG_H
#define KRADDBOOKMARKDLG_H

#include "krbookmark.h"
#include "../VFS/vfs.h"
#include "../GUI/krtreewidget.h"
#include <kdialog.h>
#include <kurl.h>
#include <klineedit.h>
#include <qmap.h>
#include <QtGui/QToolButton>

class KrAddBookmarkDlg: public KDialog
{
    Q_OBJECT
public:
    KrAddBookmarkDlg(QWidget *parent, KUrl url = KUrl());
    KUrl url() const {
        return KUrl(_url->text());
    }
    QString name() const {
        return _name->text();
    }
    KrBookmark *folder() const;

protected:
    QWidget *createInWidget();
    void populateCreateInWidget(KrBookmark *root, QTreeWidgetItem *parent);

protected slots:
    void toggleCreateIn(bool show);
    void slotSelectionChanged();
    void newFolder();

private:
    KLineEdit *_name;
    KLineEdit *_url;
    KLineEdit *_folder;
    KrTreeWidget *_createIn;
    QMap<QTreeWidgetItem*, KrBookmark*> _xr;
    QToolButton *_createInBtn;
};

#endif // KRADDBOOKMARKDLG_H
