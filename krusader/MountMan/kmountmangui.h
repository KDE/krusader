/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2000 Rafi Yanai <krusader@users.sourceforge.net>            *
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


#ifndef KMOUNTMANGUI_H
#define KMOUNTMANGUI_H

// QtCore
#include <QTimer>
#include <QList>
#include <QDateTime>
// QtWidgets
#include <QDialog>
#include <QFrame>

#include <KIOCore/KMountPoint>

#include "../GUI/krtreewidget.h"
#include "kmountman.h"

#define  WATCHER_DELAY    500

class QCheckBox;
class KRFSDisplay;

// forward definitions
class fsData;

class KMountManGUI : public QDialog
{
    Q_OBJECT

    enum Pages {
        Filesystems = 0
    };

public:
    explicit KMountManGUI(KMountMan *mntMan);
    ~KMountManGUI();

protected:
    virtual void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;

protected slots:
    void doubleClicked(QTreeWidgetItem *);
    void clicked(QTreeWidgetItem *, const QPoint &);
    void slotToggleMount();
    void slotEject();
    void changeActive();
    void changeActive(QTreeWidgetItem *);
    void checkMountChange(); // check whether the mount table was changed

    void updateList();     // fill-up the filesystems list
    void getSpaceData();

protected:
    QLayout *createMainPage(); // creator of the main page - filesystems
    void addItemToMountList(KrTreeWidget *lst, fsData &fs);
    fsData* getFsData(QTreeWidgetItem *item);
    QString getMntPoint(QTreeWidgetItem *item);
    void addNonMounted();

private:
    KMountMan *mountMan;
    KRFSDisplay *info;
    KrTreeWidget *mountList;
    QCheckBox *cbShowOnlyRemovable;
    QPushButton *mountButton;
    QPushButton *ejectButton;
    QTimer *watcher;
    QDateTime lastMtab;
    // used for the getSpace - gotSpace functions
    KMountPoint::List possible, mounted;
    QList<fsData> fileSystems;

    int sizeX;
    int sizeY;
};

// Data container for a single-filesystem data
// maximum size supported is 2GB of 1kb blocks == 2048GB, enough.
// not really needed, but kept for backward compatibility
class fsData
{
public:
    fsData() : Name(), Type(), MntPoint(), TotalBlks(0),
            FreeBlks(0), Mounted(false) {}

    // get information
    inline QString name() {
        return Name;
    }
    inline QString shortName() {
        return Name.right(Name.length() - Name.indexOf("/", 1) - 1);
    }
    inline QString type() {
        return Type;
    }
    inline QString mntPoint() {
        return MntPoint;
    }
    inline long totalBlks() {
        return TotalBlks;
    }
    inline long freeBlks() {
        return FreeBlks;
    }
    inline KIO::filesize_t totalBytes() {
        return TotalBlks * 1024;
    }
    inline KIO::filesize_t freeBytes() {
        return FreeBlks * 1024;
    }
    //////////////////// insert a good round function here /////////////////
    int usedPerct() {
        if (TotalBlks == 0)
            return 0;
        float res = ((float)(TotalBlks - FreeBlks)) / ((float) TotalBlks) * 100;
        if ((res - (int) res) > 0.5)
            return (int) res + 1;
        else
            return (int) res;
    }
    inline bool mounted() {
        return Mounted;
    }

    // set information
    inline void setName(QString n_) {
        Name = n_;
    }
    inline void setType(QString t_) {
        Type = t_;
    }
    inline void setMntPoint(QString m_) {
        MntPoint = m_;
    }
    inline void setTotalBlks(long t_) {
        TotalBlks = t_;
    }
    inline void setFreeBlks(long f_) {
        FreeBlks = f_;
    }
    inline void setMounted(bool m_) {
        Mounted = m_;
    }

private:
    QString Name;       // i.e: /dev/cdrom
    QString Type;       // i.e: iso9600
    QString MntPoint;   // i.e: /mnt/cdrom
    long TotalBlks;  // measured in 1024bytes per block
    long FreeBlks;
    bool Mounted;    // true if filesystem is mounted

    // additional attributes of a filesystem, parsed from fstab
public:
    QString options;    // additional fstab options
};

class KrMountDetector
{
    QString checksum;
#ifndef BSD
    QDateTime lastMtab;
#endif
public:
    KrMountDetector();
    static KrMountDetector * getInstance();
    bool hasMountsChanged();
};



#endif
