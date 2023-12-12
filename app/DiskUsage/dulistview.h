/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DULISTVIEW_H
#define DULISTVIEW_H

// QtGui
#include <QKeyEvent>
#include <QMouseEvent>

#include "../GUI/krtreewidget.h"
#include "diskusage.h"

class DUListViewItem : public QTreeWidgetItem
{
public:
    DUListViewItem(DiskUsage *diskUsageIn,
                   File *fileIn,
                   QTreeWidget *parent,
                   const QString &label1,
                   const QString &label2,
                   const QString &label3,
                   const QString &label4,
                   const QString &label5,
                   const QString &label6,
                   const QString &label7,
                   const QString &label8,
                   const QString &label9)
        : QTreeWidgetItem(parent)
        , diskUsage(diskUsageIn)
        , file(fileIn)
    {
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
    DUListViewItem(DiskUsage *diskUsageIn,
                   File *fileIn,
                   QTreeWidgetItem *parent,
                   const QString &label1,
                   const QString &label2,
                   const QString &label3,
                   const QString &label4,
                   const QString &label5,
                   const QString &label6,
                   const QString &label7,
                   const QString &label8,
                   const QString &label9)
        : QTreeWidgetItem(parent)
        , diskUsage(diskUsageIn)
        , file(fileIn)
    {
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
    DUListViewItem(DiskUsage *diskUsageIn,
                   File *fileIn,
                   QTreeWidget *parent,
                   QTreeWidgetItem *after,
                   const QString &label1,
                   const QString &label2,
                   const QString &label3,
                   const QString &label4,
                   const QString &label5,
                   const QString &label6,
                   const QString &label7,
                   const QString &label8,
                   const QString &label9)
        : QTreeWidgetItem(parent, after)
        , diskUsage(diskUsageIn)
        , file(fileIn)
    {
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
    DUListViewItem(DiskUsage *diskUsageIn,
                   File *fileIn,
                   QTreeWidgetItem *parent,
                   QTreeWidgetItem *after,
                   const QString &label1,
                   const QString &label2,
                   const QString &label3,
                   const QString &label4,
                   const QString &label5,
                   const QString &label6,
                   const QString &label7,
                   const QString &label8,
                   const QString &label9)
        : QTreeWidgetItem(parent, after)
        , diskUsage(diskUsageIn)
        , file(fileIn)
    {
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
    ~DUListViewItem() override
    {
        diskUsage->removeProperty(file, "ListView-Ref");
    }

    bool operator<(const QTreeWidgetItem &other) const override
    {
        int column = treeWidget() ? treeWidget()->sortColumn() : 0;

        if (text(0) == "..")
            return true;

        const auto *compWith = dynamic_cast<const DUListViewItem *>(&other);
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

    inline File *getFile()
    {
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

    File *getCurrentFile();

public slots:
    void slotDirChanged(Directory *);
    void slotChanged(File *);
    void slotDeleted(File *);
    void slotRightClicked(QTreeWidgetItem *, const QPoint &);
    void slotExpanded(QTreeWidgetItem *);

protected:
    DiskUsage *diskUsage;

    void mouseDoubleClickEvent(QMouseEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;

private:
    void addDirectory(Directory *dirEntry, QTreeWidgetItem *parent);
    bool doubleClicked(QTreeWidgetItem *item);
};

#endif /* __DU_LISTVIEW_H__ */
