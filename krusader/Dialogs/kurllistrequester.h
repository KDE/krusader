/*****************************************************************************
 * Copyright (C) 2005 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2005-2018 Krusader Krew [https://krusader.org]              *
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

#ifndef KURLLISTREQUESTER_H
#define KURLLISTREQUESTER_H

// QtGui
#include <QKeyEvent>
// QtWidgets
#include <QWidget>
#include <QToolButton>

#include <KCompletion/KLineEdit>
#include <KIOWidgets/KUrlCompletion>

#include "../GUI/krlistwidget.h"

/**
 * Widget for letting the user define a list of URLs.
 */
class KURLListRequester : public QWidget
{
    Q_OBJECT

public:
    enum Mode { RequestFiles, RequestDirs };

    explicit KURLListRequester(Mode requestMode, QWidget *parent = nullptr);

    QList<QUrl> urlList();
    void setUrlList(const QList<QUrl> &);

    KLineEdit *lineEdit() { return urlLineEdit; }
    KrListWidget *listBox() { return urlListBox; }

    void setCompletionDir(const QUrl &dir) { completion.setDir(dir); }

signals:
    void checkValidity(QString &text, QString &error);
    void changed();

protected slots:
    void slotAdd();
    void slotBrowse();
    void slotRightClicked(QListWidgetItem *, const QPoint &);

protected:
    virtual void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    void deleteSelectedItems();

    Mode mode;

    KLineEdit *urlLineEdit;
    KrListWidget *urlListBox;
    QToolButton *urlAddBtn;
    QToolButton *urlBrowseBtn;

    KUrlCompletion completion;
};

#endif /* __KURLLISTREQUESTER_H__ */
