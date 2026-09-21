/*
    SPDX-FileCopyrightText: Krusader Krew <https://krusader.org>
    SPDX-FileCopyrightText: Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: Rafi Yanai <yanai@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRBOOKMARKBUTTON_H
#define KRBOOKMARKBUTTON_H

// QtWidgets
#include <QToolButton>

#include <KActionMenu>

class KrBookmarkButton : public QToolButton
{
    Q_OBJECT
public:
    explicit KrBookmarkButton(QWidget *parent);
    void showMenu();

signals:
    void openUrl(const QUrl &url);
    void aboutToShow();

protected slots:
    void populate();

private:
    KActionMenu *acmBookmarks;
};

#endif // KRBOOKMARK_BUTTON_H
