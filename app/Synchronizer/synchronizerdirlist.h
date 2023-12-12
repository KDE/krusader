/*
    SPDX-FileCopyrightText: 2006 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SYNCHRONIZERDIRLIST_H
#define SYNCHRONIZERDIRLIST_H

// QtCore
#include <QHash>
#include <QObject>

#include <KIO/Job>

#include "../FileSystem/fileitem.h"

class SynchronizerDirList : public QObject, public QHash<QString, FileItem *>
{
    Q_OBJECT

public:
    SynchronizerDirList(QWidget *w, bool ignoreHidden);
    ~SynchronizerDirList() override;

    FileItem *search(const QString &name, bool ignoreCase = false);
    FileItem *first();
    FileItem *next();

    inline const QString &url()
    {
        return currentUrl;
    }
    bool load(const QString &urlIn, bool wait = false);

public slots:

    void slotEntries(KIO::Job *job, const KIO::UDSEntryList &entries);
    void slotListResult(KJob *job);

signals:
    void finished(bool err);

private:
    QHashIterator<QString, FileItem *> *fileIterator; //< Point to a dictionary of file items
    QWidget *parentWidget;
    bool busy;
    bool result;
    bool ignoreHidden;
    QString currentUrl;
};

#endif /* __SYNCHRONIZER_DIR_LIST_H__ */
