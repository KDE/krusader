/*
    SPDX-FileCopyrightText: 2005 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DISKUSAGEVIEWER_H
#define DISKUSAGEVIEWER_H

#include "../DiskUsage/diskusage.h"

// QtCore
#include <QUrl>
// QtWidgets
#include <QGridLayout>
#include <QLabel>
#include <QLayout>

class DiskUsageViewer : public QWidget
{
    Q_OBJECT

public:
    explicit DiskUsageViewer(QWidget *parent = nullptr);
    ~DiskUsageViewer() override;

    void openUrl(QUrl url);
    void closeUrl();
    void setStatusLabel(QLabel *statLabel, QString pref);

    inline DiskUsage *getWidget()
    {
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
