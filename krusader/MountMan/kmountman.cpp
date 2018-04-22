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

#include "kmountman.h"

// QtCore
#include <QDir>
// QtWidgets
#include <QApplication>
#include <QMenu>

#include <KConfigCore/KSharedConfig>
#include <KCoreAddons/KJobTrackerInterface>
#include <KCoreAddons/KProcess>
#include <KI18n/KLocalizedString>
#include <KIO/JobUiDelegate>
#include <KWidgetsAddons/KMessageBox>
#include <KWidgetsAddons/KToolBarPopupAction>

#include <Solid/Block>
#include <Solid/OpticalDisc>
#include <Solid/OpticalDrive>
#include <Solid/StorageAccess>
#include <Solid/StorageVolume>

#include "../krglobal.h"
#include "../icon.h"
#include "../kractions.h"
#include "../defaults.h"
#include "../Dialogs/krdialogs.h"
#include "../krservices.h"
#include "kmountmangui.h"
#include "../FileSystem/krpermhandler.h"

#ifdef _OS_SOLARIS_
#define FSTAB "/etc/filesystemtab"
#else
#define FSTAB "/etc/fstab"
#endif

KMountMan::KMountMan(QWidget *parent) : QObject(), _operational(false), waiting(false), mountManGui(0), parentWindow(parent)
{
    _action = new KToolBarPopupAction(Icon("kr_mountman"), i18n("&MountMan..."), this);
    connect(_action, &QAction::triggered, this, &KMountMan::mainWindow);
    connect(_action->menu(), &QMenu::aboutToShow, this, &KMountMan::quickList);
    _manageAction = _action->menu()->addAction(i18n("Open &MountMan"));
    connect(_manageAction, &QAction::triggered, this, &KMountMan::mainWindow);
    _action->menu()->addSeparator();

    // added as a precaution, although we use kde services now
    _operational = KrServices::cmdExist("mount");

    network_fs << "nfs" << "smbfs" << "fuse.fusesmb" << "fuse.sshfs"; //TODO: is this list complete ?

    // list of FS that we don't manage at all
    invalid_fs << "swap" << "/dev/pts" << "tmpfs" << "devpts" << "sysfs" << "rpc_pipefs" << "usbfs" << "binfmt_misc";
#ifdef BSD
    invalid_fs << "procfs";
#else
    invalid_fs << "proc";
#endif

    // list of FS that we don't allow to mount/unmount
    nonmount_fs << "supermount";
    {
        KConfigGroup group(krConfig, "Advanced");
        QStringList nonmount = group.readEntry("Nonmount Points", _NonMountPoints).split(',');
        nonmount_fs_mntpoint += nonmount;
        // simplify the white space
        for (QStringList::Iterator it = nonmount_fs_mntpoint.begin(); it != nonmount_fs_mntpoint.end(); ++it) {
            *it = (*it).simplified();
        }
    }

}

KMountMan::~KMountMan() {}

bool KMountMan::invalidFilesystem(QString type)
{
    return (invalid_fs.contains(type) > 0);
}

// this is an ugly hack, but type can actually be a mountpoint. oh well...
bool KMountMan::nonmountFilesystem(QString type, QString mntPoint)
{
    return((nonmount_fs.contains(type) > 0) || (nonmount_fs_mntpoint.contains(mntPoint) > 0));
}

bool KMountMan::networkFilesystem(QString type)
{
    return (network_fs.contains(type) > 0);
}

void KMountMan::mainWindow()
{
    // left as a precaution, although we use kde's services now
    if (!KrServices::cmdExist("mount")) {
        KMessageBox::error(0, i18n("Cannot start 'mount'. Check the 'Dependencies' page in konfigurator."));
        return;
    }

    mountManGui = new KMountManGUI(this);
    delete mountManGui;   /* as KMountManGUI is modal, we can now delete it */
    mountManGui = nullptr; /* for sanity */
}

QExplicitlySharedDataPointer<KMountPoint> KMountMan::findInListByMntPoint(KMountPoint::List &lst, QString value)
{
    if (value.length() > 1 && value.endsWith('/'))
        value = value.left(value.length() - 1);

    QExplicitlySharedDataPointer<KMountPoint> m;
    for (KMountPoint::List::iterator it = lst.begin(); it != lst.end(); ++it) {
        m = it->data();
        QString mntPnt = m->mountPoint();
        if (mntPnt.length() > 1 && mntPnt.endsWith('/'))
            mntPnt = mntPnt.left(mntPnt.length() - 1);
        if (mntPnt == value)
            return m;
    }

    return QExplicitlySharedDataPointer<KMountPoint>();
}

void KMountMan::jobResult(KJob *job)
{
    waiting = false;
    if (job->error())
        job->uiDelegate()->showErrorMessage();
}

void KMountMan::mount(QString mntPoint, bool blocking)
{
    QString udi = findUdiForPath(mntPoint, Solid::DeviceInterface::StorageAccess);
    if (!udi.isNull()) {
        Solid::Device device(udi);
        Solid::StorageAccess *access = device.as<Solid::StorageAccess>();
        if (access && !access->isAccessible()) {
            connect(access, &Solid::StorageAccess::setupDone, this, &KMountMan::slotSetupDone, Qt::UniqueConnection);
            if (blocking)
                waiting = true; // prepare to block
            access->setup();
        }
    } else {
        KMountPoint::List possible = KMountPoint::possibleMountPoints(KMountPoint::NeedMountOptions);
        QExplicitlySharedDataPointer<KMountPoint> m = findInListByMntPoint(possible, mntPoint);
        if (!((bool)m)) return;
        if (blocking)
            waiting = true; // prepare to block

        // KDE4 doesn't allow mounting devices as user, because they think it's the right behaviour.
        // I add this patch, as I don't think so.
        if (geteuid()) { // tries to mount as an user?
            KProcess proc;
            proc << KrServices::fullPathName("mount") << mntPoint;
            proc.start();
            if (!blocking)
                return;
            proc.waitForFinished(-1); // -1 msec blocks without timeout
            if (proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0)
                return;
        }

        KIO::SimpleJob *job = KIO::mount(false, m->mountType().toLocal8Bit(), m->mountedFrom(), m->mountPoint(), KIO::DefaultFlags);
        job->setUiDelegate(new KIO::JobUiDelegate());
        KIO::getJobTracker()->registerJob(job);
        connect(job, SIGNAL(result(KJob*)), this, SLOT(jobResult(KJob*)));
    }
    while (blocking && waiting) {
        qApp->processEvents();
        usleep(1000);
    }
}

void KMountMan::unmount(QString mntPoint, bool blocking)
{
    //if working dir is below mountpoint cd to ~ first
    if(QUrl::fromLocalFile(QDir(mntPoint).canonicalPath()).isParentOf(QUrl::fromLocalFile(QDir::current().canonicalPath())))
        QDir::setCurrent(QDir::homePath());

    QString udi = findUdiForPath(mntPoint, Solid::DeviceInterface::StorageAccess);
    if (!udi.isNull()) {
        Solid::Device device(udi);
        Solid::StorageAccess *access = device.as<Solid::StorageAccess>();
        if (access && access->isAccessible()) {
            connect(access, &Solid::StorageAccess::teardownDone, this, &KMountMan::slotTeardownDone, Qt::UniqueConnection);
            access->teardown();
        }
    } else {
        if (blocking)
            waiting = true; // prepare to block

        // KDE4 doesn't allow unmounting devices as user, because they think it's the right behaviour.
        // I add this patch, as I don't think so.
        if (geteuid()) { // tries to mount as an user?
            KProcess proc;
            proc << KrServices::fullPathName("umount") << mntPoint;
            proc.start();
            if (!blocking)
                return;
            proc.waitForFinished(-1); // -1 msec blocks without timeout
            if (proc.exitStatus() == QProcess::NormalExit && proc.exitCode() == 0)
                return;
        }

        KIO::SimpleJob *job = KIO::unmount(mntPoint, KIO::DefaultFlags);
        job->setUiDelegate(new KIO::JobUiDelegate());
        KIO::getJobTracker()->registerJob(job);
        connect(job, SIGNAL(result(KJob*)), this, SLOT(jobResult(KJob*)));
    }
    while (blocking && waiting) {
        qApp->processEvents();
        usleep(1000);
    }
}

KMountMan::mntStatus KMountMan::getStatus(QString mntPoint)
{
    QExplicitlySharedDataPointer<KMountPoint> mountPoint;

    // 1: is it already mounted
    KMountPoint::List current = KMountPoint::currentMountPoints();
    mountPoint = findInListByMntPoint(current, mntPoint);
    if ((bool) mountPoint)
        return MOUNTED;

    // 2: is it a mount point but not mounted?
    KMountPoint::List possible = KMountPoint::possibleMountPoints();
    mountPoint = findInListByMntPoint(possible, mntPoint);
    if ((bool) mountPoint)
        return NOT_MOUNTED;

    // 3: unknown
    return DOESNT_EXIST;
}


void KMountMan::toggleMount(QString mntPoint)
{
    mntStatus status = getStatus(mntPoint);
    switch (status) {
    case MOUNTED:
        unmount(mntPoint);
        break;
    case NOT_MOUNTED:
        mount(mntPoint);
        break;
    case DOESNT_EXIST:
        // do nothing: no-op to make the compiler quiet ;-)
        break;
    }
}

void KMountMan::autoMount(QString path)
{
    KConfigGroup group(krConfig, "Advanced");
    if (!group.readEntry("AutoMount", _AutoMount))
        return; // auto mount disabled

    if (getStatus(path) == NOT_MOUNTED)
        mount(path);
}

void KMountMan::eject(QString mntPoint)
{
    QString udi = findUdiForPath(mntPoint, Solid::DeviceInterface::OpticalDrive);
    if (udi.isNull())
        return;

    Solid::Device dev(udi);
    Solid::OpticalDrive *drive = dev.as<Solid::OpticalDrive>();
    if (drive == 0)
        return;

    //if working dir is below mountpoint cd to ~ first
    if(QUrl::fromLocalFile(QDir(mntPoint).canonicalPath()).isParentOf(QUrl::fromLocalFile(QDir::current().canonicalPath())))
        QDir::setCurrent(QDir::homePath());


    connect(drive, SIGNAL(ejectDone(Solid::ErrorType,QVariant,QString)),
            this, SLOT(slotTeardownDone(Solid::ErrorType,QVariant,QString)));

    drive->eject();
}

// returns true if the path is an ejectable mount point (at the moment CDROM and DVD)
bool KMountMan::ejectable(QString path)
{
    QString udi = findUdiForPath(path, Solid::DeviceInterface::OpticalDisc);
    if (udi.isNull())
        return false;

    Solid::Device dev(udi);
    return dev.as<Solid::OpticalDisc>() != 0;
}

bool KMountMan::removable(QString path)
{
    QString udi = findUdiForPath(path, Solid::DeviceInterface::StorageAccess);
    if (udi.isNull())
        return false;

    return removable(Solid::Device(udi));
}

bool KMountMan::removable(Solid::Device d)
{
    if(!d.isValid())
        return false;
    Solid::StorageDrive *drive = d.as<Solid::StorageDrive>();
    if(drive)
        return drive->isRemovable();
    else
        return(removable(d.parent()));
}

// a mountMan special version of KIO::convertSize, which deals
// with large filesystems ==> > 4GB, it actually receives size in
// a minimum block of 1024 ==> data is KB not bytes
QString KMountMan::convertSize(KIO::filesize_t size)
{
    float fsize;
    QString s;
    QLocale loc;
    // Tera-byte
    if (size >= 1073741824) {
        fsize = (float) size / (float) 1073741824;
        if (fsize > 1024)           // no name for something bigger than tera byte
            // let's call it Zega-Byte, who'll ever find out? :-)
            s = i18n("%1 ZB", loc.toString(fsize / (float) 1024, 'f', 1));
        else
            s = i18n("%1 TB", loc.toString(fsize, 'f', 1));
    }
    // Giga-byte
    else if (size >= 1048576) {
        fsize = (float) size / (float) 1048576;
        s = i18n("%1 GB", loc.toString(fsize, 'f', 1));
    }
    // Mega-byte
    else if (size > 1024) {
        fsize = (float) size / (float) 1024;
        s = i18n("%1 MB", loc.toString(fsize, 'f', 1));
    }
    // Kilo-byte
    else {
        fsize = (float) size;
        s = i18n("%1 KB", loc.toString(fsize, 'f', 0));
    }
    return s;
}


// populate the pop-up menu of the mountman tool-button with actions
void KMountMan::quickList()
{
    if (!_operational) {
        KMessageBox::error(0, i18n("MountMan is not operational. Sorry"));
        return;
    }

    // clear mount / unmount actions
    for (QAction *action : _action->menu()->actions()) {
        if (action == _manageAction || action->isSeparator()) {
            continue;
        }
        _action->menu()->removeAction(action);
    }

    // create lists of current and possible mount points
    const KMountPoint::List currentMountPoints = KMountPoint::currentMountPoints();

    // create a menu, displaying mountpoints with possible actions
    for (QExplicitlySharedDataPointer<KMountPoint> possibleMountPoint : KMountPoint::possibleMountPoints()) {
        // skip nonmountable file systems
        if (nonmountFilesystem(possibleMountPoint->mountType(), possibleMountPoint->mountPoint())
            || invalidFilesystem(possibleMountPoint->mountType())) {
            continue;
        }
        // does the mountpoint exist in current list?
        // if so, it can only be umounted, otherwise, it can be mounted
        bool needUmount = false;
        for (QExplicitlySharedDataPointer<KMountPoint> currentMountPoint : currentMountPoints) {
            if (currentMountPoint->mountPoint() == possibleMountPoint->mountPoint()) {
                // found -> needs umount
                needUmount = true;
                break;
            }
        }
        // add the item to the menu
        const QString text = QString("%1 %2 (%3)").arg(needUmount ? i18n("Unmount") : i18n("Mount"),
                                                 possibleMountPoint->mountPoint(), possibleMountPoint->mountedFrom());

        QAction *act = _action->menu()->addAction(text);
        act->setData(QList<QVariant>({
            QVariant(needUmount ? KMountMan::ActionType::Unmount : KMountMan::ActionType::Mount),
            QVariant(possibleMountPoint->mountPoint())
        }));
    }
    connect(_action->menu(), &QMenu::triggered, this, &KMountMan::delayedPerformAction);

}

void KMountMan::delayedPerformAction(const QAction *action)
{

    if (!action || !action->data().canConvert<QList<QVariant>>()) {
        return;
    }

    disconnect(_action->menu(), &QMenu::triggered, 0, 0);

    const QList<QVariant> actData = action->data().toList();
    const int actionType = actData[0].toInt();
    const QString mountPoint = actData[1].toString();

    QTimer::singleShot(0, this, [=] {

        if (actionType == KMountMan::ActionType::Mount) {
            mount(mountPoint);
        } else {
            unmount(mountPoint);
        }
    });
}

QString KMountMan::findUdiForPath(QString path, const Solid::DeviceInterface::Type &expType)
{
    KMountPoint::List current = KMountPoint::currentMountPoints();
    KMountPoint::List possible = KMountPoint::possibleMountPoints();
    QExplicitlySharedDataPointer<KMountPoint> mp = findInListByMntPoint(current, path);
    if (!(bool)mp) {
        mp = findInListByMntPoint(possible, path);
        if (!(bool)mp)
            return QString();
    }
    QString dev = QDir(mp->mountedFrom()).canonicalPath();
    QList<Solid::Device> storageDevices = Solid::Device::listFromType(Solid::DeviceInterface::Block);

    for (int p = storageDevices.count() - 1 ; p >= 0; p--) {
        Solid::Device device = storageDevices[ p ];
        QString udi     = device.udi();

        Solid::Block * sb = device.as<Solid::Block>();
        if (sb) {
            QString devb = QDir(sb->device()).canonicalPath();
            if (expType != Solid::DeviceInterface::Unknown && !device.isDeviceInterface(expType))
                continue;
            if (devb == dev)
                return udi;
        }
    }

    return QString();
}

QString KMountMan::pathForUdi(QString udi)
{
    Solid::Device device(udi);
    Solid::StorageAccess *access = device.as<Solid::StorageAccess>();
    if(access)
        return access->filePath();
    else
        return QString();
}

void KMountMan::slotTeardownDone(Solid::ErrorType error, QVariant errorData, const QString& /*udi*/)
{
    waiting = false;
    if (error != Solid::NoError && errorData.isValid()) {
        KMessageBox::sorry(parentWindow, errorData.toString());
    }
}

void KMountMan::slotSetupDone(Solid::ErrorType error, QVariant errorData, const QString& /*udi*/)
{
    waiting = false;
    if (error != Solid::NoError && errorData.isValid()) {
        KMessageBox::sorry(parentWindow, errorData.toString());
    }
}

