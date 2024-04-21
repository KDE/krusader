/*
    SPDX-FileCopyrightText: 2009 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2009-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRTRASHHANDLER_H
#define KRTRASHHANDLER_H

// QtCore
#include <QString>
#include <QUrl>

#include <KDirWatch>

class KrTrashWatcher;

class KrTrashHandler
{
public:
    static bool isTrashEmpty();
    static QString trashIconName();
    static void emptyTrash();
    static void restoreTrashedFiles(const QList<QUrl> &url);
    static void startWatcher();
    static void stopWatcher();

private:
    static KrTrashWatcher *_trashWatcher;
};

/** Watches the trashrc config file for changes and updates the trash icon. */
class KrTrashWatcher : public QObject
{
    Q_OBJECT

public:
    KrTrashWatcher();
    ~KrTrashWatcher() override;

public slots:
    void slotTrashChanged();

private:
    KDirWatch *_watcher;
};

#endif /* __KR_TRASH_HANDLER__ */
