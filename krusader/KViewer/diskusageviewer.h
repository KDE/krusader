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

#ifndef DISKUSAGEVIEWER_H
#define DISKUSAGEVIEWER_H

#include "../DiskUsage/diskusage.h"

// QtCore
#include <QUrl>
// QtWidgets
#include <QLayout>
#include <QLabel>
#include <QGridLayout>

class DiskUsageViewer : public QWidget
{
    Q_OBJECT

public:
    explicit DiskUsageViewer(QWidget *parent = nullptr);
    ~DiskUsageViewer();

    void openUrl(QUrl url);
    void closeUrl();
    void setStatusLabel(QLabel *statLabel, QString pref);

    inline DiskUsage * getWidget() {
        return diskUsage;
    }

signals:
    void openUrlRequest(const QUrl &);

protected slots:
    void slotUpdateStatus(QString status = QString());
    void slotNewSearch();

protected:
    DiskUsage *diskUsage;
    QGridLayout *layout;

    QLabel *statusLabel;
    QString prefix;
};

#endif /* DISKUSAGEVIEWER_H */
