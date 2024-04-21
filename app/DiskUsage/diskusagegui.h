/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DISKUSAGEGUI_H
#define DISKUSAGEGUI_H

// QtCore
#include <QUrl>
// QtGui
#include <QResizeEvent>
// QtWidgets
#include <QDialog>
#include <QLayout>
#include <QToolButton>

#include <KSqueezedTextLabel>

#include "diskusage.h"

class DiskUsageGUI : public QDialog
{
    Q_OBJECT

public:
    explicit DiskUsageGUI(const QUrl &openDir);
    ~DiskUsageGUI() override = default;
    void askDirAndShow();

protected slots:
    void closeEvent(QCloseEvent *event) override;

protected:
    void resizeEvent(QResizeEvent *e) override;

private slots:
    bool askDir();
    void slotLoadUsageInfo();
    void slotStatus(const QString &);

    void slotSelectLinesView()
    {
        diskUsage->setView(VIEW_LINES);
    }
    void slotSelectListView()
    {
        diskUsage->setView(VIEW_DETAILED);
    }
    void slotSelectFilelightView()
    {
        diskUsage->setView(VIEW_FILELIGHT);
    }

    void slotViewChanged(int view);
    void slotLoadFinished(bool);

private:
    void enableButtons(bool);

    DiskUsage *diskUsage;
    QUrl baseDirectory;

    KSqueezedTextLabel *status;

    QToolButton *btnNewSearch;
    QToolButton *btnRefresh;
    QToolButton *btnDirUp;

    QToolButton *btnLines;
    QToolButton *btnDetailed;
    QToolButton *btnFilelight;

    int sizeX;
    int sizeY;

    bool exitAtFailure;
};

#endif /* __DISK_USAGE_GUI_H__ */
