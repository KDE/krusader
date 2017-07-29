/*****************************************************************************
 * Copyright (C) 2003 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2003 Rafi Yanai <yanai@users.sourceforge.net>               *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#include "panelcontextmenu.h"

// QtGui
#include <QPixmap>

#include <KCoreAddons/KProcess>
#include <KI18n/KLocalizedString>
#include <KIOWidgets/KRun>
#include <KIconThemes/KIconLoader>
#include <KWidgetsAddons/KMessageBox>
#include <KService/KMimeTypeTrader>
#include <KService/KToolInvocation>
#include <KXmlGui/KActionCollection>
#include <KIOCore/KFileItemListProperties>
#include <KIOWidgets/KAbstractFileItemActionPlugin>
#include <KCoreAddons/KPluginMetaData>

#include "krpreviewpopup.h"
#include "listpanel.h"
#include "listpanelactions.h"
#include "panelfunc.h"
#include "PanelView/krview.h"
#include "PanelView/krviewitem.h"
#include "../defaults.h"
#include "../krservices.h"
#include "../krslots.h"
#include "../krusader.h"
#include "../krusaderview.h"
#include "../panelmanager.h"
#include "../Archive/krarchandler.h"
#include "../FileSystem/fileitem.h"
#include "../FileSystem/filesystem.h"
#include "../FileSystem/krtrashhandler.h"
#include "../MountMan/kmountman.h"
#include "../UserAction/useractionpopupmenu.h"

void PanelContextMenu::run(const QPoint &pos, KrPanel *panel)
{
    PanelContextMenu menu(panel);
    QAction * res = menu.exec(pos);
    int result = -1;
    if (res && res->data().canConvert<int>())
        result = res->data().toInt();
    menu.performAction(result);
}

/**
 * Copied from dolphin/src/dolphincontextmenu.cpp and modified to add only compress and extract submenus.
 */
void PanelContextMenu::addCompressAndExtractPluginActions()
{
    KFileItemListProperties props(_items);

    QVector<KPluginMetaData> jsonPlugins = KPluginLoader::findPlugins("kf5/kfileitemaction",
                                                                      [=](const KPluginMetaData& metaData) {
        return metaData.pluginId() == "compressfileitemaction" || metaData.pluginId() == "extractfileitemaction";
    });

    foreach (const KPluginMetaData &jsonMetadata, jsonPlugins) {
        KAbstractFileItemActionPlugin* abstractPlugin = KPluginLoader(jsonMetadata.fileName())
                                                            .factory()->create<KAbstractFileItemActionPlugin>();
        if (abstractPlugin) {
            abstractPlugin->setParent(this);
            addActions(abstractPlugin->actions(props, this));
        }
    }
}

PanelContextMenu::PanelContextMenu(KrPanel *krPanel, QWidget *parent)
    : QMenu(parent), panel(krPanel)
{
    // selected file names
    const QStringList fileNames = panel->gui->getSelectedNames();

    // file items
    QList<FileItem*> files;
    for (const QString fileName : fileNames) {
        files.append(panel->func->files()->getFileItem(fileName));
    }

    // KFileItems
    for (FileItem *file : files) {
        _items.append(KFileItem(file->getUrl(), file->getMime(),
                                file->getMode()));
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
    QAction * openAct = addAction(i18n("Open/Run"));
    openAct->setData(QVariant(OPEN_ID));
    if (!multipleSelections) { // meaningful only if one file is selected
        KrViewItemList viewItems;
        panel->view->getSelectedKrViewItems(&viewItems);
        openAct->setIcon(viewItems.first()->icon());
        openAct->setText(file->isExecutable() && !file->isDir() ?
                             i18n("Run") : i18n("Open"));
        // open in a new tab (if folder)
        if (file->isDir()) {
            QAction * openTab = addAction(i18n("Open in New Tab"));
            openTab->setData(QVariant(OPEN_TAB_ID));
            openTab->setIcon(krLoader->loadIcon("tab-new", KIconLoader::Panel));
            openTab->setText(i18n("Open in New Tab"));
        }
        // if the file can be browsed as archive...
        if (!panel->func->browsableArchivePath(file->getName()).isEmpty()
            // ...but user disabled archive browsing...
            && (!KConfigGroup(krConfig, "Archives")
                .readEntry("ArchivesAsDirectories", _ArchivesAsDirectories)
             // ...or the file is not a standard archive (e.g. odt, docx, etc.)...
             || !KRarcHandler::arcSupported(file->getMime()))) {
            // ...it will not be browsed as a directory by default, but add an option for it
            QAction *browseAct = addAction(i18n("Browse"));
            browseAct->setData(QVariant(BROWSE_ID));
            browseAct->setIcon(krLoader->loadIcon("", KIconLoader::Panel));
            browseAct->setText(i18n("Browse Archive"));
        }
        addSeparator();
    }

    // ------------- Preview - local filesystem only ?
    if (panel->func->files()->isLocal()) {
        // create the preview popup
        KrPreviewPopup preview;
        preview.setUrls(panel->func->files()->getUrls(fileNames));
        QAction *previewAction = addMenu(&preview);
        previewAction->setData(QVariant(PREVIEW_ID));
        previewAction->setText(i18n("Preview"));
        previewAction->setIcon(krLoader->loadIcon("document-print-preview", KIconLoader::Small));
    }

    // -------------- Open with: try to find-out which apps can open the file
    QSet<QString> uniqueMimeTypes;
    for (FileItem *file : files)
        uniqueMimeTypes.insert(file->getMime());
    const QStringList mimeTypes = uniqueMimeTypes.toList();

    offers = mimeTypes.count() == 1 ?
                 KMimeTypeTrader::self()->query(mimeTypes.first()) :
                 KFileItemActions::associatedApplications(mimeTypes, QString());

    if (!offers.isEmpty()) {
        QMenu *openWithMenu = new QMenu(this);
        for (int i = 0; i < offers.count(); ++i) {
            QExplicitlySharedDataPointer<KService> service = offers[i];
            if (service->isValid() && service->isApplication()) {
                openWithMenu->addAction(krLoader->loadIcon(service->icon(), KIconLoader::Small),
                                   service->name())->setData(QVariant(SERVICE_LIST_ID + i));
            }
        }
        openWithMenu->addSeparator();
        if (!multipleSelections && file->isDir())
            openWithMenu->addAction(krLoader->loadIcon("utilities-terminal", KIconLoader::Small),
                               i18n("Terminal"))->setData(QVariant(OPEN_TERM_ID));
        openWithMenu->addAction(i18n("Other..."))->setData(QVariant(CHOOSE_ID));
        QAction *openWithAction = addMenu(openWithMenu);
        openWithAction->setText(i18n("Open With"));
        openWithAction->setIcon(krLoader->loadIcon("document-open", KIconLoader::Small));
        addSeparator();
    }

    // --------------- user actions
    QAction *userAction = new UserActionPopupMenu(file->getUrl(), this);
    userAction->setText(i18n("User Actions"));
    addAction(userAction);

    // --------------- compress/extract actions
    // workaround for Bug 372999: application freezes very long time if many files are selected
    if (_items.length() < 1000)
        // add compress and extract plugins (if available)
        addCompressAndExtractPluginActions();

    // --------------- KDE file item actions
    // NOTE: design and usability problem here. Services disabled in kservicemenurc settings won't
    // be added to the menu. But Krusader does not provide a way do change these settings (only
    // Dolphin does).
    KFileItemActions *fileItemActions = new KFileItemActions(this);
    fileItemActions->setItemListProperties(KFileItemListProperties(_items));
    fileItemActions->setParentWidget(MAIN_VIEW);
    fileItemActions->addServiceActionsTo(this);

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
    if (KConfigGroup(krConfig, "General").readEntry("Move To Trash", _MoveToTrash)
        && panel->func->files()->canMoveToTrash(fileNames)) {
        addAction(krLoader->loadIcon("user-trash", KIconLoader::Small),
                  i18n("Move to Trash"))->setData(QVariant(TRASH_ID));
    }
    // -------- DELETE
    addAction(krLoader->loadIcon("edit-delete", KIconLoader::Small),
              i18n("Delete"))->setData(QVariant(DELETE_ID));
    // -------- SHRED - only one file
    /*      if ( panel->func->files() ->getType() == filesystem:fileSystemM_NORMAL &&
                !fileitem->isDir() && !multipleSelections )
             addAction( i18n( "Shred" ) )->setData( QVariant( SHRED_ID ) );*/

    // ---------- link handling
    // create new shortcut or redirect links - only on local directories:
    if (panel->func->files()->isLocal()) {
        addSeparator();
        QMenu *linkMenu = new QMenu(this);
        linkMenu->addAction(i18n("New Symlink..."))->setData(QVariant(NEW_SYMLINK_ID));
        linkMenu->addAction(i18n("New Hardlink..."))->setData(QVariant(NEW_LINK_ID));
        if (file->isSymLink()) {
            linkMenu->addAction(i18n("Redirect Link..."))->setData(QVariant(REDIRECT_LINK_ID));
        }
        QAction *linkAction = addMenu(linkMenu);
        linkAction->setText(i18n("Link Handling"));
        linkAction->setIcon(krLoader->loadIcon("insert-link", KIconLoader::Small));
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
        addAction(krLoader->loadIcon("mail-send", KIconLoader::Small),
                  i18n("Send by Email"))->setData(QVariant(SEND_BY_EMAIL_ID));
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
    QMenu *createNewMenu = new QMenu(this);

    createNewMenu->addAction(krLoader->loadIcon("folder", KIconLoader::Small),
                             i18n("Folder..."))->setData(QVariant(MKDIR_ID));
    createNewMenu->addAction(krLoader->loadIcon("text-plain", KIconLoader::Small),
                             i18n("Text File..."))->setData(QVariant(NEW_TEXT_FILE_ID));

    QAction *newMenuAction = addMenu(createNewMenu);
    newMenuAction->setText(i18n("Create New"));
    newMenuAction->setIcon(krLoader->loadIcon("document-new", KIconLoader::Small));
}

void PanelContextMenu::performAction(int id)
{
    if (_items.isEmpty())
        return; // sanity check, empty file list

    KFileItem *item = &_items.first();

    switch (id) {
    case - 1 : // the user clicked outside of the menu
        return;
    case OPEN_TAB_ID :
        // assuming only 1 file is selected (otherwise we won't get here)
        panel->manager()->newTab(item->url(), panel);
        break;
    case OPEN_ID :
        foreach(const KFileItem &fi, _items)
            panel->func->execute(fi.name());
        break;
    case BROWSE_ID :
        panel->func->goInside(item->url().fileName());
        break;
    case COPY_ID :
        panel->func->copyFiles();
        break;
    case MOVE_ID :
        panel->func->moveFiles();
        break;
    case TRASH_ID :
        panel->func->deleteFiles(true);
        break;
    case DELETE_ID :
        panel->func->deleteFiles(false);
        break;
    case EJECT_ID :
        krMtMan.eject(item->url().adjusted(QUrl::StripTrailingSlash).path());
        break;
//     case SHRED_ID :
//        if ( KMessageBox::warningContinueCancel( krApp,
//             i18n("<qt>Do you really want to shred <b>%1</b>? Once shred, the file is gone forever.</qt>", item->name()),
//             QString(), KStandardGuiItem::cont(), KStandardGuiItem::cancel(), "Shred" ) == KMessageBox::Continue )
//           KShred::shred( panel->func->files() ->getFile( item->name() ).adjusted(QUrl::RemoveTrailingSlash).path() );
//      break;
    case OPEN_KONQ_ID :
        KToolInvocation::startServiceByDesktopName("konqueror", item->url().toDisplayString(QUrl::PreferLocalFile));
        break;
    case CHOOSE_ID : // open-with dialog
        panel->func->displayOpenWithDialog(_items.urlList());
        break;
    case MOUNT_ID :
        krMtMan.mount(item->url().adjusted(QUrl::StripTrailingSlash).path());
        break;
    case NEW_LINK_ID :
        panel->func->krlink(false);
        break;
    case NEW_SYMLINK_ID :
        panel->func->krlink(true);
        break;
    case REDIRECT_LINK_ID :
        panel->func->redirectLink();
        break;
    case EMPTY_TRASH_ID :
        KrTrashHandler::emptyTrash();
        break;
    case RESTORE_TRASHED_FILE_ID :
        KrTrashHandler::restoreTrashedFiles(_items.urlList());
    break;
    case UNMOUNT_ID :
        krMtMan.unmount(item->url().adjusted(QUrl::StripTrailingSlash).path());
        break;
    case SEND_BY_EMAIL_ID : {
        SLOTS->sendFileByEmail(_items.urlList());
        break;
    }
    case MKDIR_ID :
        panel->func->mkdir();
        break;
    case NEW_TEXT_FILE_ID:
        panel->func->editNew();
        break;
#ifdef SYNCHRONIZER_ENABLED
    case SYNC_SELECTED_ID : {
        QStringList selectedNames;
        foreach(const KFileItem &item, _items)
            selectedNames << item.name();
        if (panel->otherPanel()->view->numSelected()) {
            KrViewItemList otherItems;
            panel->otherPanel()->view->getSelectedKrViewItems(&otherItems);

            for (KrViewItemList::Iterator it2 = otherItems.begin(); it2 != otherItems.end(); ++it2) {
                QString name = (*it2) ->name();
                if (!selectedNames.contains(name))
                    selectedNames.append(name);
            }
        }
        SLOTS->slotSynchronizeDirs(selectedNames);
    }
    break;
#endif
    case OPEN_TERM_ID :
        SLOTS->runTerminal(item->url().path());
        break;
    }

    // check if something from the open-with-offered-services was selected
    if (id >= SERVICE_LIST_ID) {
        const QStringList names = panel->gui->getSelectedNames();
        panel->func->runService(*(offers[ id - SERVICE_LIST_ID ]),
                                panel->func->files()->getUrls(names));
    }
}
