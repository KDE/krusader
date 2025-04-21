/*
    SPDX-FileCopyrightText: 2006 Csaba Karai <cskarai@freemail.hu>
    SPDX-FileCopyrightText: 2006-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mediabutton.h"

#include "../MountMan/kmountman.h"
#include "../icon.h"
#include "../krglobal.h"

// QtCore
#include <QEvent>
// QtGui
#include <QCursor>
#include <QMouseEvent>

#include <KConfigGroup>
#include <KIO/Global>
#include <KLocalizedString>
#include <KMessageBox>
#include <KMountPoint>
#include <KSharedConfig>

#include <Solid/DeviceInterface>
#include <Solid/DeviceNotifier>
#include <Solid/OpticalDisc>
#include <Solid/OpticalDrive>
#include <Solid/StorageAccess>
#include <Solid/StorageVolume>
#include <utility>

QString MediaButton::remotePrefix = QLatin1String("remote:");

MediaButton::MediaButton(QWidget *parent)
    : QToolButton(parent)
    , popupMenu(nullptr)
    , rightMenu(nullptr)
    , openInNewTab(false)
{
    setAutoRaise(true);
    setIcon(Icon("system-file-manager"));
    setText(i18n("Open the available media list"));
    setToolTip(i18n("Open the available media list"));
    setPopupMode(QToolButton::InstantPopup);
    setAcceptDrops(false);

    popupMenu = new QMenu(this);
    popupMenu->installEventFilter(this);
    Q_CHECK_PTR(popupMenu);

    setMenu(popupMenu);

    connect(popupMenu, &QMenu::aboutToShow, this, &MediaButton::slotAboutToShow);
    connect(popupMenu, &QMenu::aboutToHide, this, &MediaButton::slotAboutToHide);
    connect(popupMenu, &QMenu::triggered, this, &MediaButton::slotPopupActivated);

    Solid::DeviceNotifier *notifier = Solid::DeviceNotifier::instance();
    connect(notifier, &Solid::DeviceNotifier::deviceAdded, this, &MediaButton::slotDeviceAdded);
    connect(notifier, &Solid::DeviceNotifier::deviceRemoved, this, &MediaButton::slotDeviceRemoved);

    connect(&mountCheckerTimer, &QTimer::timeout, this, &MediaButton::slotCheckMounts);
}

MediaButton::~MediaButton() = default;

void MediaButton::updateIcon(const QString &mountPoint)
{
    if (!mountPoint.isEmpty() && mountPoint == currentMountPoint)
        return;

    currentMountPoint = mountPoint;

    QString iconName("system-file-manager");
    QStringList overlays;

    if (!mountPoint.isEmpty()) {
        Solid::Device device(krMtMan.findUdiForPath(mountPoint, Solid::DeviceInterface::StorageAccess));
        auto *vol = device.as<Solid::StorageVolume>();

        if (device.isValid())
            iconName = device.icon();
        if (vol && vol->usage() == Solid::StorageVolume::Encrypted)
            overlays << "security-high";
    }
    setIcon(Icon(iconName, overlays));
}

void MediaButton::slotAboutToShow()
{
    emit aboutToShow();

    popupMenu->clear();
    udiNameMap.clear();

    createMediaList();
}

void MediaButton::slotAboutToHide()
{
    if (rightMenu)
        rightMenu->close();
    mountCheckerTimer.stop();
}

void MediaButton::createMediaList()
{
    // devices detected by solid
    storageDevices = Solid::Device::listFromType(Solid::DeviceInterface::StorageAccess);

    for (qsizetype p = storageDevices.count() - 1; p >= 0; p--) {
        Solid::Device device = storageDevices[p];
        QString udi = device.udi();

        QString name;
        QIcon kdeIcon;
        if (!getNameAndIcon(device, name, kdeIcon))
            continue;

        QAction *act = popupMenu->addAction(kdeIcon, name);
        act->setData(QVariant(udi));
        udiNameMap[udi] = name;

        connect(device.as<Solid::StorageAccess>(), &Solid::StorageAccess::accessibilityChanged, this, &MediaButton::slotAccessibilityChanged);
    }

    KMountPoint::List possibleMountList = KMountPoint::possibleMountPoints();
    KMountPoint::List currentMountList = KMountPoint::currentMountPoints();

    for (auto &it : possibleMountList) {
        if (krMtMan.networkFilesystem(it->mountType())) {
            bool mounted = false;

            for (auto &it2 : currentMountList) {
                if (krMtMan.networkFilesystem(it2->mountType()) && it->mountPoint() == it2->mountPoint()) {
                    mounted = true;
                    break;
                }
            }

            QString name = i18nc("%1 is the mount point of the remote share", "Remote Share [%1]", it->mountPoint());
            QStringList overlays;
            if (mounted)
                overlays << "emblem-mounted";
            QAction *act = popupMenu->addAction(Icon("network-wired", overlays), name);
            QString udi = remotePrefix + it->mountPoint();
            act->setData(QVariant(udi));
        }
    }

    mountCheckerTimer.setSingleShot(true);
    mountCheckerTimer.start(1000);
}

bool MediaButton::getNameAndIcon(Solid::Device &device, QString &name, QIcon &iconOut)
{
    auto *access = device.as<Solid::StorageAccess>();
    if (access == nullptr)
        return false;

    QString label = i18nc("Unknown label", "Unknown");
    bool mounted = access->isAccessible();
    QString path = access->filePath();
    QString type = i18nc("Unknown media type", "Unknown");
    QString iconName = device.icon();
    QString fstype;
    QString size;

    auto *vol = device.as<Solid::StorageVolume>();
    if (vol) {
        label = vol->label();
        fstype = vol->fsType();
        size = KIO::convertSize(vol->size());
    }

    bool printSize = false;

    if (iconName == "media-floppy")
        type = i18n("Floppy");
    else if (iconName == "drive-optical")
        type = i18n("CD/DVD-ROM");
    else if (iconName == "drive-removable-media-usb-pendrive")
        type = i18n("USB pen drive"), printSize = true;
    else if (iconName == "drive-removable-media-usb")
        type = i18n("USB device"), printSize = true;
    else if (iconName == "drive-removable-media")
        type = i18n("Removable media"), printSize = true;
    else if (iconName == "drive-harddisk")
        type = i18n("Hard Disk"), printSize = true;
    else if (iconName == "camera-photo")
        type = i18n("Camera");
    else if (iconName == "media-optical-video")
        type = i18n("Video CD/DVD-ROM");
    else if (iconName == "media-optical-audio")
        type = i18n("Audio CD/DVD-ROM");
    else if (iconName == "media-optical")
        type = i18n("Recordable CD/DVD-ROM");

    KConfigGroup cfg(KSharedConfig::openConfig(), QStringLiteral("MediaMenu"));

    if (fstype == "squashfs" && cfg.readEntry("HideSquashFS", false)) {
        return false;
    }

    if (fstype.isEmpty() && cfg.readEntry("HideUnknownFS", false)) {
        return false;
    }

    if (cfg.readEntry("Hidden Mountpoints", QStringList()).contains(path)) {
        return false;
    }

    if (printSize) {
        QString showSizeSetting = cfg.readEntry("ShowSize", "Always");
        if (showSizeSetting == "WhenNoLabel")
            printSize = label.isEmpty();
        else if (showSizeSetting == "Never")
            printSize = false;
    }

    if (printSize && !size.isEmpty())
        name += size + ' ';
    if (!label.isEmpty())
        name += label + ' ';
    else
        name += type + ' ';
    if (!fstype.isEmpty() && cfg.readEntry("ShowFSType", true))
        name += '(' + fstype + ") ";
    if (!path.isEmpty() && cfg.readEntry("ShowPath", true))
        name += '[' + path + "] ";

    name = name.trimmed();

    QStringList overlays;
    if (mounted) {
        overlays << "emblem-mounted";
    } else {
        overlays << QString(); // We have to guarantee the placement of the next emblem
    }
    if (vol && vol->usage() == Solid::StorageVolume::Encrypted) {
        overlays << "security-high";
    }
    iconOut = Icon(iconName, overlays);
    return true;
}

void MediaButton::slotPopupActivated(QAction *action)
{
    if (action && action->data().canConvert<QString>()) {
        QString udi = action->data().toString();
        if (!udi.isEmpty()) {
            bool mounted = false;
            QString mountPoint;
            getStatus(udi, mounted, &mountPoint);

            if (mounted)
                emit openUrl(QUrl::fromLocalFile(mountPoint));
            else
                mount(udi, true);
        }
    }
}

void MediaButton::showMenu()
{
    QMenu *pP = menu();
    if (pP) {
        menu()->exec(mapToGlobal(QPoint(0, height())));
    }
}

bool MediaButton::eventFilter(QObject *o, QEvent *e)
{
    if (o == popupMenu) {
        if (e->type() == QEvent::ContextMenu) {
            QAction *act = popupMenu->activeAction();
            if (act && act->data().canConvert<QString>()) {
                QString id = act->data().toString();
                if (!id.isEmpty()) {
                    QPoint globalPos = popupMenu->mapToGlobal(popupMenu->actionGeometry(act).topRight());
                    rightClickMenu(id, globalPos);
                    return true;
                }
            }
        } else if (e->type() == QEvent::KeyPress) {
            auto *ke = dynamic_cast<QKeyEvent *>(e);
            if (ke->key() == Qt::Key_Return && ke->modifiers() == Qt::ControlModifier) {
                if (QAction *act = popupMenu->activeAction()) {
                    QString id = act->data().toString();
                    if (!id.isEmpty()) {
                        toggleMount(id);
                        return true;
                    }
                }
            }
        } else if (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonRelease) {
            auto *m = dynamic_cast<QMouseEvent *>(e);
            if (m->button() == Qt::RightButton) {
                if (e->type() == QEvent::MouseButtonPress) {
                    QAction *act = popupMenu->actionAt(m->pos());
                    if (act && act->data().canConvert<QString>()) {
                        QString id = act->data().toString();
                        if (!id.isEmpty())
                            rightClickMenu(id, m->globalPosition().toPoint());
                    }
                }
                m->accept();
                return true;
            }
        }
    }
    return false;
}

void MediaButton::rightClickMenu(const QString &udi, QPoint pos)
{
    if (rightMenu)
        rightMenu->close();

    bool ejectable = false;
    bool mounted = false;
    QString mountPoint;

    getStatus(udi, mounted, &mountPoint, &ejectable);

    QUrl openURL = QUrl::fromLocalFile(mountPoint);

    QMenu *myMenu = rightMenu = new QMenu(popupMenu);
    QAction *actOpen = myMenu->addAction(i18n("Open"));
    actOpen->setData(QVariant(1));
    QAction *actOpenNewTab = myMenu->addAction(i18n("Open in a new tab"));
    actOpenNewTab->setData(QVariant(2));

    myMenu->addSeparator();
    if (!mounted) {
        QAction *actMount = myMenu->addAction(i18n("Mount"));
        actMount->setData(QVariant(3));
    } else {
        QAction *actUnmount = myMenu->addAction(i18n("Unmount"));
        actUnmount->setData(QVariant(4));
    }
    if (ejectable) {
        QAction *actEject = myMenu->addAction(i18n("Eject"));
        actEject->setData(QVariant(5));
    }

    QAction *act = myMenu->exec(pos);
    int result = -1;
    if (act != nullptr && act->data().canConvert<int>())
        result = act->data().toInt();

    delete myMenu;
    if (rightMenu == myMenu)
        rightMenu = nullptr;
    else
        return;

    switch (result) {
    case 1:
    case 2:
        popupMenu->close();
        if (mounted) {
            if (result == 1)
                emit openUrl(openURL);
            else
                emit newTab(openURL);
        } else {
            mount(udi, true, result == 2); // mount first, when mounted open the tab
        }
        break;
    case 3:
        mount(udi);
        break;
    case 4:
        umount(udi);
        break;
    case 5:
        eject(udi);
        break;
    default:
        break;
    }
}

void MediaButton::toggleMount(const QString &udi)
{
    bool mounted = false;
    getStatus(udi, mounted);

    if (mounted)
        umount(udi);
    else
        mount(udi);
}

void MediaButton::getStatus(const QString &udi, bool &mounted, QString *mountPointOut, bool *ejectableOut)
{
    mounted = false;
    bool network = udi.startsWith(remotePrefix);
    bool ejectable = false;
    QString mountPoint;

    if (network) {
        mountPoint = udi.mid(remotePrefix.length());

        KMountPoint::List currentMountList = KMountPoint::currentMountPoints();
        for (auto &it : currentMountList) {
            if (krMtMan.networkFilesystem(it->mountType()) && it->mountPoint() == mountPoint) {
                mounted = true;
                break;
            }
        }
    } else {
        Solid::Device device(udi);

        auto *access = device.as<Solid::StorageAccess>();
        auto *optdisc = device.as<Solid::OpticalDisc>();
        if (access)
            mountPoint = access->filePath();
        if (access && access->isAccessible())
            mounted = true;
        if (optdisc)
            ejectable = true;
    }

    if (mountPointOut)
        *mountPointOut = mountPoint;
    if (ejectableOut)
        *ejectableOut = ejectable;
}

void MediaButton::mount(const QString &udi, bool open, bool newtab)
{
    if (udi.startsWith(remotePrefix)) {
        QString mp = udi.mid(remotePrefix.length());
        krMtMan.mount(mp, true);
        if (newtab)
            emit newTab(QUrl::fromLocalFile(mp));
        else
            emit openUrl(QUrl::fromLocalFile(mp));
        return;
    }
    Solid::Device device(udi);
    auto *access = device.as<Solid::StorageAccess>();
    if (access && !access->isAccessible()) {
        if (open)
            udiToOpen = device.udi(), openInNewTab = newtab;
        connect(access, &Solid::StorageAccess::setupDone, this, &MediaButton::slotSetupDone);
        connect(access, &Solid::StorageAccess::teardownRequested, &krMtMan, &KMountMan::slotTeardownRequested, Qt::UniqueConnection);
        access->setup();
    }
}

void MediaButton::slotSetupDone(Solid::ErrorType error, const QVariant &errorData, const QString &udi)
{
    if (error == Solid::NoError) {
        if (udi == udiToOpen) {
            auto *access = Solid::Device(udi).as<Solid::StorageAccess>();
            if (access && access->isAccessible()) {
                if (openInNewTab)
                    emit newTab(QUrl::fromLocalFile(access->filePath()));
                else
                    emit openUrl(QUrl::fromLocalFile(access->filePath()));
            }
            udiToOpen = QString(), openInNewTab = false;
        }
    } else {
        if (udi == udiToOpen)
            udiToOpen = QString(), openInNewTab = false;
        QString name;
        if (udiNameMap.contains(udi))
            name = udiNameMap[udi];

        if (errorData.isValid()) {
            KMessageBox::error(this, i18n("An error occurred while accessing '%1', the system responded: %2", name, errorData.toString()));
        } else {
            KMessageBox::error(this, i18n("An error occurred while accessing '%1'", name));
        }
    }
}

void MediaButton::umount(const QString &udi)
{
    if (udi.startsWith(remotePrefix)) {
        krMtMan.unmount(udi.mid(remotePrefix.length()), false);
        return;
    }
    krMtMan.unmount(krMtMan.pathForUdi(udi), false);
}

void MediaButton::eject(QString udi)
{
    krMtMan.eject(krMtMan.pathForUdi(std::move(udi)));
}

void MediaButton::slotAccessibilityChanged(bool /*accessible*/, const QString &udi)
{
    const QList<QAction *> actionList = popupMenu->actions();
    for (QAction *act : actionList) {
        if (act && act->data().canConvert<QString>() && act->data().toString() == udi) {
            Solid::Device device(udi);

            QString name;
            QIcon kdeIcon;
            if (getNameAndIcon(device, name, kdeIcon)) {
                act->setText(name);
                act->setIcon(kdeIcon);
            }
            break;
        }
    }
}

void MediaButton::slotDeviceAdded(const QString &udi)
{
    if (popupMenu->isHidden())
        return;

    Solid::Device device(udi);
    auto *access = device.as<Solid::StorageAccess>();
    if (access == nullptr)
        return;

    QString name;
    QIcon kdeIcon;
    if (!getNameAndIcon(device, name, kdeIcon))
        return;

    QAction *act = popupMenu->addAction(kdeIcon, name);
    act->setData(QVariant(udi));
    udiNameMap[udi] = name;

    connect(device.as<Solid::StorageAccess>(), &Solid::StorageAccess::accessibilityChanged, this, &MediaButton::slotAccessibilityChanged);
}

void MediaButton::slotDeviceRemoved(const QString &udi)
{
    if (popupMenu->isHidden())
        return;

    const QList<QAction *> actionList = popupMenu->actions();
    for (QAction *act : actionList) {
        if (act && act->data().canConvert<QString>() && act->data().toString() == udi) {
            popupMenu->removeAction(act);
            delete act;
            break;
        }
    }
}

void MediaButton::slotCheckMounts()
{
    if (isHidden())
        return;

    KMountPoint::List possibleMountList = KMountPoint::possibleMountPoints();
    KMountPoint::List currentMountList = KMountPoint::currentMountPoints();
    const QList<QAction *> actionList = popupMenu->actions();

    for (QAction *act : actionList) {
        if (act && act->data().canConvert<QString>() && act->data().toString().startsWith(remotePrefix)) {
            QString mountPoint = act->data().toString().mid(remotePrefix.length());
            bool available = false;

            for (auto &it : possibleMountList) {
                if (krMtMan.networkFilesystem(it->mountType()) && it->mountPoint() == mountPoint) {
                    available = true;
                    break;
                }
            }

            if (!available) {
                popupMenu->removeAction(act);
                delete act;
            }
        }
    }

    for (auto &it : possibleMountList) {
        if (krMtMan.networkFilesystem(it->mountType())) {
            QString path = it->mountPoint();
            bool mounted = false;
            QString udi = remotePrefix + path;

            QAction *correspondingAct = nullptr;
            for (QAction *act : actionList) {
                if (act && act->data().canConvert<QString>() && act->data().toString() == udi) {
                    correspondingAct = act;
                    break;
                }
            }
            for (auto &it2 : currentMountList) {
                if (krMtMan.networkFilesystem(it2->mountType()) && path == it2->mountPoint()) {
                    mounted = true;
                    break;
                }
            }

            QString name = i18nc("%1 is the mount point of the remote share", "Remote Share [%1]", it->mountPoint());
            QStringList overlays;
            if (mounted)
                overlays << "emblem-mounted";
            QIcon kdeIcon = Icon("network-wired", overlays);

            if (!correspondingAct) {
                QAction *act = popupMenu->addAction(kdeIcon, name);
                act->setData(QVariant(udi));
            } else {
                correspondingAct->setText(name);
                correspondingAct->setIcon(kdeIcon);
            }
        }
    }

    mountCheckerTimer.setSingleShot(true);
    mountCheckerTimer.start(1000);
}
