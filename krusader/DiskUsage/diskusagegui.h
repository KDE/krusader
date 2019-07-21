/*****************************************************************************
 * Copyright (C) 2004 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2004-2019 Krusader Krew [https://krusader.org]              *
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

#include <KWidgetsAddons/KSqueezedTextLabel>

#include "diskusage.h"

class DiskUsageGUI : public QDialog
{
    Q_OBJECT

public:
    explicit DiskUsageGUI(const QUrl &openDir);
    ~DiskUsageGUI() override = default;
    void askDirAndShow();

protected slots:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

protected:
    void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;

private slots:
    bool askDir();
    void slotLoadUsageInfo();
    void slotStatus(const QString&);

    void slotSelectLinesView() { diskUsage->setView(VIEW_LINES); }
    void slotSelectListView() { diskUsage->setView(VIEW_DETAILED); }
    void slotSelectFilelightView() { diskUsage->setView(VIEW_FILELIGHT); }

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

