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

#include <QPixmap>

#include <klocale.h>
#include <kprocess.h>
#include <krun.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kmimetypetrader.h>
#include <ktoolinvocation.h>
#include <kactioncollection.h>

#include "listpanel.h"
#include "krview.h"
#include "krviewitem.h"
#include "panelfunc.h"
#include "../krservices.h"
#include "../defaults.h"
#include "../MountMan/kmountman.h"
#include "../krusader.h"
#include "../kractions.h"
#include "../krslots.h"
#include "../krusaderview.h"
#include "../panelmanager.h"
#include "../krtrashhandler.h"

#ifdef __LIBKONQ__
#include <konq_popupmenu.h>
#include <konq_menuactions.h>
#include <konq_popupmenuinformation.h>
#endif

void KrPopupMenu::run(const QPoint &pos, KrPanel *panel)
{
    KrPopupMenu menu(panel);
    QAction * res = menu.exec(pos);
    int result = -1;
    if (res && res->data().canConvert<int>())
        result = res->data().toInt();
    menu.performAction(result);
}

KrPopupMenu::KrPopupMenu(KrPanel *thePanel, QWidget *parent) : KMenu(parent), panel(thePanel), empty(false),
        multipleSelections(false), actions(0)
{
#ifdef __LIBKONQ__
    konqMenu = 0;
    konqMenuActions = 0;
#endif

    panel->view->getSelectedKrViewItems(&items);
    if (items.empty()) {
        addCreateNewMenu();
        addSeparator();
        addEmptyMenuEntries();
        return;
    } else if (items.size() > 1) multipleSelections = true;

    QList<QString> protocols;
    for (int i = 0; i < items.size(); ++i) {
        protocols.append(panel->func->getVFile(items[ i ]) ->vfile_getUrl().protocol());
    }
    bool inTrash = protocols.contains("trash");
    bool trashOnly = (protocols.count() == 1) && (protocols[ 0 ] == "trash");

    item = items.first();
    vfile *vf = panel->func->getVFile(item);

    // ------------ the OPEN option - open preferred service
    QAction * openAct = addAction(i18n("Open/Run"));
    openAct->setData(QVariant(OPEN_ID));
    if (!multipleSelections) {   // meaningful only if one file is selected
        openAct->setIcon(item->icon());
        openAct->setText(vf->vfile_isExecutable() && !vf->vfile_isDir() ? i18n("Run") : i18n("Open"));
        // open in a new tab (if folder)
        if (vf->vfile_isDir()) {
            QAction * openTab = addAction(i18n("Open in New Tab"));
            openTab->setData(QVariant(OPEN_TAB_ID));
            openTab->setIcon(krLoader->loadIcon("tab-new", KIconLoader::Panel));
            openTab->setText(i18n("Open in New Tab"));
        }
        addSeparator();
    }

    // ------------- Preview - normal vfs only ?
    if (panel->func->files()->vfs_getType() == vfs::VFS_NORMAL) {
        // create the preview popup
        QStringList names;
        panel->gui->getSelectedNames(&names);
        preview.setUrls(panel->func->files() ->vfs_getFiles(&names));
        QAction *pAct = addMenu(&preview);
        pAct->setData(QVariant(PREVIEW_ID));
        pAct->setText(i18n("Preview"));
    }

    // -------------- Open with: try to find-out which apps can open the file
    // this too, is meaningful only if one file is selected or if all the files
    // have the same mimetype !
    QString mime = panel->func->getVFile(item)->vfile_getMime();
    // check if all the list have the same mimetype
    for (int i = 1; i < items.size(); ++i) {
        if (panel->func->getVFile(items[ i ]) ->vfile_getMime() != mime) {
            mime.clear();
            break;
        }
    }
    if (!mime.isEmpty()) {
        offers = KMimeTypeTrader::self()->query(mime);
        for (int i = 0; i < offers.count(); ++i) {
            KSharedPtr<KService> service = offers[ i ];
            if (service->isValid() && service->isApplication()) {
                openWith.addAction(krLoader->loadIcon(service->icon(), KIconLoader::Small), service->name())->setData(QVariant(SERVICE_LIST_ID + i));
            }
        }
        openWith.addSeparator();
        if (vf->vfile_isDir())
            openWith.addAction(krLoader->loadIcon("konsole", KIconLoader::Small), i18n("Terminal"))->setData(QVariant(OPEN_TERM_ID));
        openWith.addAction(i18n("Other..."))->setData(QVariant(CHOOSE_ID));
        QAction *owAct = addMenu(&openWith);
        owAct->setData(QVariant(OPEN_WITH_ID));
        owAct->setText(i18n("Open With"));
        addSeparator();
    }

    // --------------- user actions
    QAction *uAct = new UserActionPopupMenu(panel->func->files()->vfs_getFile(item->name()).url());
    addAction(uAct);
    uAct->setText(i18n("User Actions"));
    for (KrViewItemList::Iterator it = items.begin(); it != items.end(); ++it) {
        vfile *file = panel->func->files()->vfs_search(((*it)->name()));
        KUrl url = file->vfile_getUrl();
        _items.append(KFileItem(url,  file->vfile_getMime(), file->vfile_getMode()));
    }

#ifdef __LIBKONQ__
    // -------------- konqueror menu
    // This section adds all Konqueror/Dolphin menu items.
    // It's now updated to KDE4 and working, I've just commented it out.
    // Section below this one adds only servicemenus.

    // Append only servicemenus

    //TODO: deprecated since KDE4.3: remove these three lines
    KonqPopupMenuInformation info;
    info.setItems(_items);
    info.setParentWidget(this);

    konqMenuActions = new KonqMenuActions();
    //TODO: deprecated since KDE4.3: remove this line, use two commented lines
    konqMenuActions->setPopupMenuInfo(info);
    //konqMenuActions->setParentWidget( this );
    //konqMenuActions->setItemListProperties( _items );
    konqMenuActions->addActionsTo(this);

    addSeparator();
#endif

    // ------------- 'create new' submenu
    addCreateNewMenu();
    addSeparator();

    // ---------- COPY
    addAction(i18n("Copy..."))->setData(QVariant(COPY_ID));
    if (panel->func->files() ->vfs_isWritable()) {
        // ------- MOVE
        addAction(i18n("Move..."))->setData(QVariant(MOVE_ID));
        // ------- RENAME - only one file
        if (!multipleSelections && !inTrash)
            addAction(i18n("Rename"))->setData(QVariant(RENAME_ID));

        // -------- MOVE TO TRASH
        KConfigGroup saver(krConfig, "General");
        bool trash = saver.readEntry("Move To Trash", _MoveToTrash);
        if (trash && !inTrash)
            addAction(i18n("Move to Trash"))->setData(QVariant(TRASH_ID));
        // -------- DELETE
        addAction(i18n("Delete"))->setData(QVariant(DELETE_ID));
        // -------- SHRED - only one file
        /*      if ( panel->func->files() ->vfs_getType() == vfs::VFS_NORMAL &&
                    !vf->vfile_isDir() && !multipleSelections )
                 addAction( i18n( "Shred" ) )->setData( QVariant( SHRED_ID ) );*/
    }

    // ---------- link handling
    // create new shortcut or redirect links - only on local directories:
    if (panel->func->files() ->vfs_getType() == vfs::VFS_NORMAL) {
        addSeparator();
        linkPopup.addAction(i18n("New Symlink..."))->setData(QVariant(NEW_SYMLINK_ID));
        linkPopup.addAction(i18n("New Hardlink..."))->setData(QVariant(NEW_LINK_ID));
        if (panel->func->getVFile(item)->vfile_isSymLink())
            linkPopup.addAction(i18n("Redirect Link..."))->setData(QVariant(REDIRECT_LINK_ID));
        QAction *linkAct = addMenu(&linkPopup);
        linkAct->setData(QVariant(LINK_HANDLING_ID));
        linkAct->setText(i18n("Link Handling"));
    }
    addSeparator();

    // ---------- calculate space
    if (panel->func->files() ->vfs_getType() == vfs::VFS_NORMAL && (vf->vfile_isDir() || multipleSelections))
        addAction(krCalculate);

    // ---------- mount/umount/eject
    if (panel->func->files() ->vfs_getType() == vfs::VFS_NORMAL && vf->vfile_isDir() && !multipleSelections) {
        if (krMtMan.getStatus(panel->func->files() ->vfs_getFile(item->name()).path(KUrl::RemoveTrailingSlash)) == KMountMan::MOUNTED)
            addAction(i18n("Unmount"))->setData(QVariant(UNMOUNT_ID));
        else if (krMtMan.getStatus(panel->func->files() ->vfs_getFile(item->name()).path(KUrl::RemoveTrailingSlash)) == KMountMan::NOT_MOUNTED)
            addAction(i18n("Mount"))->setData(QVariant(MOUNT_ID));
        if (krMtMan.ejectable(panel->func->files() ->vfs_getFile(item->name()).path(KUrl::RemoveTrailingSlash)))
            addAction(i18n("Eject"))->setData(QVariant(EJECT_ID));
    }

    // --------- send by mail
    if (Krusader::supportedTools().contains("MAIL") && !vf->vfile_isDir()) {
        addAction(i18n("Send by Email"))->setData(QVariant(SEND_BY_EMAIL_ID));
    }

    // --------- empty trash
    if (trashOnly) {
        addAction(i18n("Restore"))->setData(QVariant(RESTORE_TRASHED_FILE_ID));
        addAction(i18n("Empty trash"))->setData(QVariant(EMPTY_TRASH_ID));
    }

    // --------- synchronize
    if (panel->view->numSelected()) {
        addAction(i18n("Synchronize Selected Files..."))->setData(QVariant(SYNC_SELECTED_ID));
    }

    // --------- copy/paste
    addSeparator();
    addAction(i18n("Copy to Clipboard"))->setData(QVariant(COPY_CLIP_ID));
    if (panel->func->files() ->vfs_isWritable()) {
        addAction(i18n("Cut to Clipboard"))->setData(QVariant(MOVE_CLIP_ID));
        addAction(i18n("Paste from Clipboard"))->setData(QVariant(PASTE_CLIP_ID));
    }
    addSeparator();

    // --------- properties
    addAction(krProperties);
}

KrPopupMenu::~KrPopupMenu()
{
    _items.clear();
    if (actions) delete actions;
#ifdef __LIBKONQ__
    if (konqMenu) delete konqMenu;
    if (konqMenuActions) delete konqMenuActions;
#endif
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
    KUrl u;
    KUrl::List lst;

    switch (id) {
    case - 1 : // the user clicked outside of the menu
        return ;
    case OPEN_TAB_ID :
        // assuming only 1 file is selected (otherwise we won't get here)
        (ACTIVE_PANEL == LEFT_PANEL ? LEFT_MNG : RIGHT_MNG)->
        slotNewTab(panel->func->files()->vfs_getFile(item->name()).url());
        break;
    case OPEN_ID :
        for (KrViewItemList::Iterator it = items.begin(); it != items.end(); ++it) {
            u = panel->func->files()->vfs_getFile((*it) ->name());
            KRun::runUrl(u, panel->func->getVFile(item)->vfile_getMime(), parentWidget());
        }
        break;
    case COPY_ID :
        panel->func->copyFiles();
        break;
    case MOVE_ID :
        panel->func->moveFiles();
        break;
    case RENAME_ID :
        SLOTS->rename();
        break;
    case TRASH_ID :
        panel->func->deleteFiles(false);
        break;
    case DELETE_ID :
        panel->func->deleteFiles(true);
        break;
    case EJECT_ID :
        krMtMan.eject(panel->func->files() ->vfs_getFile(item->name()).path(KUrl::RemoveTrailingSlash));
        break;
        /*         case SHRED_ID :
                    if ( KMessageBox::warningContinueCancel( krApp,
                         i18n("<qt>Do you really want to shred <b>%1</b>? Once shred, the file is gone forever!</qt>", item->name()),
                         QString(), KStandardGuiItem::cont(), KStandardGuiItem::cancel(), "Shred" ) == KMessageBox::Continue )
                       KShred::shred( panel->func->files() ->vfs_getFile( item->name() ).path( KUrl::RemoveTrailingSlash ) );
                  break;*/
    case OPEN_KONQ_ID :
        KToolInvocation::startServiceByDesktopName("konqueror", panel->func->files() ->vfs_getFile(item->name()).url());
        break;
    case CHOOSE_ID : // open-with dialog
        u = panel->func->files() ->vfs_getFile(item->name());
        lst.append(u);
        KRun::displayOpenWithDialog(lst, parentWidget());
        break;
    case MOUNT_ID :
        krMtMan.mount(panel->func->files() ->vfs_getFile(item->name()).path(KUrl::RemoveTrailingSlash));
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
    case RESTORE_TRASHED_FILE_ID : {
        QStringList fileNames;
        panel->gui->getSelectedNames(&fileNames);
        KrTrashHandler::restoreTrashedFiles(*panel->func->files() ->vfs_getFiles(&fileNames));
    }
    break;
    case UNMOUNT_ID :
        krMtMan.unmount(panel->func->files() ->vfs_getFile(item->name()).path(KUrl::RemoveTrailingSlash));
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
        QStringList fileNames;
        panel->gui->getSelectedNames(&fileNames);
        SLOTS->sendFileByEmail(*panel->func->files() ->vfs_getFiles(&fileNames));
        break;
    }
    case MKDIR_ID :
        SLOTS->mkdir();
        break;
    case NEW_TEXT_FILE_ID:
        SLOTS->editDlg();
        break;
    case SYNC_SELECTED_ID : {
        QStringList selectedNames;
        for (KrViewItemList::Iterator it = items.begin(); it != items.end(); ++it)
            selectedNames.append((*it) ->name());
        if (panel->otherPanel->view->numSelected()) {
            KrViewItemList otherItems;
            panel->otherPanel->view->getSelectedKrViewItems(&otherItems);

            for (KrViewItemList::Iterator it2 = otherItems.begin(); it2 != otherItems.end(); ++it2) {
                QString name = (*it2) ->name();
                if (!selectedNames.contains(name))
                    selectedNames.append(name);
            }
        }
        SLOTS->slotSynchronizeDirs(selectedNames);
    }
    break;
    case OPEN_TERM_ID : {
        QStringList args;
        if (!panel->func->getVFile(item)->vfile_isDir())
            args << item->name();
        SLOTS->runTerminal(panel->func->files() ->vfs_getFile(item->name()).path(),
                           args);
    }
    break;
    }

    // check if something from the open-with-offered-services was selected
    if (id >= SERVICE_LIST_ID) {
        QStringList names;
        panel->gui->getSelectedNames(&names);
        KRun::run(*(offers[ id - SERVICE_LIST_ID ]),
                  *(panel->func->files() ->vfs_getFiles(&names)), parentWidget());
    }
}

#include "krpopupmenu.moc"
