/***************************************************************************
                        diskusageviewer.cpp  -  description
                             -------------------
    copyright            : (C) 2005 by Csaba Karai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "diskusageviewer.h"

#include <QGridLayout>
#include <QLabel>

#include "../krglobal.h"
#include "../Panel/krpanel.h"
#include "../Panel/panelfunc.h"

DiskUsageViewer::DiskUsageViewer(QWidget *parent)
        : QWidget(parent), diskUsage(0), statusLabel(0)
{
    layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
}

DiskUsageViewer::~ DiskUsageViewer()
{
    if (diskUsage) {
        KConfigGroup group(krConfig, "DiskUsageViewer");
        group.writeEntry("View", diskUsage->getActiveView());
        delete diskUsage;
    }
}

void DiskUsageViewer::openUrl(KUrl url)
{
    if (diskUsage == 0) {
        diskUsage = new DiskUsage("DiskUsageViewer", this);

        connect(diskUsage, SIGNAL(enteringDirectory(Directory *)), this, SLOT(slotUpdateStatus()));
        connect(diskUsage, SIGNAL(status(QString)), this, SLOT(slotUpdateStatus(QString)));
        connect(diskUsage, SIGNAL(newSearch()), this, SLOT(slotNewSearch()));
        layout->addWidget(diskUsage, 0, 0);
        this->show();
        diskUsage->show();

        KConfigGroup group(krConfig, "DiskUsageViewer");
        int view = group.readEntry("View",  VIEW_FILELIGHT);
        if (view < VIEW_LINES || view > VIEW_FILELIGHT)
            view = VIEW_FILELIGHT;
        diskUsage->setView(view);
    }

    url.setPath(url.path(KUrl::RemoveTrailingSlash));

    KUrl baseURL = diskUsage->getBaseURL();
    if (!diskUsage->isLoading() && !baseURL.isEmpty()) {
        if (url.protocol() == baseURL.protocol() && (!url.hasHost() || url.host() == baseURL.host())) {
            QString baseStr = baseURL.path(KUrl::AddTrailingSlash), urlStr = url.path(KUrl::AddTrailingSlash);

            if (urlStr.startsWith(baseStr)) {
                QString relURL = urlStr.mid(baseStr.length());
                if (relURL.endsWith('/'))
                    relURL.truncate(relURL.length() - 1);

                Directory *dir = diskUsage->getDirectory(relURL);
                if (dir) {
                    diskUsage->changeDirectory(dir);
                    return;
                }
            }
        }
    }
    diskUsage->load(url);
}

void DiskUsageViewer::closeUrl()
{
    if (diskUsage)
        diskUsage->close();
}

void DiskUsageViewer::setStatusLabel(QLabel *statLabel, QString pref)
{
    statusLabel = statLabel;
    prefix = pref;
}

void DiskUsageViewer::slotUpdateStatus(QString status)
{
    if (statusLabel) {
        if (status.isEmpty()) {
            Directory * dir = diskUsage->getCurrentDir();
            if (dir)
                status = prefix + dir->name() + "  [" + KIO::convertSize(dir->size()) + ']';
        }
        statusLabel->setText(status);
    }
}

void DiskUsageViewer::slotNewSearch()
{
    diskUsage->load(ACTIVE_PANEL->func->files()->vfs_getOrigin());
}

#include "diskusageviewer.moc"
