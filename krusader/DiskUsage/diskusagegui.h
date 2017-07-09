/***************************************************************************
                        diskusagegui.h  -  description
                             -------------------
    copyright            : (C) 2004 by Csaba Karai
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

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
    ~DiskUsageGUI() {}
    void askDirAndShow();

protected slots:
    virtual void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

protected:
    virtual void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;

private slots:
    bool askDir();
    void slotLoadUsageInfo();
    void slotStatus(QString);

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

