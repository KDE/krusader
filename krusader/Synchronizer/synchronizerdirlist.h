/*****************************************************************************
 * Copyright (C) 2006 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2006-2018 Krusader Krew [https://krusader.org]              *
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

#ifndef SYNCHRONIZERDIRLIST_H
#define SYNCHRONIZERDIRLIST_H

// QtCore
#include <QHash>
#include <QObject>

#include <KIO/Job>

class FileItem;

class SynchronizerDirList : public QObject, public QHash<QString, FileItem *>
{
    Q_OBJECT

public:
    SynchronizerDirList(QWidget *w, bool ignoreHidden);
    ~SynchronizerDirList();

    FileItem *search(const QString &name, bool ignoreCase = false);
    FileItem *first();
    FileItem *next();

    inline const QUrl &url() { return currentUrl; }
    bool load(const QUrl &url, bool wait = false);

public slots:

    void slotEntries(KIO::Job *job, const KIO::UDSEntryList &entries);
    void slotListResult(KJob *job);

signals:
    void finished(bool err);

private:
    QHashIterator<QString, FileItem *> *fileIterator; //< Point to a dictionary of file items
    QWidget *parentWidget;
    bool     busy;
    bool     result;
    bool     ignoreHidden;
    QUrl     currentUrl;
};

#endif /* __SYNCHRONIZER_DIR_LIST_H__ */
