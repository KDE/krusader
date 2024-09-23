/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmountman.h"

// QtCore
#include <QDir>
// QtWidgets
#include <QApplication>
#include <QMenu>

#include <KIO/JobUiDelegate>
#include <KIO/JobUiDelegateFactory>
#include <KIO/SimpleJob>
#include <KIO/JobTracker>
#include <KJobTrackerInterface>
#include <KLocalizedString>
#include <KMessageBox>
#include <KProcess>
#include <KSharedConfig>
#include <KToolBarPopupAction>

#include <Solid/Block>
#include <Solid/Device>
#include <Solid/DeviceNotifier>
#include <Solid/OpticalDisc>
#include <Solid/OpticalDrive>
#include <Solid/StorageAccess>
#include <Solid/StorageVolume>
#include <utility>

#include "../Dialogs/krdialogs.h"
#include "../FileSystem/krpermhandler.h"
#include "../defaults.h"
#include "../icon.h"
#include "../kractions.h"
#include "../krglobal.h"
#include "../krservices.h"
#include "kmountmangui.h"

#ifdef _OS_SOLARIS_
#define FSTAB "/etc/filesystemtab"
#else
#define FSTAB "/etc/fstab"
#endif

KMountMan::KMountMan(QWidget *parent)
    : _operational(false)
    , waiting(false)
    , mountManGui(nullptr)
    , parentWindow(parent)
{
    _action = new KToolBarPopupAction(Icon("kr_mountman"), i18n("&MountMan..."), this);
    connect(_action, &QAction::triggered, this, &KMountMan::mainWindow);
    connect(_action->popupMenu(), &QMenu::aboutToShow, this, &KMountMan::quickList);
    _manageAction = _action->popupMenu()->addAction(i18n("Open &MountMan"));
    connect(_manageAction, &QAction::triggered, this, &KMountMan::mainWindow);
    _action->popupMenu()->addSeparator();

    // added as a precaution, although we use kde services now
    _operational = KrServices::cmdExist("mount");

    network_fs << "nfs"
               << "smbfs"
               << "fuse.fusesmb"
               << "fuse.sshfs"; // TODO: is this list complete ?

    // list of FS that we don't manage at all
    invalid_fs << "swap"
               << "/dev/pts"
               << "tmpfs"
               << "devpts"
               << "sysfs"
               << "rpc_pipefs"
               << "usbfs"
               << "binfmt_misc";
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
        for (auto &it : nonmount_fs_mntpoint) {
            it = it.simplified();
        }
    }

    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded, this, &KMountMan::deviceAdded);

    for (Solid::Device &device : Solid::Device::allDevices()) {
        if (device.isValid()) {
            QPointer<Solid::StorageAccess> access = device.as<Solid::StorageAccess>();
            if (access) {
                connect(access, &Solid::StorageAccess::teardownRequested, this, &KMountMan::slotTeardownRequested, Qt::UniqueConnection);
            }
        }
    }
}

KMountMan::~KMountMan() = default;

bool KMountMan::invalidFilesystem(const QString &type)
{
    return (invalid_fs.contains(type));
}

// this is an ugly hack, but type can actually be a mountpoint. oh well...
bool KMountMan::nonmountFilesystem(const QString &type, const QString &mntPoint)
{
    return (nonmount_fs.contains(type) || nonmount_fs_mntpoint.contains(mntPoint));
}

bool KMountMan::networkFilesystem(const QString &type)
{
    return (network_fs.contains(type));
}

void KMountMan::mainWindow()
{
    // left as a precaution, although we use kde's services now
    if (!KrServices::cmdExist("mount")) {
        KMessageBox::error(nullptr, i18n("Cannot start 'mount'. Check the 'Dependencies' page in konfigurator."));
        return;
    }

    mountManGui = new KMountManGUI(this);
    delete mountManGui; /* as KMountManGUI is modal, we can now delete it */
    mountManGui = nullptr; /* for sanity */
}

QExplicitlySharedDataPointer<KMountPoint> KMountMan::findInListByMntPoint(KMountPoint::List &lst, QString value)
{
    if (value.length() > 1 && value.endsWith('/'))
        value = value.left(value.length() - 1);

    QExplicitlySharedDataPointer<KMountPoint> m;
    for (auto &it : lst) {
        m = it.data();
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

void KMountMan::mount(const QString &mntPoint, bool blocking)
{
    QString udi = findUdiForPath(mntPoint, Solid::DeviceInterface::StorageAccess);
    if (!udi.isNull()) {
        Solid::Device device(udi);
        auto *access = device.as<Solid::StorageAccess>();
        if (access && !access->isAccessible()) {
            connect(access, &Solid::StorageAccess::setupDone, this, &KMountMan::slotSetupDone, Qt::UniqueConnection);
            if (blocking)
                waiting = true; // prepare to block
            access->setup();
        }
    } else {
        KMountPoint::List possible = KMountPoint::possibleMountPoints(KMountPoint::NeedMountOptions);
        QExplicitlySharedDataPointer<KMountPoint> m = findInListByMntPoint(possible, mntPoint);
        if (!((bool)m))
            return;
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
        job->setUiDelegate(KIO::createDefaultJobUiDelegate());
        KIO::getJobTracker()->registerJob(job);
        connect(job, &KIO::SimpleJob::result, this, &KMountMan::jobResult);
    }
    while (blocking && waiting) {
        qApp->processEvents();
        usleep(1000);
    }
}

void KMountMan::unmount(const QString &mntPoint, bool blocking)
{
    // if working dir is below mountpoint cd to ~ first
    if (QUrl::fromLocalFile(QDir(mntPoint).canonicalPath()).isParentOf(QUrl::fromLocalFile(QDir::current().canonicalPath())))
        QDir::setCurrent(QDir::homePath());

    if (QUrl::fromLocalFile(QDir(mntPoint).canonicalPath()) == QUrl::fromLocalFile(QDir::current().canonicalPath()))
        QDir::setCurrent(QDir::homePath());

    QString udi = findUdiForPath(mntPoint, Solid::DeviceInterface::StorageAccess);
    if (!udi.isNull()) {
        Solid::Device device(udi);
        auto *access = device.as<Solid::StorageAccess>();
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
        job->setUiDelegate(KIO::createDefaultJobUiDelegate());
        KIO::getJobTracker()->registerJob(job);
        connect(job, &KIO::SimpleJob::result, this, &KMountMan::jobResult);
    }
    while (blocking && waiting) {
        qApp->processEvents();
        usleep(1000);
    }
}

KMountMan::mntStatus KMountMan::getStatus(const QString &mntPoint)
{
    QExplicitlySharedDataPointer<KMountPoint> mountPoint;

    // 1: is it already mounted
    KMountPoint::List current = KMountPoint::currentMountPoints();
    mountPoint = findInListByMntPoint(current, mntPoint);
    if ((bool)mountPoint)
        return MOUNTED;

    // 2: is it a mount point but not mounted?
    KMountPoint::List possible = KMountPoint::possibleMountPoints();
    mountPoint = findInListByMntPoint(possible, mntPoint);
    if ((bool)mountPoint)
        return NOT_MOUNTED;

    // 3: unknown
    return DOESNT_EXIST;
}

void KMountMan::toggleMount(const QString &mntPoint)
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

void KMountMan::autoMount(const QString &path)
{
    KConfigGroup group(krConfig, "Advanced");
    if (!group.readEntry("AutoMount", _AutoMount))
        return; // auto mount disabled

    if (getStatus(path) == NOT_MOUNTED)
        mount(path);
}

void KMountMan::eject(const QString &mntPoint)
{
    QString udi = findUdiForPath(mntPoint, Solid::DeviceInterface::OpticalDrive);
    if (udi.isNull())
        return;

    Solid::Device dev(udi);
    auto *drive = dev.as<Solid::OpticalDrive>();
    if (drive == nullptr)
        return;

    // if working dir is below mountpoint cd to ~ first
    if (QUrl::fromLocalFile(QDir(mntPoint).canonicalPath()).isParentOf(QUrl::fromLocalFile(QDir::current().canonicalPath())))
        QDir::setCurrent(QDir::homePath());

    connect(drive, &Solid::OpticalDrive::ejectDone, this, &KMountMan::slotTeardownDone);

    drive->eject();
}

// returns true if the path is an ejectable mount point (at the moment CDROM and DVD)
bool KMountMan::ejectable(QString path)
{
    QString udi = findUdiForPath(std::move(path), Solid::DeviceInterface::OpticalDisc);
    if (udi.isNull())
        return false;

    Solid::Device dev(udi);
    return dev.as<Solid::OpticalDisc>() != nullptr;
}

bool KMountMan::removable(QString path)
{
    QString udi = findUdiForPath(std::move(path), Solid::DeviceInterface::StorageAccess);
    if (udi.isNull())
        return false;

    return removable(Solid::Device(udi));
}

bool KMountMan::removable(Solid::Device d)
{
    if (!d.isValid())
        return false;
    auto *drive = d.as<Solid::StorageDrive>();
    if (drive)
        return drive->isRemovable();
    else
        return (removable(d.parent()));
}

// populate the pop-up menu of the mountman tool-button with actions
void KMountMan::quickList()
{
    if (!_operational) {
        KMessageBox::error(nullptr, i18n("MountMan is not operational. Sorry"));
        return;
    }

    // clear mount / unmount actions
    for (QAction *action : _action->popupMenu()->actions()) {
        if (action == _manageAction || action->isSeparator()) {
            continue;
        }
        _action->popupMenu()->removeAction(action);
    }

    // create lists of current and possible mount points
    const KMountPoint::List currentMountPoints = KMountPoint::currentMountPoints();

    // create a menu, displaying mountpoints with possible actions
    for (QExplicitlySharedDataPointer<KMountPoint> possibleMountPoint : KMountPoint::possibleMountPoints()) {
        // skip nonmountable file systems
        if (nonmountFilesystem(possibleMountPoint->mountType(), possibleMountPoint->mountPoint()) || invalidFilesystem(possibleMountPoint->mountType())) {
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
        const QString text =
            QString("%1 %2 (%3)").arg(needUmount ? i18n("Unmount") : i18n("Mount"), possibleMountPoint->mountPoint(), possibleMountPoint->mountedFrom());

        QAction *act = _action->popupMenu()->addAction(text);
        act->setData(QList<QVariant>(
            {QVariant(needUmount ? KMountMan::ActionType::Unmount : KMountMan::ActionType::Mount), QVariant(possibleMountPoint->mountPoint())}));
    }
    connect(_action->popupMenu(), &QMenu::triggered, this, &KMountMan::delayedPerformAction);
}

void KMountMan::delayedPerformAction(const QAction *action)
{
    if (!action || !action->data().canConvert<QList<QVariant>>()) {
        return;
    }

    disconnect(_action->popupMenu(), &QMenu::triggered, nullptr, nullptr);

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

QString KMountMan::findUdiForPath(const QString &path, const Solid::DeviceInterface::Type &expType)
{
    const std::function deviceGetter = []() {
        return Solid::Device::listFromType(Solid::DeviceInterface::Block);
    };
    return findUdiForPath(path, expType, deviceGetter);
}

QString KMountMan::findUdiForPath(const QString &path, const Solid::DeviceInterface::Type &expType, const std::function<QList<Solid::Device>()> &devicesGetter)
{
    KMountPoint::List current = KMountPoint::currentMountPoints();
    QExplicitlySharedDataPointer<KMountPoint> mountPoint = findInListByMntPoint(current, path);

    if (!static_cast<bool>(mountPoint)) {
        KMountPoint::List possible = KMountPoint::possibleMountPoints();
        mountPoint = findInListByMntPoint(possible, path);
        if (!static_cast<bool>(mountPoint))
            return {}; // cannot not find mount point for path
    }
    const QString mountPath = QDir(mountPoint->mountedFrom()).canonicalPath();
    if (mountPath.isEmpty()) {
        return {}; // mount point does not have a device (e.g. for virtual kernel filesystems)
    }

    QList<Solid::Device> storageDevices = devicesGetter();

    for (qsizetype p = storageDevices.count() - 1; p >= 0; p--) {
        Solid::Device device = storageDevices[p];
        QString udi = device.udi();

        auto *sb = device.as<Solid::Block>();
        if (sb) {
            QString devicePath = QDir(sb->device()).canonicalPath();
            if (expType != Solid::DeviceInterface::Unknown && !device.isDeviceInterface(expType))
                continue;
            if (devicePath == mountPath)
                return udi;
        }
    }

    return {}; // device not found
}

QString KMountMan::pathForUdi(const QString &udi)
{
    Solid::Device device(udi);
    auto *access = device.as<Solid::StorageAccess>();
    if (access)
        return access->filePath();
    else
        return QString();
}

void KMountMan::slotTeardownDone(Solid::ErrorType error, const QVariant &errorData, const QString & /*udi*/)
{
    waiting = false;
    if (error != Solid::NoError && errorData.isValid()) {
        KMessageBox::error(parentWindow, errorData.toString());
    }
}

void KMountMan::slotSetupDone(Solid::ErrorType error, const QVariant &errorData, const QString & /*udi*/)
{
    waiting = false;
    if (error != Solid::NoError && errorData.isValid()) {
        KMessageBox::error(parentWindow, errorData.toString());
    }
}

void KMountMan::slotTeardownRequested(const QString &udi)
{
    const QString mntPoint = KMountMan::pathForUdi(udi);
    KMountMan::unmount(mntPoint, false);
}

void KMountMan::deviceAdded(const QString &udi)
{
    Solid::Device device(udi);

    if (device.isValid()) {
        QPointer<Solid::StorageAccess> access = device.as<Solid::StorageAccess>();
        if (access) {
            connect(access, &Solid::StorageAccess::teardownRequested, this, &KMountMan::slotTeardownRequested, Qt::UniqueConnection);
        }
    }
}
