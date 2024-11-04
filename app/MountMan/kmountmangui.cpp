/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kmountmangui.h"
#include "../Dialogs/krspecialwidgets.h"
#include "../FileSystem/filesystem.h"
#include "../compat.h"
#include "../defaults.h"
#include "../icon.h"
#include "../krglobal.h"

// QtCore
#include <QCryptographicHash>
#include <QFileInfo>
#include <QList>
#include <QEventLoop>
// QtGui
#include <QBitmap>
#include <QCursor>
#include <QPixmap>
// QtWidgets
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMenu>
#include <QPushButton>

#include <KGuiItem>
#include <KLocalizedString>
#include <KMessageBox>
#include <KMountPoint>
#include <KSharedConfig>
#include <KIO/FileSystemFreeSpaceJob>

#include <Solid/StorageVolume>

#ifndef BSD
#define MTAB "/etc/mtab"
#endif

constexpr int fileSystemsFreeSpaceTimeout = 5'000; // msec

KMountManGUI::KMountManGUI(KMountMan *mntMan)
    : QDialog(mntMan->parentWindow)
    , mountMan(mntMan)
    , info(nullptr)
    , mountList(nullptr)
    , cbShowOnlyRemovable(nullptr)
    , watcher(nullptr)
    , sizeX(-1)
    , sizeY(-1)
{
    setWindowTitle(i18n("MountMan - Your Mount-Manager"));
    setWindowModality(Qt::WindowModal);

    auto *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    watcher = new QTimer(this);
    connect(watcher, &QTimer::timeout, this, &KMountManGUI::checkMountChange);

    mainLayout->addLayout(createMainPage());

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    mainLayout->addWidget(buttonBox);

    ejectButton = new QPushButton(i18n("&Eject"));
    ejectButton->setIcon(Icon(QStringLiteral("media-eject")));
    ejectButton->setEnabled(false);
    buttonBox->addButton(ejectButton, QDialogButtonBox::ActionRole);

    mountButton = new QPushButton(i18n("&Unmount"));
    mountButton->setEnabled(false);
    buttonBox->addButton(mountButton, QDialogButtonBox::ActionRole);

    // connections
    connect(buttonBox, &QDialogButtonBox::rejected, this, &KMountManGUI::reject);
    connect(ejectButton, &QPushButton::clicked, this, &KMountManGUI::slotEject);
    connect(mountButton, &QPushButton::clicked, this, &KMountManGUI::slotToggleMount);
    connect(mountList, &KrTreeWidget::itemDoubleClicked, this, &KMountManGUI::doubleClicked);
    connect(mountList, &KrTreeWidget::itemRightClicked, this, &KMountManGUI::clicked);
    connect(mountList, &KrTreeWidget::itemClicked, this, QOverload<QTreeWidgetItem *>::of(&KMountManGUI::changeActive));
    connect(mountList, &KrTreeWidget::itemSelectionChanged, this, QOverload<>::of(&KMountManGUI::changeActive));

    KConfigGroup group(krConfig, "MountMan");
    int sx = group.readEntry("Window Width", -1);
    int sy = group.readEntry("Window Height", -1);

    if (sx != -1 && sy != -1)
        resize(sx, sy);
    else
        resize(600, 300);

    if (group.readEntry("Window Maximized", false))
        showMaximized();
    else
        show();

    getSpaceData();

    exec();
}

KMountManGUI::~KMountManGUI()
{
    watcher->stop();
    delete watcher;

    KConfigGroup group(krConfig, "MountMan");

    group.writeEntry("Window Width", sizeX);
    group.writeEntry("Window Height", sizeY);
    group.writeEntry("Window Maximized", isMaximized());
    group.writeEntry("Last State", mountList->header()->saveState());
    group.writeEntry("ShowOnlyRemovable", cbShowOnlyRemovable->isChecked());
}

void KMountManGUI::resizeEvent(QResizeEvent *e)
{
    if (!isMaximized()) {
        sizeX = e->size().width();
        sizeY = e->size().height();
    }

    QDialog::resizeEvent(e);
}

QLayout *KMountManGUI::createMainPage()
{
    auto *layout = new QGridLayout();
    layout->setSpacing(10);
    mountList = new KrTreeWidget(this); // create the main container
    KConfigGroup grp(krConfig, "Look&Feel");
    mountList->setFont(grp.readEntry("Filelist Font", _FilelistFont));
    mountList->setSelectionMode(QAbstractItemView::SingleSelection);
    mountList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mountList->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QStringList labels;
    labels << i18n("Name");
    labels << i18n("Type");
    labels << i18n("Mnt.Point");
    labels << i18n("Total Size");
    labels << i18n("Free Size");
    labels << i18n("Free %");
    mountList->setHeaderLabels(labels);

    mountList->header()->setSectionResizeMode(QHeaderView::Interactive);

    grp = KConfigGroup(krConfig, "MountMan");
    if (grp.hasKey("Last State"))
        mountList->header()->restoreState(grp.readEntry("Last State", QByteArray()));
    else {
        int i = QFontMetrics(mountList->font()).horizontalAdvance("W");
        int j = QFontMetrics(mountList->font()).horizontalAdvance("0");
        j = (i > j ? i : j);

        mountList->setColumnWidth(0, j * 8);
        mountList->setColumnWidth(1, j * 4);
        mountList->setColumnWidth(2, j * 8);
        mountList->setColumnWidth(3, j * 6);
        mountList->setColumnWidth(4, j * 6);
        mountList->setColumnWidth(5, j * 5);
    }

    mountList->setAllColumnsShowFocus(true);
    mountList->header()->setSortIndicatorShown(true);

    mountList->sortItems(0, Qt::AscendingOrder);

    // now the list is created, time to fill it with data.
    //=>mountMan->forceUpdate();

    QGroupBox *box = new QGroupBox(i18n("MountMan.Info"), this);
    box->setAlignment(Qt::AlignHCenter);
    auto *vboxl = new QVBoxLayout(box);
    info = new KrFSDisplay(box);
    vboxl->addWidget(info);
    info->resize(info->width(), height());

    cbShowOnlyRemovable = new QCheckBox(i18n("Show only removable devices"), this);
    cbShowOnlyRemovable->setChecked(grp.readEntry("ShowOnlyRemovable", false));
    connect(cbShowOnlyRemovable, &QCheckBox::stateChanged, this, &KMountManGUI::updateList);

    layout->addWidget(box, 0, 0);
    layout->addWidget(cbShowOnlyRemovable, 1, 0);
    layout->addWidget(mountList, 0, 1, 2, 1);

    return layout;
}

void KMountManGUI::getSpaceData()
{
    fileSystems.clear();
    KrMountDetector::getInstance()->hasMountsChanged();

    mounted = KMountPoint::currentMountPoints();
    possible = KMountPoint::possibleMountPoints();
    if (mounted.size() == 0) { // nothing is mounted
        addNonMounted();
        updateList(); // let's continue
        return;
    }

    // Potentially long running
    this->setCursor(Qt::WaitCursor);
    auto *signalsToWaitFor = new QAtomicInteger(0);
    auto *eventLoop = new QEventLoop(this);
    for (auto &it : mounted) {
        // don't bother with invalid file systems
        if (mountMan->invalidFilesystem(it->mountType())) {
            continue;
        }
        fsData data;

        data.setMntPoint(it->mountPoint());
        data.setMounted(true);
        data.setName(it->mountedFrom());
        data.setType(it->mountType());
        KIO::FileSystemFreeSpaceJob *job = KIO::fileSystemFreeSpace(QUrl::fromLocalFile(it->mountPoint()));
        Q_ASSERT(job != nullptr);

        signalsToWaitFor->operator++();
        connect(job, &KIO::FileSystemFreeSpaceJob::finished, this, [this, data, eventLoop, signalsToWaitFor](KJob *job) {
            this->freeSpaceResult(job, data);

            if (!signalsToWaitFor->deref()) {
                eventLoop->quit(); // all done
            }
        });
    }

    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, eventLoop, &QEventLoop::quit);
    timer.start(fileSystemsFreeSpaceTimeout);

    eventLoop->exec(); // wait for signals until quit()

    this->setCursor(Qt::ArrowCursor);
    addNonMounted();
    updateList();
}

void KMountManGUI::freeSpaceResult(KJob *job, fsData data)
{
    if (!job->error()) {
        KIO::FileSystemFreeSpaceJob *freeSpaceJob = qobject_cast<KIO::FileSystemFreeSpaceJob *>(job);
        Q_ASSERT(freeSpaceJob != nullptr);
        // Set the missing information, with the assumption the caller already set the rest
        data.setTotalBlks(freeSpaceJob->size() / 1024);
        data.setFreeBlks(freeSpaceJob->availableSize() / 1024);

        fileSystems.append(data);
    } else {
        // How can we signal the error
        data.setTotalBlks(0);
        data.setFreeBlks(0);
        fileSystems.append(data);
    }
}

void KMountManGUI::addNonMounted()
{
    // handle the non-mounted ones
    for (auto &it : possible) {
        // make sure we don't add things we've already added
        if (KMountMan::findInListByMntPoint(mounted, it->mountPoint())) {
            continue;
        } else {
            fsData data;
            data.setMntPoint(it->mountPoint());
            data.setMounted(false);
            data.setType(it->mountType());
            data.setName(it->mountedFrom());

            if (mountMan->invalidFilesystem(data.type()))
                continue;

            fileSystems.append(data);
        }
    }
}

void KMountManGUI::addItemToMountList(KrTreeWidget *lst, fsData &fs, const QList<Solid::Device> &blockDevices) const
{
    const std::function deviceGetter = [blockDevices]() noexcept {
        return blockDevices;
    };
    const auto udi = KMountMan::findUdiForPath(fs.mntPoint(), Solid::DeviceInterface::StorageAccess, deviceGetter);
    Solid::Device device(udi);

    if (cbShowOnlyRemovable->isChecked() && !mountMan->removable(device))
        return;
    const bool mtd = fs.mounted();

    const QString tSize = QString("%1").arg(KIO::convertSizeFromKiB(fs.totalBlks()));
    const QString fSize = QString("%1").arg(KIO::convertSizeFromKiB(fs.freeBlks()));
    const QString sPrct = QString("%1%").arg(100 - (fs.usedPerct()));
    auto *item = new QTreeWidgetItem(lst);
    item->setText(0, fs.name());
    item->setText(1, fs.type());
    item->setText(2, fs.mntPoint());
    item->setText(3, (mtd ? tSize : QString("N/A")));
    item->setText(4, (mtd ? fSize : QString("N/A")));
    item->setText(5, (mtd ? sPrct : QString("N/A")));
    const auto *vol = device.as<Solid::StorageVolume>();

    QString iconName;
    if (device.isValid())
        iconName = device.icon();
    else if (mountMan->networkFilesystem(fs.type()))
        iconName = "folder-remote";

    QStringList overlays;
    if (mtd) {
        overlays << "emblem-mounted";
    } else {
        overlays << QString(); // We have to guarantee the placement of the next emblem
    }
    if (vol && vol->usage() == Solid::StorageVolume::Encrypted) {
        overlays << "security-high";
    }
    item->setIcon(0, Icon(iconName, overlays));
}

void KMountManGUI::updateList()
{
    QString currentMP;
    int currentIdx = 0;
    QTreeWidgetItem *currentItem = mountList->currentItem();
    if (currentItem) {
        currentMP = getMntPoint(currentItem);
        currentIdx = mountList->indexOfTopLevelItem(currentItem);
    }

    mountList->clearSelection();
    mountList->clear();

    // getting the list of devices is slow: do it one time for all items
    const QList<Solid::Device> blockDevices = Solid::Device::listFromType(Solid::DeviceInterface::Block);
    for (auto &fileSystem : fileSystems) {
        addItemToMountList(mountList, fileSystem, blockDevices);
    }

    currentItem = mountList->topLevelItem(currentIdx);
    for (int i = 0; i < mountList->topLevelItemCount(); i++) {
        QTreeWidgetItem *item = mountList->topLevelItem(i);
        if (getMntPoint(item) == currentMP)
            currentItem = item;
    }
    if (!currentItem)
        currentItem = mountList->topLevelItem(0);
    mountList->setCurrentItem(currentItem);
    changeActive(currentItem);

    mountList->setFocus();

    watcher->setSingleShot(true);
    watcher->start(WATCHER_DELAY); // starting the watch timer ( single shot )
}

void KMountManGUI::checkMountChange()
{
    if (KrMountDetector::getInstance()->hasMountsChanged())
        getSpaceData();
    watcher->setSingleShot(true);
    watcher->start(WATCHER_DELAY); // starting the watch timer ( single shot )
}

void KMountManGUI::doubleClicked(QTreeWidgetItem *i)
{
    if (!i)
        return; // we don't want to refresh to swap, do we ?

    // change the active panel to this mountpoint
    mountMan->emitRefreshPanel(QUrl::fromLocalFile(getMntPoint(i)));
    close();
}

void KMountManGUI::changeActive()
{
    QList<QTreeWidgetItem *> seld = mountList->selectedItems();
    if (seld.count() > 0)
        changeActive(seld[0]);
}

// when user clicks on a filesystem, change information
void KMountManGUI::changeActive(QTreeWidgetItem *i)
{
    if (!i) {
        if (info) {
            info->setEmpty(true);
            info->update();
        }
        mountButton->setEnabled(false);
        ejectButton->setEnabled(false);
        return;
    }

    fsData *system = getFsData(i);

    info->setAlias(system->mntPoint());
    info->setRealName(system->name());
    info->setMounted(system->mounted());
    info->setEmpty(false);
    info->setTotalSpace(system->totalBlks());
    info->setFreeSpace(system->freeBlks());
    info->repaint();

    if (system->mounted())
        mountButton->setText(i18n("&Unmount"));
    else
        mountButton->setText(i18n("&Mount"));

    ejectButton->setEnabled(mountMan->ejectable(system->mntPoint()));
    mountButton->setEnabled(true);
}

// called when right-clicked on a filesystem
void KMountManGUI::clicked(QTreeWidgetItem *item, const QPoint &pos)
{
    // these are the values that will exist in the menu
#define MOUNT_ID 90
#define UNMOUNT_ID 91
#define FORMAT_ID 93
#define EJECT_ID 94
    //////////////////////////////////////////////////////////
    if (!item)
        return;

    fsData *system = getFsData(item);
    // create the menu
    QMenu popup;
    popup.setTitle(i18n("MountMan"));
    if (!system->mounted()) {
        QAction *mountAct = popup.addAction(i18n("Mount"));
        mountAct->setData(QVariant(MOUNT_ID));
        bool enable = !(mountMan->nonmountFilesystem(system->type(), system->mntPoint()));
        mountAct->setEnabled(enable);
    } else {
        QAction *umountAct = popup.addAction(i18n("Unmount"));
        umountAct->setData(QVariant(UNMOUNT_ID));
        bool enable = !(mountMan->nonmountFilesystem(system->type(), system->mntPoint()));
        umountAct->setEnabled(enable);
    }
    if (mountMan->ejectable(system->mntPoint()))
        //  if (system->type()=="iso9660" || mountMan->followLink(system->name()).left(2)=="cd")
        popup.addAction(i18n("Eject"))->setData(QVariant(EJECT_ID));
    else {
        QAction *formatAct = popup.addAction(i18n("Format"));
        formatAct->setData(QVariant(FORMAT_ID));
        formatAct->setEnabled(false);
    }

    QString mountPoint = system->mntPoint();

    QAction *res = popup.exec(pos);
    int result = -1;

    if (res && res->data().canConvert<int>())
        result = res->data().toInt();

    // check out the user's option
    switch (result) {
    case -1:
        return; // the user clicked outside of the menu
    case MOUNT_ID:
    case UNMOUNT_ID:
        mountMan->toggleMount(mountPoint);
        break;
    case FORMAT_ID:
        break;
    case EJECT_ID:
        mountMan->eject(mountPoint);
        break;
    }
}

void KMountManGUI::slotToggleMount()
{
    QTreeWidgetItem *item = mountList->currentItem();
    if (item) {
        mountMan->toggleMount(getFsData(item)->mntPoint());
    }
}

void KMountManGUI::slotEject()
{
    QTreeWidgetItem *item = mountList->currentItem();
    if (item) {
        mountMan->eject(getFsData(item)->mntPoint());
    }
}

fsData *KMountManGUI::getFsData(QTreeWidgetItem *item)
{
    for (auto &fileSystem : fileSystems) {
        // the only thing which is unique is the mount point
        if (fileSystem.mntPoint() == getMntPoint(item)) {
            return &fileSystem;
        }
    }
    // this point shouldn't be reached
    abort();
    return nullptr;
}

QString KMountManGUI::getMntPoint(QTreeWidgetItem *item)
{
    return item->text(2); // text(2) ? ugly ugly ugly
}

KrMountDetector::KrMountDetector()
{
    hasMountsChanged();
}

bool KrMountDetector::hasMountsChanged()
{
    bool result = false;
#ifndef BSD
    QFileInfo mtabInfo(MTAB);
    if (!mtabInfo.exists() || mtabInfo.isSymLink()) { // if mtab is a symlimk to /proc/mounts the mtime is unusable
#endif
        KMountPoint::List mountPoints = KMountPoint::currentMountPoints(KMountPoint::NeedRealDeviceName);
        QCryptographicHash md5(QCryptographicHash::Md5);
        for (auto &mountPoint : mountPoints) {
            md5.addData(mountPoint->mountedFrom().toUtf8());
            md5.addData(mountPoint->realDeviceName().toUtf8());
            md5.addData(mountPoint->mountPoint().toUtf8());
            md5.addData(mountPoint->mountType().toUtf8());
        }
        QString s = md5.result();
        result = s != checksum;
        checksum = s;
#ifndef BSD
    } else {
        result = mtabInfo.lastModified() != lastMtab;
        lastMtab = mtabInfo.lastModified();
    }
#endif
    return result;
}

KrMountDetector krMountDetector;
KrMountDetector *KrMountDetector::getInstance()
{
    return &krMountDetector;
}
