/*
    SPDX-FileCopyrightText: 2003 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "panelcontextmenu.h"

// QtGui
#include <QPixmap>

#include <KAbstractFileItemActionPlugin>
#include <KActionCollection>
#include <KApplicationTrader>
#include <KFileItem>
#include <KFileItemListProperties>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginMetaData>
#include <KPluginFactory>
#include <KProcess>
#include <kio_version.h>
#include <kservice_version.h>

#include "../Archive/krarchandler.h"
#include "../FileSystem/fileitem.h"
#include "../FileSystem/filesystem.h"
#include "../FileSystem/krtrashhandler.h"
#include "../MountMan/kmountman.h"
#include "../UserAction/useractionpopupmenu.h"
#include "../defaults.h"
#include "../icon.h"
#include "../krservices.h"
#include "../krslots.h"
#include "../krusader.h"
#include "../krusaderview.h"
#include "../panelmanager.h"
#include "PanelView/krview.h"
#include "PanelView/krviewitem.h"
#include "krpreviewpopup.h"
#include "listpanel.h"
#include "listpanelactions.h"
#include "panelfunc.h"

PanelContextMenu *PanelContextMenu::run(const QPoint &pos, KrPanel *panel)
{
    auto menu = new PanelContextMenu(panel);
    QAction *res = menu->exec(pos);
    int result = res && res->data().canConvert<int>() ? res->data().toInt() : -1;
    menu->performAction(result);
    return menu;
}

PanelContextMenu::PanelContextMenu(KrPanel *krPanel, QWidget *parent)
    : QMenu(parent)
    , panel(krPanel)
{
    // selected file names
    const QStringList fileNames = panel->gui->getSelectedNames();

    // file items
    QList<FileItem *> files;
    for (const QString &fileName : fileNames) {
        files.append(panel->func->files()->getFileItem(fileName));
    }

    // KFileItems
    bool allFilesAreDirs = true;
    for (FileItem *file : files) {
        _items.append(KFileItem(file->getUrl(), file->getMime(), file->getMode()));
        allFilesAreDirs &= file->isDir();
    }

    if (files.empty()) {
        addCreateNewMenu();
        addSeparator();
        addEmptyMenuEntries();
        return;
    }

    const bool multipleSelections = files.size() > 1;

    QSet<QString> protocols;
    for (FileItem *file : files) {
        protocols.insert(file->getUrl().scheme());
    }
    const bool inTrash = protocols.contains("trash");
    const bool trashOnly = inTrash && protocols.count() == 1;

    FileItem *file = files.first();

    // ------------ the OPEN/BROWSE option - open preferred service

    // open/run - if not only multiple dirs are selected
    if (!(multipleSelections && allFilesAreDirs)) {
        QAction *openAction = new QAction(this);
        openAction->setData(QVariant(OPEN_ID));
        if (multipleSelections) {
            openAction->setText(i18n("Open/Run Files"));
        } else {
            openAction->setText(file->isExecutable() && !file->isDir() ? i18n("Run") : i18n("Open"));
            const KrViewItemList viewItems = panel->view->getSelectedKrViewItems();
            openAction->setIcon(viewItems.first()->icon());
        }
        addAction(openAction);
    }

    // open in a new tab (if only folder(s) are selected)
    if (allFilesAreDirs) {
        QAction *openTab = addAction(multipleSelections ? i18n("Open in New Tabs") : i18n("Open in New Tab"));
        openTab->setData(QVariant(OPEN_TAB_ID));
        openTab->setIcon(Icon("tab-new"));
    }

    // browse archive - if one file is selected and the file can be browsed as archive...
    if (!multipleSelections
        && !panel->func->browsableArchivePath(file->getName()).isEmpty()
        // ...but user disabled archive browsing...
        && (!KConfigGroup(krConfig, "Archives").readEntry("ArchivesAsDirectories", _ArchivesAsDirectories)
            // ...or the file is not a standard archive (e.g. odt, docx, etc.)...
            || !KrArcHandler::arcSupported(file->getMime()))) {
        // ...it will not be browsed as a directory by default, but add an option for it
        QAction *browseAct = addAction(i18n("Browse Archive"));
        browseAct->setData(QVariant(BROWSE_ID));
        browseAct->setIcon(Icon("archive-insert-directory"));
    }

    // ------------- Preview - local filesystem only ?
    if (panel->func->files()->isLocal()) {
        // create the preview popup
        KrPreviewPopup preview;
        preview.setUrls(panel->func->files()->getUrls(fileNames));
        QAction *previewAction = addMenu(&preview);
        previewAction->setData(QVariant(PREVIEW_ID));
        previewAction->setText(i18n("Preview"));
        previewAction->setIcon(Icon("document-print-preview"));
    }

    // -------------- Open with: try to find-out which apps can open the file
    QSet<QString> uniqueMimeTypes;
    for (FileItem *file : files)
        uniqueMimeTypes.insert(file->getMime());
    const QStringList mimeTypes = uniqueMimeTypes.values();

#if KSERVICE_VERSION >= QT_VERSION_CHECK(5, 83, 0)
    offers = mimeTypes.count() == 1 ? KApplicationTrader::queryByMimeType(mimeTypes.first()) : KFileItemActions::associatedApplications(mimeTypes);
#elif KSERVICE_VERSION >= QT_VERSION_CHECK(5, 68, 0)
    offers = mimeTypes.count() == 1 ? KApplicationTrader::queryByMimeType(mimeTypes.first()) : KFileItemActions::associatedApplications(mimeTypes, QString());
#else
    offers = mimeTypes.count() == 1 ? KMimeTypeTrader::self()->query(mimeTypes.first()) : KFileItemActions::associatedApplications(mimeTypes, QString());
#endif

    if (!offers.isEmpty()) {
        auto *openWithMenu = new QMenu(this);
        for (int i = 0; i < offers.count(); ++i) {
            QExplicitlySharedDataPointer<KService> service = offers[i];
            if (service->isValid() && service->isApplication()) {
                openWithMenu->addAction(Icon(service->icon()), service->name())->setData(QVariant(SERVICE_LIST_ID + i));
            }
        }
        openWithMenu->addSeparator();
        if (!multipleSelections && file->isDir()) {
            openWithMenu->addAction(Icon("utilities-terminal"), i18n("Terminal"))->setData(QVariant(OPEN_TERM_ID));
        }
        openWithMenu->addAction(i18n("Other..."))->setData(QVariant(CHOOSE_ID));
        QAction *openWithAction = addMenu(openWithMenu);
        openWithAction->setText(i18n("Open With"));
        openWithAction->setIcon(Icon("document-open"));
    }

    addSeparator();

    // --------------- user actions
    QAction *userAction = new UserActionPopupMenu(file->getUrl(), this);
    userAction->setText(i18n("User Actions"));
    addAction(userAction);

    // --------------- KDE file item actions
    // workaround for Bug 372999: application freezes very long time if many files are selected
    if (_items.length() < 1000) {
        // NOTE: design and usability problem here. Services disabled in kservicemenurc settings won't
        // be added to the menu. But Krusader does not provide a way do change these settings (only
        // Dolphin does).
        auto *fileItemActions = new KFileItemActions(this);
        fileItemActions->setItemListProperties(KFileItemListProperties(_items));
        fileItemActions->setParentWidget(MAIN_VIEW);
#if KIO_VERSION >= QT_VERSION_CHECK(5, 79, 0)
        fileItemActions->addActionsTo(this);
#else
        fileItemActions->addServiceActionsTo(this);
#endif
    }

    addSeparator();

    // ------------- 'create new' submenu
    addCreateNewMenu();
    addSeparator();

    // ---------- COPY
    addAction(panel->gui->actions()->actCopyF5);
    // ------- MOVE
    addAction(panel->gui->actions()->actMoveF6);
    // ------- RENAME - only one file
    if (!multipleSelections && !inTrash) {
        addAction(panel->gui->actions()->actRenameF2);
    }

    // -------- MOVE TO TRASH
    if (KConfigGroup(krConfig, "General").readEntry("Move To Trash", _MoveToTrash) && panel->func->files()->canMoveToTrash(fileNames)) {
        addAction(Icon("user-trash"), i18n("Move to Trash"))->setData(QVariant(TRASH_ID));
    }
    // -------- DELETE
    addAction(Icon("edit-delete"), i18n("Delete"))->setData(QVariant(DELETE_ID));
    // -------- SHRED - only one file
    /*      if ( panel->func->files() ->getType() == filesystem:fileSystemM_NORMAL &&
                !fileitem->isDir() && !multipleSelections )
             addAction( i18n( "Shred" ) )->setData( QVariant( SHRED_ID ) );*/

    // ---------- link handling
    // create new shortcut or redirect links - only on local directories:
    if (panel->func->files()->isLocal()) {
        addSeparator();
        auto *linkMenu = new QMenu(this);
        linkMenu->addAction(i18n("New Symlink..."))->setData(QVariant(NEW_SYMLINK_ID));
        linkMenu->addAction(i18n("New Hardlink..."))->setData(QVariant(NEW_LINK_ID));
        if (file->isSymLink()) {
            linkMenu->addAction(i18n("Redirect Link..."))->setData(QVariant(REDIRECT_LINK_ID));
        }
        QAction *linkAction = addMenu(linkMenu);
        linkAction->setText(i18n("Link Handling"));
        linkAction->setIcon(Icon("insert-link"));
    }
    addSeparator();

    // ---------- calculate space
    if (panel->func->files()->isLocal() && (file->isDir() || multipleSelections))
        addAction(panel->gui->actions()->actCalculate);

    // ---------- mount/umount/eject
    if (panel->func->files()->isLocal() && file->isDir() && !multipleSelections) {
        const QString selectedDirectoryPath = file->getUrl().path();
        if (krMtMan.getStatus(selectedDirectoryPath) == KMountMan::MOUNTED)
            addAction(i18n("Unmount"))->setData(QVariant(UNMOUNT_ID));
        else if (krMtMan.getStatus(selectedDirectoryPath) == KMountMan::NOT_MOUNTED)
            addAction(i18n("Mount"))->setData(QVariant(MOUNT_ID));
        if (krMtMan.ejectable(selectedDirectoryPath))
            addAction(i18n("Eject"))->setData(QVariant(EJECT_ID));
    }

    // --------- send by mail
    if (KrServices::supportedTools().contains("MAIL") && !file->isDir()) {
        addAction(Icon("mail-send"), i18n("Send by Email"))->setData(QVariant(SEND_BY_EMAIL_ID));
    }

    // --------- empty trash
    if (trashOnly) {
        addAction(i18n("Restore"))->setData(QVariant(RESTORE_TRASHED_FILE_ID));
        addAction(i18n("Empty Trash"))->setData(QVariant(EMPTY_TRASH_ID));
    }

#ifdef SYNCHRONIZER_ENABLED
    // --------- synchronize
    if (panel->view->numSelected()) {
        addAction(i18n("Synchronize Selected Files..."))->setData(QVariant(SYNC_SELECTED_ID));
    }
#endif

    // --------- copy/paste
    addSeparator();
    addAction(panel->gui->actions()->actCut);
    addAction(panel->gui->actions()->actCopy);
    addAction(panel->gui->actions()->actPaste);
    addSeparator();

    // --------- properties
    addAction(panel->gui->actions()->actProperties);
}

void PanelContextMenu::addEmptyMenuEntries()
{
    addAction(panel->gui->actions()->actPaste);
}

void PanelContextMenu::addCreateNewMenu()
{
    auto *createNewMenu = new QMenu(this);
    createNewMenu->addAction(panel->gui->actions()->actNewFolderF7);
    createNewMenu->addAction(panel->gui->actions()->actNewFileShiftF4);

    QAction *newMenuAction = addMenu(createNewMenu);
    newMenuAction->setText(i18n("Create New"));
    newMenuAction->setIcon(Icon("document-new"));
}

void PanelContextMenu::performAction(int id)
{
    const QUrl singleURL = _items.isEmpty() ? QUrl() : _items.first().url();

    switch (id) {
    case -1: // the user clicked outside of the menu
        return;
    case OPEN_TAB_ID:
        for (const KFileItem &fileItem : _items) {
            panel->manager()->duplicateTab(fileItem.url(), panel);
        }
        break;
    case OPEN_ID:
        for (const KFileItem &fileItem : _items) {
            // do not open dirs if multiple files are selected
            if (_items.size() == 1 || !fileItem.isDir()) {
                panel->func->execute(fileItem.name());
            }
        }
        break;
    case BROWSE_ID:
        panel->func->goInside(singleURL.fileName());
        break;
    case TRASH_ID:
        panel->func->deleteFiles(true);
        break;
    case DELETE_ID:
        panel->func->deleteFiles(false);
        break;
    case EJECT_ID:
        krMtMan.eject(singleURL.adjusted(QUrl::StripTrailingSlash).path());
        break;
    case CHOOSE_ID: // open-with dialog
        panel->func->displayOpenWithDialog(_items.urlList());
        break;
    case MOUNT_ID:
        krMtMan.mount(singleURL.adjusted(QUrl::StripTrailingSlash).path());
        break;
    case NEW_LINK_ID:
        panel->func->krlink(false);
        break;
    case NEW_SYMLINK_ID:
        panel->func->krlink(true);
        break;
    case REDIRECT_LINK_ID:
        panel->func->redirectLink();
        break;
    case EMPTY_TRASH_ID:
        KrTrashHandler::emptyTrash();
        break;
    case RESTORE_TRASHED_FILE_ID:
        KrTrashHandler::restoreTrashedFiles(_items.urlList());
        break;
    case UNMOUNT_ID:
        krMtMan.unmount(singleURL.adjusted(QUrl::StripTrailingSlash).path());
        break;
    case SEND_BY_EMAIL_ID: {
        SLOTS->sendFileByEmail(_items.urlList());
        break;
    }
#ifdef SYNCHRONIZER_ENABLED
    case SYNC_SELECTED_ID: {
        QStringList selectedNames;
        for (const KFileItem &item : _items) {
            selectedNames.append(item.name());
        }
        const KrViewItemList otherItems = panel->otherPanel()->view->getSelectedKrViewItems();
        for (KrViewItem *otherItem : otherItems) {
            const QString &name = otherItem->name();
            if (!selectedNames.contains(name)) {
                selectedNames.append(name);
            }
        }
        SLOTS->slotSynchronizeDirs(selectedNames);
    } break;
#endif
    case OPEN_TERM_ID:
        SLOTS->runTerminal(singleURL.path());
        break;
    }

    // check if something from the open-with-offered-services was selected
    if (id >= SERVICE_LIST_ID) {
        const QStringList names = panel->gui->getSelectedNames();
        panel->func->runService(*(offers[id - SERVICE_LIST_ID]), panel->func->files()->getUrls(names));
    }
}
