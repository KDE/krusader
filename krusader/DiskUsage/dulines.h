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
    ~DULines();

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

    virtual bool event(QEvent * event) Q_DECL_OVERRIDE;
    virtual void mouseDoubleClickEvent(QMouseEvent * e) Q_DECL_OVERRIDE;
    virtual void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;

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

