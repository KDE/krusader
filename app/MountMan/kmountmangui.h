/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KMOUNTMANGUI_H
#define KMOUNTMANGUI_H

// QtCore
#include <QDateTime>
#include <QList>
#include <QTimer>
// QtWidgets
#include <QDialog>
#include <QFrame>

#include <KMountPoint>
#include <utility>

#include "../GUI/krtreewidget.h"
#include "kmountman.h"

#include <math.h>

#define WATCHER_DELAY 500

class QCheckBox;
class KrFSDisplay;

// forward definitions
class fsData;

class KMountManGUI : public QDialog
{
    Q_OBJECT

    enum Pages { Filesystems = 0 };

public:
    explicit KMountManGUI(KMountMan *mntMan);
    ~KMountManGUI() override;

protected:
    void resizeEvent(QResizeEvent *e) override;

protected slots:
    void doubleClicked(QTreeWidgetItem *);
    void clicked(QTreeWidgetItem *, const QPoint &);
    void slotToggleMount();
    void slotEject();
    void changeActive();
    void changeActive(QTreeWidgetItem *);
    void checkMountChange(); // check whether the mount table was changed

    void updateList(); // fill-up the filesystems list
    void getSpaceData();

protected:
    QLayout *createMainPage(); // creator of the main page - filesystems
    void addItemToMountList(KrTreeWidget *lst, fsData &fs, const QList<Solid::Device> &blockDevices) const;
    fsData *getFsData(QTreeWidgetItem *item);
    QString getMntPoint(QTreeWidgetItem *item);
    void addNonMounted();

private:
    void freeSpaceResult(KJob *job, fsData data);


    KMountMan *mountMan;
    KrFSDisplay *info;
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
    fsData()
        : Name()
        , Type()
        , MntPoint()
        , TotalBlks(0)
        , FreeBlks(0)
        , Mounted(false)
    {
    }

    // get information
    inline QString name()
    {
        return Name;
    }
    inline QString shortName()
    {
        return Name.right(Name.length() - Name.indexOf("/", 1) - 1);
    }
    inline QString type()
    {
        return Type;
    }
    inline QString mntPoint()
    {
        return MntPoint;
    }
    inline unsigned long totalBlks()
    {
        return TotalBlks;
    }
    inline unsigned long freeBlks()
    {
        return FreeBlks;
    }
    inline KIO::filesize_t totalBytes()
    {
        return TotalBlks * 1024;
    }
    inline KIO::filesize_t freeBytes()
    {
        return FreeBlks * 1024;
    }
    int usedPerct()
    {
        if (TotalBlks == 0)
            return 0;

        return static_cast<int>(roundl((static_cast<long double>(TotalBlks - FreeBlks) * 100) / TotalBlks));
    }
    inline bool mounted()
    {
        return Mounted;
    }

    // set information
    inline void setName(QString n_)
    {
        Name = std::move(n_);
    }
    inline void setType(QString t_)
    {
        Type = std::move(t_);
    }
    inline void setMntPoint(QString m_)
    {
        MntPoint = std::move(m_);
    }
    inline void setTotalBlks(unsigned long t_)
    {
        TotalBlks = t_;
    }
    inline void setFreeBlks(unsigned long f_)
    {
        FreeBlks = f_;
    }
    inline void setMounted(bool m_)
    {
        Mounted = m_;
    }

private:
    QString Name; // i.e: /dev/cdrom
    QString Type; // i.e: iso9600
    QString MntPoint; // i.e: /mnt/cdrom
    unsigned long TotalBlks; // measured in 1024 bytes per block
    unsigned long FreeBlks;
    bool Mounted; // true if filesystem is mounted

    // additional attributes of a filesystem, parsed from fstab
public:
    QString options; // additional fstab options
};

class KrMountDetector
{
    QString checksum;
#ifndef BSD
    QDateTime lastMtab;
#endif
public:
    KrMountDetector();
    static KrMountDetector *getInstance();
    bool hasMountsChanged();
};

#endif
