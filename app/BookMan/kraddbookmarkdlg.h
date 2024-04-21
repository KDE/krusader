/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRADDBOOKMARKDLG_H
#define KRADDBOOKMARKDLG_H

#include "../GUI/krtreewidget.h"
#include "krbookmark.h"

// QtCore
#include <QMap>
#include <QUrl>
// QtWidgets
#include <QDialog>
#include <QToolButton>

#include <KLineEdit>

class KrAddBookmarkDlg : public QDialog
{
    Q_OBJECT
public:
    explicit KrAddBookmarkDlg(QWidget *parent, const QUrl &url = QUrl());
    QUrl url() const
    {
        return QUrl::fromUserInput(_url->text(), QString(), QUrl::AssumeLocalFile);
    }
    QString name() const
    {
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
    QMap<QTreeWidgetItem *, KrBookmark *> _xr;
    QToolButton *_createInBtn;
    QPushButton *newFolderButton;
    QWidget *detailsWidget;
};

#endif // KRADDBOOKMARKDLG_H
