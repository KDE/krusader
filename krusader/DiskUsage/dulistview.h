/*****************************************************************************
 * Copyright (C) 2004 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
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

#ifndef DULISTVIEW_H
#define DULISTVIEW_H

// QtGui
#include <QMouseEvent>
#include <QKeyEvent>

#include "../GUI/krtreewidget.h"
#include "diskusage.h"

class DUListViewItem : public QTreeWidgetItem
{
public:
    DUListViewItem(DiskUsage *diskUsageIn, File *fileIn, QTreeWidget * parent, QString label1,
                   QString label2, QString label3, QString label4, QString label5, QString label6,
                   QString label7, QString label8, QString label9)
            : QTreeWidgetItem(parent), diskUsage(diskUsageIn), file(fileIn) {
        setText(0, label1);
        setText(1, label2);
        setText(2, label3);
        setText(3, label4);
        setText(4, label5);
        setText(5, label6);
        setText(6, label7);
        setText(7, label8);
        setText(8, label9);

        setTextAlignment(1, Qt::AlignRight);
        setTextAlignment(2, Qt::AlignRight);
        setTextAlignment(3, Qt::AlignRight);

        setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicatorWhenChildless);
        diskUsage->addProperty(file, "ListView-Ref", this);
    }
    DUListViewItem(DiskUsage *diskUsageIn, File *fileIn, QTreeWidgetItem * parent, QString label1,
                   QString label2, QString label3, QString label4, QString label5, QString label6,
                   QString label7, QString label8, QString label9)
            : QTreeWidgetItem(parent), diskUsage(diskUsageIn), file(fileIn) {
        setText(0, label1);
        setText(1, label2);
        setText(2, label3);
        setText(3, label4);
        setText(4, label5);
        setText(5, label6);
        setText(6, label7);
        setText(7, label8);
        setText(8, label9);

        setTextAlignment(1, Qt::AlignRight);
        setTextAlignment(2, Qt::AlignRight);
        setTextAlignment(3, Qt::AlignRight);

        setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicatorWhenChildless);
        diskUsage->addProperty(file, "ListView-Ref", this);
    }
    DUListViewItem(DiskUsage *diskUsageIn, File *fileIn, QTreeWidget * parent, QTreeWidgetItem * after,
                   QString label1, QString label2, QString label3, QString label4, QString label5,
                   QString label6, QString label7, QString label8, QString label9)
            : QTreeWidgetItem(parent, after), diskUsage(diskUsageIn), file(fileIn) {
        setText(0, label1);
        setText(1, label2);
        setText(2, label3);
        setText(3, label4);
        setText(4, label5);
        setText(5, label6);
        setText(6, label7);
        setText(7, label8);
        setText(8, label9);

        setTextAlignment(1, Qt::AlignRight);
        setTextAlignment(2, Qt::AlignRight);
        setTextAlignment(3, Qt::AlignRight);

        setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicatorWhenChildless);
        diskUsage->addProperty(file, "ListView-Ref", this);
    }
    DUListViewItem(DiskUsage *diskUsageIn, File *fileIn, QTreeWidgetItem * parent, QTreeWidgetItem * after,
                   QString label1, QString label2, QString label3, QString label4, QString label5,
                   QString label6, QString label7, QString label8, QString label9)
            : QTreeWidgetItem(parent, after),
            diskUsage(diskUsageIn), file(fileIn) {
        setText(0, label1);
        setText(1, label2);
        setText(2, label3);
        setText(3, label4);
        setText(4, label5);
        setText(5, label6);
        setText(6, label7);
        setText(7, label8);
        setText(8, label9);

        setTextAlignment(1, Qt::AlignRight);
        setTextAlignment(2, Qt::AlignRight);
        setTextAlignment(3, Qt::AlignRight);

        setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicatorWhenChildless);
        diskUsage->addProperty(file, "ListView-Ref", this);
    }
    ~DUListViewItem() override {
        diskUsage->removeProperty(file, "ListView-Ref");
    }

    bool operator<(const QTreeWidgetItem &other) const Q_DECL_OVERRIDE {
        int column = treeWidget() ? treeWidget()->sortColumn() : 0;

        if (text(0) == "..")
            return true;

        const auto *compWith = dynamic_cast< const DUListViewItem * >(&other);
        if (compWith == nullptr)
            return false;

        switch (column) {
        case 1:
        case 2:
            return file->size() > compWith->file->size();
        case 3:
            return file->ownSize() > compWith->file->ownSize();
        case 5:
            return file->time() < compWith->file->time();
        default:
            return text(column) < other.text(column);
        }
    }

    inline File * getFile() {
        return file;
    }

private:
    DiskUsage *diskUsage;
    File *file;
};

class DUListView : public KrTreeWidget
{
    Q_OBJECT

public:
    explicit DUListView(DiskUsage *usage);
    ~DUListView() override;

    File * getCurrentFile();

public slots:
    void slotDirChanged(Directory *);
    void slotChanged(File *);
    void slotDeleted(File *);
    void slotRightClicked(QTreeWidgetItem *, const QPoint &);
    void slotExpanded(QTreeWidgetItem *);

protected:
    DiskUsage *diskUsage;

    void mouseDoubleClickEvent(QMouseEvent * e) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;

private:
    void addDirectory(Directory *dirEntry, QTreeWidgetItem *parent);
    bool doubleClicked(QTreeWidgetItem * item);
};

#endif /* __DU_LISTVIEW_H__ */

