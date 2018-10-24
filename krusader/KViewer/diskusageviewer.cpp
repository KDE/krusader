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

#include "diskusageviewer.h"

// QtWidgets
#include <QGridLayout>
#include <QLabel>

#include <KConfigCore/KSharedConfig>

#include "../FileSystem/filesystem.h"
#include "../Panel/krpanel.h"
#include "../Panel/panelfunc.h"
#include "../krglobal.h"

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

void DiskUsageViewer::openUrl(QUrl url)
{
    if (diskUsage == 0) {
        diskUsage = new DiskUsage("DiskUsageViewer", this);

        connect(diskUsage, &DiskUsage::enteringDirectory, this, [=]() { slotUpdateStatus(); });
        connect(diskUsage, &DiskUsage::status, this, &DiskUsageViewer::slotUpdateStatus);
        connect(diskUsage, &DiskUsage::newSearch, this, &DiskUsageViewer::slotNewSearch);
        layout->addWidget(diskUsage, 0, 0);
        this->show();
        diskUsage->show();

        KConfigGroup group(krConfig, "DiskUsageViewer");
        int view = group.readEntry("View",  VIEW_FILELIGHT);
        if (view < VIEW_LINES || view > VIEW_FILELIGHT)
            view = VIEW_FILELIGHT;
        diskUsage->setView(view);
    }

    url.setPath(url.adjusted(QUrl::StripTrailingSlash).path());

    QUrl baseURL = diskUsage->getBaseURL();
    if (!diskUsage->isLoading() && !baseURL.isEmpty()) {
        if (url.scheme() == baseURL.scheme() && (url.host().isEmpty() || url.host() == baseURL.host())) {
            QString baseStr = FileSystem::ensureTrailingSlash(baseURL).path();
            QString urlStr = FileSystem::ensureTrailingSlash(url).path();

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
    diskUsage->load(ACTIVE_PANEL->virtualPath());
}

