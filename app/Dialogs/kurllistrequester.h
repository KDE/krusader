/*
    SPDX-FileCopyrightText: 2005 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KURLLISTREQUESTER_H
#define KURLLISTREQUESTER_H

// QtGui
#include <QKeyEvent>
// QtWidgets
#include <QToolButton>
#include <QWidget>

#include <KLineEdit>
#include <KUrlCompletion>

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

    KLineEdit *lineEdit()
    {
        return urlLineEdit;
    }
    KrListWidget *listBox()
    {
        return urlListBox;
    }

    void setCompletionDir(const QUrl &dir)
    {
        completion.setDir(dir);
    }

signals:
    void checkValidity(QString &text, QString &error);
    void changed();

protected slots:
    void slotAdd();
    void slotBrowse();
    void slotRightClicked(QListWidgetItem *, const QPoint &);

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void deleteSelectedItems();

    Mode mode;

    KLineEdit *urlLineEdit;
    KrListWidget *urlListBox;
    QToolButton *urlAddBtn;
    QToolButton *urlBrowseBtn;

    KUrlCompletion completion;
};

#endif /* __KURLLISTREQUESTER_H__ */
