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

#include "krpopupmenu.h"

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

#include "listpanel.h"
#include "krview.h"
#include "krviewitem.h"
#include "panelfunc.h"
#include "listpanelactions.h"
#include "../krservices.h"
#include "../defaults.h"
#include "../MountMan/kmountman.h"
#include "../krusader.h"
#include "../krslots.h"
#include "../krusaderview.h"
#include "../panelmanager.h"
#include "../UserAction/useractionpopupmenu.h"
#include "../Archive/krarchandler.h"
#include "../VFS/krtrashhandler.h"

void KrPopupMenu::run(const QPoint &pos, KrPanel *panel)
{
    KrPopupMenu menu(panel);
    QAction * res = menu.exec(pos);
    int result = -1;
    if (res && res->data().canConvert<int>())
        result = res->data().toInt();
    menu.performAction(result);
}

/**
 * Copied from dolphin/src/dolphincontextmenu.cpp and modified to add only compress and extract submenus.
 */
void KrPopupMenu::addCompressAndExtractPluginActions()
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

KrPopupMenu::KrPopupMenu(KrPanel *thePanel, QWidget *parent)
    : QMenu(parent), panel(thePanel), empty(false), multipleSelections(false)
{
    // selected file names
    const QStringList fileNames = panel->gui->getSelectedNames();

    // vfiles
    QList<vfile*> files;
    for (const QString fileName : fileNames) {
        files.append(panel->func->files()->getVfile(fileName));
    }

    // KFileItems
    for (vfile *file : files) {
        _items.append(KFileItem(file->vfile_getUrl(), file->vfile_getMime(),
                                file->vfile_getMode()));
    }

    if (files.empty()) {
        addCreateNewMenu();
        addSeparator();
        addEmptyMenuEntries();
        return;
    } else if (files.size() > 1) {
        multipleSelections = true;
    }

    QSet<QString> protocols;
    for (vfile *file : files) {
        protocols.insert(file->vfile_getUrl().scheme());
    }
    const bool inTrash = protocols.contains("trash");
    const bool trashOnly = inTrash && protocols.count() == 1;

    vfile *file = files.first();

    // ------------ the OPEN/BROWSE option - open preferred service
    QAction * openAct = addAction(i18n("Open/Run"));
    openAct->setData(QVariant(OPEN_ID));
    if (!multipleSelections) { // meaningful only if one file is selected
        KrViewItemList viewItems;
        panel->view->getSelectedKrViewItems(&viewItems);
        openAct->setIcon(viewItems.first()->icon());
        openAct->setText(file->vfile_isExecutable() && !file->vfile_isDir() ?
                             i18n("Run") : i18n("Open"));
        // open in a new tab (if folder)
        if (file->vfile_isDir()) {
            QAction * openTab = addAction(i18n("Open in New Tab"));
            openTab->setData(QVariant(OPEN_TAB_ID));
            openTab->setIcon(krLoader->loadIcon("tab-new", KIconLoader::Panel));
            openTab->setText(i18n("Open in New Tab"));
        }
        // if the file can be browsed as archive...
        if (!panel->func->browsableArchivePath(file->vfile_getName()).isEmpty()
            // ...but user disabled archive browsing...
            && (!KConfigGroup(krConfig, "Archives")
                .readEntry("ArchivesAsDirectories", _ArchivesAsDirectories)
             // ...or the file is not a standard archive (e.g. odt, docx, etc.)...
             || !KRarcHandler::arcSupported(file->vfile_getMime()))) {
            // ...it will not be browsed as a directory by default, but add an option for it
            QAction *browseAct = addAction(i18n("Browse"));
            browseAct->setData(QVariant(BROWSE_ID));
            browseAct->setIcon(krLoader->loadIcon("", KIconLoader::Panel));
            browseAct->setText(i18n("Browse Archive"));
        }
        addSeparator();
    }

    // ------------- Preview - local vfs only ?
    if (panel->func->files()->isLocal()) {
        // create the preview popup
        preview.setUrls(panel->func->files()->getUrls(fileNames));
        QAction *pAct = addMenu(&preview);
        pAct->setData(QVariant(PREVIEW_ID));
        pAct->setText(i18n("Preview"));
    }

    // -------------- Open with: try to find-out which apps can open the file
    // this too, is meaningful only if one file is selected or if all the files
    // have the same mimetype !
    QString mime = file->vfile_getMime();
    // check if all files have the same mimetype
    for (vfile *file : files) {
        if (file->vfile_getMime() != mime) {
            mime.clear();
            break;
        }
    }
    if (!mime.isEmpty()) {
        offers = KMimeTypeTrader::self()->query(mime);
        for (int i = 0; i < offers.count(); ++i) {
            QExplicitlySharedDataPointer<KService> service = offers[i];
            if (service->isValid() && service->isApplication()) {
                openWith.addAction(krLoader->loadIcon(service->icon(), KIconLoader::Small),
                                   service->name())->setData(QVariant(SERVICE_LIST_ID + i));
            }
        }
        openWith.addSeparator();
        if (file->vfile_isDir())
            openWith.addAction(krLoader->loadIcon("utilities-terminal", KIconLoader::Small),
                               i18n("Terminal"))->setData(QVariant(OPEN_TERM_ID));
        openWith.addAction(i18n("Other..."))->setData(QVariant(CHOOSE_ID));
        QAction *owAct = addMenu(&openWith);
        owAct->setData(QVariant(OPEN_WITH_ID));
        owAct->setText(i18n("Open With"));
        addSeparator();
    }

    // --------------- user actions
    QAction *userAction = new UserActionPopupMenu(file->vfile_getUrl());
    userAction->setText(i18n("User Actions"));
    addAction(userAction);

    // workaround for Bug 372999: application freezes very long time if many files are selected
    if (_items.length() < 1000)
        // add compress and extract plugins (if available)
        addCompressAndExtractPluginActions();

    // NOTE: design and usability problem here. Services disabled in kservicemenurc settings won't
    // be added to the menu. But Krusader does not provide a way do change these settings (only
    // Dolphin does).
    fileItemActions.setItemListProperties(KFileItemListProperties(_items));
    fileItemActions.addServiceActionsTo(this);

    addSeparator();

    // ------------- 'create new' submenu
    addCreateNewMenu();
    addSeparator();

    // ---------- COPY
    addAction(i18n("Copy..."))->setData(QVariant(COPY_ID));
    // ------- MOVE
    addAction(i18n("Move..."))->setData(QVariant(MOVE_ID));
    // ------- RENAME - only one file
    if (!multipleSelections && !inTrash)
        addAction(i18n("Rename"))->setData(QVariant(RENAME_ID));

    // -------- MOVE TO TRASH
    if (KConfigGroup(krConfig, "General").readEntry("Move To Trash", _MoveToTrash)
        && panel->func->files()->canMoveToTrash(fileNames))
        addAction(i18n("Move to Trash"))->setData(QVariant(TRASH_ID));
    // -------- DELETE
    addAction(i18n("Delete"))->setData(QVariant(DELETE_ID));
    // -------- SHRED - only one file
    /*      if ( panel->func->files() ->vfs_getType() == vfs::VFS_NORMAL &&
                !vf->vfile_isDir() && !multipleSelections )
             addAction( i18n( "Shred" ) )->setData( QVariant( SHRED_ID ) );*/

    // ---------- link handling
    // create new shortcut or redirect links - only on local directories:
    if (panel->func->files()->isLocal()) {
        addSeparator();
        linkPopup.addAction(i18n("New Symlink..."))->setData(QVariant(NEW_SYMLINK_ID));
        linkPopup.addAction(i18n("New Hardlink..."))->setData(QVariant(NEW_LINK_ID));
        if (file->vfile_isSymLink())
            linkPopup.addAction(i18n("Redirect Link..."))->setData(QVariant(REDIRECT_LINK_ID));
        QAction *linkAct = addMenu(&linkPopup);
        linkAct->setData(QVariant(LINK_HANDLING_ID));
        linkAct->setText(i18n("Link Handling"));
    }
    addSeparator();

    // ---------- calculate space
    if (panel->func->files()->isLocal() && (file->vfile_isDir() || multipleSelections))
        addAction(panel->gui->actions()->actCalculate);

    // ---------- mount/umount/eject
    if (panel->func->files()->isLocal() && file->vfile_isDir() && !multipleSelections) {
        const QString selectedDirectoryPath = file->vfile_getUrl().path();
        if (krMtMan.getStatus(selectedDirectoryPath) == KMountMan::MOUNTED)
            addAction(i18n("Unmount"))->setData(QVariant(UNMOUNT_ID));
        else if (krMtMan.getStatus(selectedDirectoryPath) == KMountMan::NOT_MOUNTED)
            addAction(i18n("Mount"))->setData(QVariant(MOUNT_ID));
        if (krMtMan.ejectable(selectedDirectoryPath))
            addAction(i18n("Eject"))->setData(QVariant(EJECT_ID));
    }

    // --------- send by mail
    if (KrServices::supportedTools().contains("MAIL") && !file->vfile_isDir()) {
        addAction(i18n("Send by Email"))->setData(QVariant(SEND_BY_EMAIL_ID));
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
    addAction(i18n("Copy to Clipboard"))->setData(QVariant(COPY_CLIP_ID));
    addAction(i18n("Cut to Clipboard"))->setData(QVariant(MOVE_CLIP_ID));
    addAction(i18n("Paste from Clipboard"))->setData(QVariant(PASTE_CLIP_ID));
    addSeparator();

    // --------- properties
    addAction(panel->gui->actions()->actProperties);
}

void KrPopupMenu::addEmptyMenuEntries()
{
    addAction(i18n("Paste from Clipboard"))->setData(QVariant(PASTE_CLIP_ID));
}

void KrPopupMenu::addCreateNewMenu()
{
    createNewPopup.addAction(krLoader->loadIcon("folder", KIconLoader::Small), i18n("Folder..."))->setData(QVariant(MKDIR_ID));
    createNewPopup.addAction(krLoader->loadIcon("text-plain", KIconLoader::Small), i18n("Text File..."))->setData(QVariant(NEW_TEXT_FILE_ID));

    QAction *newAct = addMenu(&createNewPopup);
    newAct->setData(QVariant(CREATE_NEW_ID));
    newAct->setText(i18n("Create New"));

}

void KrPopupMenu::performAction(int id)
{
    if (_items.isEmpty())
        return; // sanity check, empty file list

    KFileItem *item = &_items.first();

    switch (id) {
    case - 1 : // the user clicked outside of the menu
        return ;
    case OPEN_TAB_ID :
        // assuming only 1 file is selected (otherwise we won't get here)
        panel->manager()->newTab(item->url(), panel);
        break;
    case OPEN_ID :
        foreach(KFileItem fi, _items)
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
    case RENAME_ID :
        panel->func->rename();
        break;
    case TRASH_ID :
        panel->func->deleteFiles(false);
        break;
    case DELETE_ID :
        panel->func->deleteFiles(true);
        break;
    case EJECT_ID :
        krMtMan.eject(item->url().adjusted(QUrl::StripTrailingSlash).path());
        break;
        /*         case SHRED_ID :
                    if ( KMessageBox::warningContinueCancel( krApp,
                         i18n("<qt>Do you really want to shred <b>%1</b>? Once shred, the file is gone forever.</qt>", item->name()),
                         QString(), KStandardGuiItem::cont(), KStandardGuiItem::cancel(), "Shred" ) == KMessageBox::Continue )
                       KShred::shred( panel->func->files() ->vfs_getFile( item->name() ).adjusted(QUrl::RemoveTrailingSlash).path() );
                  break;*/
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
    case COPY_CLIP_ID :
        panel->func->copyToClipboard();
        break;
    case MOVE_CLIP_ID :
        panel->func->copyToClipboard(true);
        break;
    case PASTE_CLIP_ID :
        panel->func->pasteFromClipboard();
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
        foreach(KFileItem item, _items)
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

