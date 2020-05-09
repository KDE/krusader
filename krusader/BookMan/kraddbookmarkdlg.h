/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 * Copyright (C) 2004-2020 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#ifndef KRADDBOOKMARKDLG_H
#define KRADDBOOKMARKDLG_H

#include "krbookmark.h"
#include "../GUI/krtreewidget.h"

// QtCore
#include <QMap>
#include <QUrl>
// QtWidgets
#include <QDialog>
#include <QToolButton>

#include <KCompletion/KLineEdit>

class KrAddBookmarkDlg: public QDialog
{
    Q_OBJECT
public:
    explicit KrAddBookmarkDlg(QWidget *parent, const QUrl& url = QUrl());
    QUrl url() const {
        return QUrl::fromUserInput(_url->text(), QString(), QUrl::AssumeLocalFile);
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
    QPushButton *newFolderButton;
    QWidget *detailsWidget;
};

#endif // KRADDBOOKMARKDLG_H
