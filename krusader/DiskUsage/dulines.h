/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2020 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DULINES_H
#define DULINES_H

// QtGui
#include <QMouseEvent>
#include <QPixmap>
#include <QKeyEvent>
#include <QResizeEvent>

#include "diskusage.h"
#include "../GUI/krtreewidget.h"

class DULinesToolTip;
class DULinesItemDelegate;

class DULines : public KrTreeWidget
{
    Q_OBJECT

public:
    explicit DULines(DiskUsage *usage);
    ~DULines() override;

    File * getCurrentFile();

public slots:
    void slotDirChanged(Directory *dirEntry);
    void sectionResized(int);
    void slotRightClicked(QTreeWidgetItem *, const QPoint &);
    void slotChanged(File *);
    void slotDeleted(File *);
    void slotShowFileSizes();
    void slotRefresh();

protected:
    DiskUsage *diskUsage;

    bool event(QEvent * event) override;
    void mouseDoubleClickEvent(QMouseEvent * e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void resizeEvent(QResizeEvent *) override;

private:
    QPixmap createPixmap(int percent, int maxPercent, int maxWidth);

    bool doubleClicked(QTreeWidgetItem * item);

    bool refreshNeeded;
    bool started;

    bool showFileSize;

    DULinesToolTip *toolTip;
    DULinesItemDelegate *itemDelegate;
};

#endif /* __DU_LINES_H__ */

