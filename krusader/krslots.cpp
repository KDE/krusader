/***************************************************************************
                                 krslots.cpp
                             -------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
    web site   : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "krslots.h"

#include <QtCore/QDir>
#include <QtCore/QPoint>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QEvent>
#include <QtGui/QPixmapCache>
#include <QtGui/QKeyEvent>

#include <KToggleAction>
#include <KToolBar>
#include <KLocale>
#include <KMessageBox>
#include <KEditToolBar>
#include <KCmdLineArgs>

#ifdef __KJSEMBED__
#include <kjsembed/jsconsolewidget.h>
#include "KrJS/krjs.h"
#endif

#include "KViewer/krviewer.h"
#include "Panel/krviewfactory.h"
#include "krusader.h"
#include "krusaderview.h"
#include "Panel/listpanel.h"
#include "Panel/krselectionmode.h"
#include "Dialogs/krdialogs.h"
#include "Dialogs/krspwidgets.h"
#include "Dialogs/krkeydialog.h"
#include "GUI/krusaderstatus.h"
#include "Panel/panelfunc.h"
#include "Konfigurator/konfigurator.h"
#include "MountMan/kmountman.h"
#include "defaults.h"
#include "resources.h"
#include "GUI/kfnkeys.h"
#include "GUI/kcmdline.h"
#include "GUI/terminaldock.h"
#include "GUI/syncbrowsebutton.h"
#include "VFS/krquery.h"
#include "Search/krsearchmod.h"
#include "Search/krsearchdialog.h"
#include "Locate/locate.h"
#include "VFS/vfs.h"
#include "VFS/vfile.h"
#include "panelmanager.h"
#include "Splitter/splittergui.h"
#include "Splitter/splitter.h"
#include "Splitter/combiner.h"
#include "ActionMan/actionman.h"
#include "UserMenu/usermenu.h"
#include "Panel/panelpopup.h"
#include "Synchronizer/synchronizergui.h"
#include "DiskUsage/diskusagegui.h"
#include "krservices.h"
#include "Panel/krviewitem.h"
#include "Queue/queuedialog.h"
#include "krtrashhandler.h"

#define REFRESH_BOTH_PANELS { ListPanel *p=ACTIVE_PANEL;        \
        MAIN_VIEW->left->func->refresh(); \
        MAIN_VIEW->right->func->refresh();\
        p->slotFocusOnMe(); }
#define ACTIVE_PANEL_MANAGER  (ACTIVE_PANEL == krApp->mainView->left ? krApp->mainView->leftMng : \
                               krApp->mainView->rightMng)

void KRslots::sendFileByEmail(const KUrl::List &urls)
{
    if (urls.count() == 0) {
        KMessageBox::error(0, i18n("No selected files to send!"));
        return;
    }

    QString mailProg;
    QStringList lst = Krusader::supportedTools();
    if (lst.contains("MAIL")) mailProg = lst[lst.indexOf("MAIL") + 1];
    else {
        KMessageBox::error(0, i18n("Krusader can't find a supported mail client. Please install one to your path. Hint: Krusader supports Kmail."));
        return;
    }

    QString subject, separator;
    foreach(const KUrl &url, urls) {
        subject += separator + url.fileName();
        separator = ',';
    }
    subject = i18np("Sending file: %2", "Sending files: %2", urls.count(), subject);

    KProcess proc;

    if (KUrl(mailProg).fileName() == "kmail") {
        proc << mailProg << "--subject"
        << subject;
        foreach(const KUrl &url2, urls)
        proc << "--attach" << url2.prettyUrl();
    } else   if (KUrl(mailProg).fileName() == "thunderbird") {
        QString param = "attachment=\'";
        separator = "";
        foreach(const KUrl &url2, urls) {
            param += separator + url2.prettyUrl();
            separator = ',';
        }
        param += "\',subject=\'" + subject + "\'";
        proc << mailProg << "--compose" << param;
    } else if (KUrl(mailProg).fileName() == "evolution") {
        QString param = "mailto:?cc=&subject=" + subject + "&attach=";
        separator = "";
        foreach(const KUrl &url2, urls) {
            param += separator + url2.prettyUrl();
            separator = "&attach=";
        }
        proc << mailProg << param + "";
    }

    if (!proc.startDetached())
        KMessageBox::error(0, i18n("Error executing %1!", mailProg));
}

void KRslots::compareContent()
{
    QStringList lstLeft, lstRight;
    QStringList* lstActive;
    KUrl name1, name2;

    MAIN_VIEW->left->getSelectedNames(&lstLeft);
    MAIN_VIEW->right->getSelectedNames(&lstRight);
    lstActive = (ACTIVE_PANEL->isLeft() ? &lstLeft : &lstRight);

    if (lstLeft.count() == 1 && lstRight.count() == 1) {
        // first, see if we've got exactly 1 selected file in each panel:
        name1 = MAIN_VIEW->left->func->files()->vfs_getFile(lstLeft[0]);
        name2 = MAIN_VIEW->right->func->files()->vfs_getFile(lstRight[0]);
    } else if (lstActive->count() == 2) {
        // next try: are in the current panel exacty 2 files selected?
        name1 = ACTIVE_PANEL->func->files()->vfs_getFile((*lstActive)[0]);
        name2 = ACTIVE_PANEL->func->files()->vfs_getFile((*lstActive)[1]);
    } else if (ACTIVE_PANEL->otherPanel->func->files()->vfs_search(ACTIVE_PANEL->view->getCurrentItem())) {
        // next try: is in the other panel a file with the same name?
        name1 = ACTIVE_PANEL->func->files()->vfs_getFile(ACTIVE_PANEL->view->getCurrentItem());
        name2 = ACTIVE_PANEL->otherPanel->func->files()->vfs_getFile(ACTIVE_PANEL->view->getCurrentItem());
    } else  {
        // if we got here, then we can't be sure what file to diff
        KMessageBox::detailedError(0, i18n("Don't know which files to compare."), "<qt>" + i18n("To compare two files by content, you can either:<ul><li>Select one file in the left panel, and one in the right panel.</li><li>Select exactly two files in the active panel.</li><li>Make sure there is a file in the other panel, with the same name as the current file in the active panel.</li></ul>") + "</qt>");

        return;
    }

    // else implied: all ok, let's call kdiff
    // but if one of the files isn't local, download them first
    compareContent(name1, name2);
}

void KRslots::compareContent(KUrl url1, KUrl url2)
{
    QString diffProg;
    QStringList lst = Krusader::supportedTools();
    if (lst.contains("DIFF")) diffProg = lst[lst.indexOf("DIFF") + 1];
    else {
        KMessageBox::error(0, i18n("Krusader can't find any of the supported diff-frontends. Please install one to your path. Hint: Krusader supports Kompare, Kdiff3 and Xxdiff."));
        return;
    }

    QString tmp1;
    QString tmp2;

    // kdiff3 sucks with spaces
    if (KUrl(diffProg).fileName() == "kdiff3" && !url1.prettyUrl().contains(" ") && !url2.prettyUrl().contains(" ")) {
        tmp1 = url1.prettyUrl();
        tmp2 = url2.prettyUrl();
    } else {
        if (!url1.isLocalFile()) {
            if (!KIO::NetAccess::download(url1, tmp1, 0)) {
                KMessageBox::sorry(krApp, i18n("Krusader is unable to download %1", url1.fileName()));
                return;
            }
        } else tmp1 = url1.path();
        if (!url2.isLocalFile()) {
            if (!KIO::NetAccess::download(url2, tmp2, 0)) {
                KMessageBox::sorry(krApp, i18n("Krusader is unable to download %1", url2.fileName()));
                if (tmp1 != url1.path())
                    KIO::NetAccess::removeTempFile(tmp1);
                return;
            }
        } else tmp2 = url2.path();
    }

    KrProcess *p = new KrProcess(tmp1 != url1.path() ? tmp1 : QString(),
                                 tmp2 != url2.path() ? tmp2 : QString());
    *p << diffProg << tmp1 << tmp2;
    p->start();
    if (!p->waitForStarted())
        KMessageBox::error(0, i18n("Error executing %1!", diffProg));
}

void KRslots::rightclickMenu()
{
    if (ACTIVE_PANEL->view->getCurrentKrViewItem()) {
        ACTIVE_PANEL->popRightClickMenu(
            ACTIVE_PANEL->mapToGlobal(
                ACTIVE_PANEL->view->getCurrentKrViewItem()->itemRect().topLeft()
            )
        );
    }
}

void KRslots::addBookmark()
{
    // TODO: this no longer works!
}

// GUI toggle slots
void KRslots::toggleFnkeys()
{
    if (MAIN_VIEW->fnKeys->isVisible()) MAIN_VIEW->fnKeys->hide();
    else MAIN_VIEW->fnKeys->show();
}

void KRslots::toggleCmdline()
{
    if (MAIN_VIEW->cmdLine->isVisible()) MAIN_VIEW->cmdLine->hide();
    else MAIN_VIEW->cmdLine->show();
}

void KRslots::toggleToolbar()
{
    if (krApp->toolBar()->isVisible())
        krApp->toolBar()->hide();
    else krApp->toolBar()->show();
}

void KRslots::toggleActionsToolbar()
{
    if (krApp->toolBar("actionsToolBar")->isVisible())
        krApp->toolBar("actionsToolBar")->hide();
    else krApp->toolBar("actionsToolBar")->show();
}


void KRslots::toggleStatusbar()
{
    if (krApp->statusBar()->isVisible())
        krApp->statusBar()->hide();
    else krApp->statusBar()->show();
}

void KRslots::toggleTerminal()
{
    if (MAIN_VIEW->terminal_dock->isVisible()) MAIN_VIEW->slotTerminalEmulator(false);
    else MAIN_VIEW->slotTerminalEmulator(true);
}

void KRslots::insertFileName(bool full_path)
{
    QString filename = ACTIVE_PANEL->view->getCurrentItem();
    if (filename.isEmpty()) {
        return;
    }

    if (full_path) {
        QString path = vfs::pathOrUrl(ACTIVE_FUNC->files()->vfs_getOrigin(), KUrl::AddTrailingSlash);
        filename = path + filename;
    }

    filename = KrServices::quote(filename);

    if (MAIN_VIEW->cmdLine->isVisible() || !MAIN_VIEW->terminal_dock->isTerminalVisible()) {
        QString current = MAIN_VIEW->cmdLine->text();
        if (current.length() != 0 && !current.endsWith(' '))
            current += ' ';
        MAIN_VIEW->cmdLine->setText(current + filename);
        MAIN_VIEW->cmdLine->setFocus();
    } else if (MAIN_VIEW->terminal_dock->isTerminalVisible()) {
        filename = QChar(' ') + filename + QChar(' ');
        MAIN_VIEW->terminal_dock->sendInput(filename);
        MAIN_VIEW->terminal_dock->setFocus();
    }
}

// directory list functions
void KRslots::allFilter()
{
    ACTIVE_PANEL->setFilter(KrViewProperties::All);
}
void KRslots::execFilter()
{
    ACTIVE_PANEL->setFilter(KrViewProperties::All);
}
void KRslots::customFilter()
{
    ACTIVE_PANEL->setFilter(KrViewProperties::Custom);
}
void KRslots::markAll()
{
    ACTIVE_PANEL->select(true, true);
}
void KRslots::unmarkAll()
{
    ACTIVE_PANEL->select(false, true);
}
void KRslots::markGroup()
{
    ACTIVE_PANEL->select(true, false);
}
void KRslots::markGroup(const QString& mask, bool select)
{
    ACTIVE_PANEL->select(KRQuery(mask), select);
}
void KRslots::unmarkGroup()
{
    ACTIVE_PANEL->select(false, false);
}
void KRslots::invert()
{
    ACTIVE_PANEL->invertSelection();
}

void KRslots::root()
{
    ACTIVE_FUNC->openUrl(KUrl("/"));
}
void KRslots::refresh(const KUrl& u)
{
    ACTIVE_FUNC->openUrl(u);
}
void KRslots::home()
{
    ACTIVE_FUNC->openUrl(QDir::homePath());
}
void KRslots::refresh()
{
    ACTIVE_FUNC->refresh();
}
void KRslots::properties()
{
    ACTIVE_FUNC->properties();
}
void KRslots::dirUp()
{
    ACTIVE_FUNC->dirUp();
}
void KRslots::back()
{
    ACTIVE_FUNC->goBack();
}
void KRslots::slotPack()
{
    ACTIVE_FUNC->pack();
}
void KRslots::slotUnpack()
{
    ACTIVE_FUNC->unpack();
}
void KRslots::testArchive()
{
    ACTIVE_FUNC->testArchive();
}
void KRslots::calcSpace()
{
    ACTIVE_FUNC->calcSpace();
}
void KRslots::FTPDisconnect()
{
    ACTIVE_FUNC->FTPDisconnect();
}
void KRslots::newFTPconnection()
{
    ACTIVE_FUNC->newFTPconnection();
}
void KRslots::cut()
{
    ACTIVE_FUNC->copyToClipboard(true);
}
void KRslots::copy()
{
    ACTIVE_FUNC->copyToClipboard(false);
}
void KRslots::paste()
{
    ACTIVE_FUNC->pasteFromClipboard();
}
void KRslots::createChecksum()
{
    ACTIVE_FUNC->createChecksum();
}
void KRslots::matchChecksum()
{
    ACTIVE_FUNC->matchChecksum();
}

// run external modules / programs
void KRslots::runKonfigurator(bool firstTime)
{

    KConfigGroup group(krConfig, "Look&Feel");
    KConfigGroup groupgen(krConfig, "General");
    int size = (group.readEntry("Filelist Icon Size", _FilelistIconSize)).toInt();

    Konfigurator *konfigurator = new Konfigurator(firstTime);

    vfile::vfile_enableMimeTypeMagic(groupgen.readEntry("Mimetype Magic", _MimetypeMagic));
    if (konfigurator->isGUIRestartNeeded()) {
        vfile::vfile_loadUserDefinedFolderIcons(group.readEntry("Load User Defined Folder Icons", _UserDefinedFolderIcons));

        if ((group.readEntry("Filelist Icon Size", _FilelistIconSize)).toInt() != size)
            QPixmapCache::clear();

        QList<KrViewInstance *> views = KrViewFactory::registeredViews();
        foreach(KrViewInstance * inst, views)
        KrViewFactory::itemHeightChanged(inst->id());

        MAIN_VIEW->leftMng->slotRecreatePanels();
        MAIN_VIEW->rightMng->slotRecreatePanels();
        MAIN_VIEW->fnKeys->updateShortcuts();
        KrSelectionMode::resetSelectionHandler();
    }

    krApp->configChanged();
    delete konfigurator;
}

void KRslots::setView0()
{
    if (ACTIVE_PANEL && ACTIVE_PANEL->getType() != 0)
        ACTIVE_PANEL->changeType(0);
}

void KRslots::setView1()
{
    if (ACTIVE_PANEL && ACTIVE_PANEL->getType() != 1)
        ACTIVE_PANEL->changeType(1);
}

void KRslots::setView2()
{
    if (ACTIVE_PANEL && ACTIVE_PANEL->getType() != 2)
        ACTIVE_PANEL->changeType(2);
}

void KRslots::setView3()
{
    if (ACTIVE_PANEL && ACTIVE_PANEL->getType() != 3)
        ACTIVE_PANEL->changeType(3);
}

void KRslots::setView4()
{
    if (ACTIVE_PANEL && ACTIVE_PANEL->getType() != 4)
        ACTIVE_PANEL->changeType(4);
}

void KRslots::setView5()
{
    if (ACTIVE_PANEL && ACTIVE_PANEL->getType() != 5)
        ACTIVE_PANEL->changeType(5);
}

void KRslots::toggleHidden()
{
    KConfigGroup group(krConfig, "Look&Feel");
    bool show = !group.readEntry("Show Hidden", _ShowHidden);
    krApp->actToggleHidden->setChecked(show);
    group.writeEntry("Show Hidden", show);

    MAIN_VIEW->leftMng->refreshAllTabs(true);
    MAIN_VIEW->rightMng->refreshAllTabs(true);
}

void KRslots::swapPanels()
{
    KUrl leftURL = MAIN_VIEW->left->func->files()->vfs_getOrigin();
    KUrl rightURL = MAIN_VIEW->right->func->files()->vfs_getOrigin();

    MAIN_VIEW->left->func->openUrl(rightURL);
    MAIN_VIEW->right->func->openUrl(leftURL);
}

void KRslots::toggleSwapSides()
{
    QList<int> lst = MAIN_VIEW->horiz_splitter->sizes();

    MAIN_VIEW->horiz_splitter->addWidget(MAIN_VIEW->leftMng);

    int old = lst[ 0 ];
    lst[ 0 ] = lst [ 1 ];
    lst[ 1 ] = old;

    MAIN_VIEW->horiz_splitter->setSizes(lst);

    ListPanel     *tmpPanel;     // temporary variables for swapping
    PanelManager  *tmpMng;

    tmpMng = MAIN_VIEW->leftMng;
    MAIN_VIEW->leftMng = MAIN_VIEW->rightMng;
    MAIN_VIEW->rightMng = tmpMng;

    tmpPanel = MAIN_VIEW->left;
    MAIN_VIEW->left = MAIN_VIEW->right;
    MAIN_VIEW->right = tmpPanel;

    MAIN_VIEW->leftMng->swapPanels();
    MAIN_VIEW->rightMng->swapPanels();

    MAIN_VIEW->left->updateGeometry();
    MAIN_VIEW->right->updateGeometry();
}

void KRslots::search()
{
    if (KrSearchDialog::SearchDialog != 0) {
        KConfigGroup group(krConfig, "Search");
        if (group.readEntry("Window Maximized",  false))
            KrSearchDialog::SearchDialog->showMaximized();
        else
            KrSearchDialog::SearchDialog->showNormal();

        KrSearchDialog::SearchDialog->raise();
        KrSearchDialog::SearchDialog->activateWindow();
    } else
        KrSearchDialog::SearchDialog = new KrSearchDialog();
}

void KRslots::locate()
{
    if (!KrServices::cmdExist("locate")) {
        KMessageBox::error(krApp, i18n("Can't find the 'locate' command. Please install the "
                                       "findutils-locate package of GNU, or set its dependencies in "
                                       "Konfigurator"));
        return;
    }

    if (LocateDlg::LocateDialog != 0) {
        LocateDlg::LocateDialog->showNormal();
        LocateDlg::LocateDialog->raise();
        LocateDlg::LocateDialog->activateWindow();
        LocateDlg::LocateDialog->reset();
    } else
        LocateDlg::LocateDialog = new LocateDlg();
}

void KRslots::runMountMan()
{
    // left as a precaution, although we use kde's services now
    if (!KrServices::cmdExist("mount")) {
        KMessageBox::error(0,
                           i18n("Can't start 'mount'! Check the 'Dependencies' page in konfigurator."));
        return;
    }

    krApp->mountMan->mainWindow();
}

void KRslots::runTerminal(const QString & dir, const QStringList & args)
{
    KProcess proc;
    proc.setWorkingDirectory(dir);
    KConfigGroup group(krConfig, "General");
    QString term = group.readEntry("Terminal", _Terminal);
    QStringList sepdArgs = KrServices::separateArgs(term);
    for (int i = 0; i != sepdArgs.size(); i++)
        if (sepdArgs[ i ] == "%d")
            sepdArgs[ i ] = dir;
    proc << sepdArgs;
    if (!args.isEmpty()) {
        proc << "-e" << args; // FIXME this depends on term!! But works in konsole, xterm and gnome-terminal
    }
#if 0 // I hope this is no longer needed...
    if (term.contains("konsole"))      /* KDE 3.2 bug (konsole is killed by pressing Ctrl+C) */
    {                                  /* Please remove the patch if the bug is corrected */
        proc << "&";
        proc.setUseShell(true);
    }
#endif
    if (!proc.startDetached())
        KMessageBox::sorry(krApp, i18n("Error executing %1!", term));
}

void KRslots::homeTerminal()
{
    runTerminal(QDir::homePath(), QStringList());
}

void KRslots::sysInfo()
{
    KProcess proc;
    proc << KrServices::fullPathName("kcmshell") << "System/ksysctrl";
    if (!proc.startDetached()) {
        KMessageBox::sorry(krApp, i18n("Can't find \"KsysCtrl\". Please install KDE admin package"));
    }
}

void KRslots::multiRename()
{
    QStringList lst = Krusader::supportedTools();
    int i = lst.indexOf("RENAME");
    if (i == -1) {
        KMessageBox::sorry(krApp, i18n("Can't find a batch rename tool.\nYou can get Krename at http://www.krename.net"));
        return;
    }
    QString pathToRename = lst[i+1];

    QStringList names;
    ((ListPanel*)ACTIVE_PANEL)->getSelectedNames(&names);
    KUrl::List* urls = ACTIVE_FUNC->files()->vfs_getFiles(&names);

    if (urls->isEmpty()) {
        delete urls;
        return;
    }

    KProcess proc;
    proc << pathToRename;

    for (KUrl::List::iterator u = urls->begin(); u != urls->end(); ++u) {
        if (QFileInfo((*u).path()).isDir()) proc << "-r";
        proc << (*u).path();
    }

    if (!proc.startDetached())
        KMessageBox::error(0, i18n("Error executing %1!", pathToRename));
    delete urls;
}

void KRslots::rootKrusader()
{
    if (!KrServices::cmdExist("krusader") || !KrServices::cmdExist("kdesu")) {
        KMessageBox::sorry(krApp, i18n("Can't start root mode krusader, because krusader or kdesu is missing from the path. Please configure the dependencies in Konfigurator!"));
        return;
    }

    KProcess proc;
    proc << KrServices::fullPathName("kdesu") << KrServices::fullPathName("krusader")
    + " --left=" + MAIN_VIEW->left->func->files()->vfs_getOrigin().url()
    + " --right=" + MAIN_VIEW->right->func->files()->vfs_getOrigin().url();

    if (!proc.startDetached())
        KMessageBox::error(0, i18n("Error executing %1!", proc.program()[0]));
}

// settings slots
void KRslots::configToolbar()
{
    KConfigGroup cg(KGlobal::config(), QString());
    krApp->saveMainWindowSettings(cg);
    QPointer<KEditToolBar> dlg = new KEditToolBar(krApp->factory());
    connect(dlg, SIGNAL(newToolBarConfig()), this, SLOT(saveNewToolbarConfig()));
    if (dlg->exec()) {
        krApp->updateGUI();
    }
    delete dlg;
}

void KRslots::saveNewToolbarConfig()
{
    KConfigGroup cg(KGlobal::config(), QString());
    krApp->applyMainWindowSettings(cg);
    krApp->updateGUI();
}

void KRslots::configKeys()
{
    KrKeyDialog(MAIN_VIEW);
}

// misc
void KRslots::changeTrashIcon()
{
#if 0 // TODO: is this needed?
    // update trash bin icon - this is "stolen" konqi code
    // Copyright (C) 2000  David Faure <faure@kde.org>
    KUrl trash;
    trash.setPath(KGlobalSettings::trashPath());
    KUrl::List lst;
    lst.append(trash);
    KDirNotify_stub allDirNotify("*", "KDirNotify*");
    allDirNotify.FilesChanged(lst);
    // end of konqi code
#endif
}

// F2
void KRslots::terminal()
{
    ACTIVE_FUNC->terminal();
}
// F3
void KRslots::view()
{
    ACTIVE_FUNC->view();
}
// F4
void KRslots::edit()
{
    ACTIVE_FUNC->editFile();
}
// F5
void KRslots::copyFiles()
{
    ACTIVE_FUNC->copyFiles();
}
// F6
void KRslots::moveFiles()
{
    ACTIVE_FUNC->moveFiles();
}
// SHIFT + F5
void KRslots::copyFilesByQueue()
{
    ACTIVE_FUNC->copyFiles(true);
}
// SHIFT + F6
void KRslots::moveFilesByQueue()
{
    ACTIVE_FUNC->moveFiles(true);
}
// F7
void KRslots::mkdir()
{
    ACTIVE_FUNC->mkdir();
}
// F8
void KRslots::deleteFiles(bool reallyDelete)
{
    ACTIVE_FUNC->deleteFiles(reallyDelete);
}
// F9
void KRslots::rename()
{
    ACTIVE_FUNC->rename();
}

// Shift F3
void KRslots::viewDlg()
{
    // ask the user for a url to view
    KUrl dest = KChooseDir::getDir(i18n("Enter a URL to view:"), ACTIVE_PANEL->virtualPath(), ACTIVE_PANEL->virtualPath());
    if (dest.isEmpty()) return ;   // the user canceled

    KrViewer::view(dest);   // view the file
//  }
    // nothing more to it!
}

// Shift F4
void KRslots::editDlg()
{
    // ask the user for the filename to edit
    KUrl dest = KChooseDir::getDir(i18n("Enter the filename to edit:"), ACTIVE_PANEL->virtualPath(), ACTIVE_PANEL->virtualPath());
    if (dest.isEmpty()) return ;   // the user canceled

    KrViewer::edit(dest);

    if (dest.upUrl().equals(ACTIVE_PANEL->virtualPath(), KUrl::CompareWithoutTrailingSlash))
        refresh();
}

void KRslots::duplicateTab()
{
    ACTIVE_PANEL_MANAGER->slotNewTab(ACTIVE_PANEL->virtualPath());
}

// ugly: do this right before release!
void KRslots::newTab(const KUrl& url)
{
    if (url.isValid())
        ACTIVE_PANEL_MANAGER->slotNewTab(url);
    else ACTIVE_PANEL_MANAGER->slotNewTab();
}

void KRslots::nextTab()
{
    ACTIVE_PANEL_MANAGER->slotNextTab();
}

void KRslots::previousTab()
{
    ACTIVE_PANEL_MANAGER->slotPreviousTab();
}

void KRslots::newTab(KrViewItem *it)
{
    if (!it) return;
    if (it->name() == "..") {
        KUrl url = ACTIVE_PANEL->virtualPath();
        ACTIVE_PANEL_MANAGER->slotNewTab(url.upUrl());
    } else if (ITEM2VFILE(ACTIVE_PANEL, it)->vfile_isDir()) {
        KUrl url = ACTIVE_PANEL->virtualPath();
        url.addPath(it->name());
        ACTIVE_PANEL_MANAGER->slotNewTab(url);
    }
}

void KRslots::closeTab()
{
    ACTIVE_PANEL_MANAGER->slotCloseTab();
}

void KRslots::closeInactiveTabs()
{
    ACTIVE_PANEL_MANAGER->slotCloseInactiveTabs();
}

void KRslots::closeDuplicatedTabs()
{
    ACTIVE_PANEL_MANAGER->slotCloseDuplicatedTabs();
}

void KRslots::lockTab()
{
    ACTIVE_PANEL_MANAGER->slotLockTab();
}

void KRslots::slotSplit()
{
    QStringList list;
    QString name;

    ((ListPanel*)ACTIVE_PANEL)->getSelectedNames(&list);

    // first, see if we've got exactly 1 selected file, if not, try the current one
    if (list.count() == 1) name = list[0];

    if (name.isEmpty()) {
        // if we got here, then one of the panel can't be sure what file to diff
        KMessageBox::error(0, i18n("Don't know which file to split."));
        return;
    }

    KUrl fileURL = ACTIVE_FUNC->files()->vfs_getFile(name);
    if (fileURL.isEmpty())
        return;

    if (ACTIVE_FUNC->files()->vfs_search(name)->vfile_isDir()) {
        KMessageBox::sorry(krApp, i18n("You can't split a directory!"));
        return ;
    }

    KUrl destDir  = ACTIVE_PANEL->otherPanel->func->files()->vfs_getOrigin();

    SplitterGUI splitterGUI(MAIN_VIEW, fileURL, destDir);

    if (splitterGUI.result() == QDialog::Accepted) {
        bool splitToOtherPanel = (splitterGUI.getDestinationDir().equals(ACTIVE_PANEL->otherPanel->virtualPath(), KUrl::CompareWithoutTrailingSlash));

        Splitter split(MAIN_VIEW, fileURL, splitterGUI.getDestinationDir());
        split.split(splitterGUI.getSplitSize());

        if (splitToOtherPanel)
            ACTIVE_PANEL->otherPanel->func->refresh();
    }
}

void KRslots::slotCombine()
{
    QStringList   list;
    KUrl          baseURL;
    bool          unixStyle = false;
    bool          windowsStyle = false;
    QString       commonName;
    int           commonLength = 0;

    ((ListPanel*)ACTIVE_PANEL)->getSelectedNames(&list);
    if (list.isEmpty()) {
        KMessageBox::error(0, i18n("Don't know which files to combine."));
        return;
    }

    /* checking splitter names */
    for (QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
        KUrl url = ACTIVE_FUNC->files()->vfs_getFile(*it);
        if (url.isEmpty())
            return;

        if (ACTIVE_FUNC->files()->vfs_search(*it)->vfile_isDir()) {
            KMessageBox::sorry(krApp, i18n("You can't combine a directory!"));
            return ;
        }

        if (!unixStyle) {
            QString name = url.fileName();
            int extPos = name.lastIndexOf('.');
            QString ext = name.mid(extPos + 1);
            name.truncate(extPos);
            url.setFileName(name);

            bool isExtInt;
            ext.toInt(&isExtInt, 10);

            if (extPos < 1 || ext.isEmpty() || (ext != "crc" && !isExtInt)) {
                if (windowsStyle) {
                    KMessageBox::error(0, i18n("Not a split file %1!", url.pathOrUrl()));
                    return;
                }
                unixStyle = true;
            } else {

                if (ext != "crc")
                    windowsStyle = true;

                if (baseURL.isEmpty())
                    baseURL = url;
                else if (baseURL != url) {
                    KMessageBox::error(0, i18n("Select only one split file!"));
                    return;
                }
            }
        }

        if (unixStyle) {
            bool error = true;

            do {
                QString shortName   = *it;
                QChar   lastChar  = shortName.at(shortName.length() - 1);

                if (lastChar.isLetter()) {
                    char fillLetter = (lastChar.toUpper() == lastChar) ? 'A' : 'a';

                    if (commonName.isNull()) {
                        commonLength = shortName.length();
                        commonName = shortName;

                        while (commonName.length()) {
                            QString shorter  = commonName.left(commonName.length() - 1);
                            QString testFile = shorter.leftJustified(commonLength, fillLetter);

                            if (ACTIVE_FUNC->files()->vfs_search(testFile) == 0)
                                break;
                            else {
                                commonName = shorter;
                                baseURL = ACTIVE_FUNC->files()->vfs_getOrigin();
                                baseURL.addPath(testFile);
                            }
                        }

                        error = (commonName == shortName);
                    } else if (commonLength == shortName.length() && shortName.startsWith(commonName))
                        error = false;
                }
            } while (false);

            if (error) {
                KMessageBox::error(0, i18n("Not a splitted file %1!", url.pathOrUrl()));
                return;
            }
        }
    }

    // ask the user for the copy dest
    KUrl dest = KChooseDir::getDir(i18n("Combining %1.* to directory:", baseURL.pathOrUrl()),
                                   ACTIVE_PANEL->otherPanel->virtualPath(), ACTIVE_PANEL->virtualPath());
    if (dest.isEmpty()) return ;   // the user canceled

    bool combineToOtherPanel = (dest.equals(ACTIVE_PANEL->otherPanel->virtualPath(), KUrl::CompareWithoutTrailingSlash));

    Combiner combine(MAIN_VIEW, baseURL, dest, unixStyle);
    combine.combine();

    if (combineToOtherPanel)
        ACTIVE_PANEL->otherPanel->func->refresh();
}

void KRslots::userMenu()
{
    //UserMenu um;
    //um.exec();
    krApp->userMenu->exec();
}

void KRslots::manageUseractions()
{
    ActionMan actionMan(MAIN_VIEW);
}

void KRslots::slotSynchronizeDirs(QStringList selected)
{
    new SynchronizerGUI(0, MAIN_VIEW->left->func->files()->vfs_getOrigin(),
                        MAIN_VIEW->right->func->files()->vfs_getOrigin(), selected);
}

void KRslots::slotSyncBrowse()
{
    ACTIVE_PANEL->syncBrowseButton->toggle();
}

void KRslots::updatePopupPanel(KrViewItem *item)
{
    // which panel to display on?
    ListPanel *lp = 0;
    if (ACTIVE_PANEL->popup->isHidden() &&
            ACTIVE_PANEL->otherPanel->popup->isHidden()) return;
    if (!ACTIVE_PANEL->popup->isHidden())
        lp = ACTIVE_PANEL;
    else if (!ACTIVE_PANEL->otherPanel->popup->isHidden())
        lp = ACTIVE_PANEL->otherPanel;

    KUrl url;
    if (item->name() != "..") // updir
        url = ACTIVE_FUNC->files()->vfs_getFile(item->name());
    lp->popup->update(url);
}

void KRslots::compareDirs()
{
    ACTIVE_PANEL->compareDirs();
    ACTIVE_PANEL->otherPanel->compareDirs();
}

void KRslots::compareSetup()
{
    for (int i = 0; Krusader::compareArray[i] != 0; i++)
        if ((*Krusader::compareArray[i])->isChecked()) {
            KConfigGroup group(krConfig, "Private");
            group.writeEntry("Compare Mode", i);
            break;
        }
}

/** called by actions actExec* to choose the built-in command line mode  */
void KRslots::execTypeSetup()
{
    for (int i = 0; Krusader::execTypeArray[i] != 0; i++)
        if ((*Krusader::execTypeArray[i])->isChecked()) {
            if (*Krusader::execTypeArray[i] == Krusader::actExecTerminalEmbedded) {
                // if commands are to be executed in the TE, it must be loaded
                MAIN_VIEW->terminal_dock->initialise();
            }
            KConfigGroup grp(krConfig,  "Private");
            grp.writeEntry("Command Execution Mode", i);
            break;
        }
}

void KRslots::togglePopupPanel()
{
    ACTIVE_PANEL->togglePanelPopup();
}

void KRslots::slotDiskUsage()
{
    DiskUsageGUI du(ACTIVE_FUNC->files()->vfs_getOrigin(), MAIN_VIEW);
}

void KRslots::slotQueueManager()
{
    QueueDialog::showDialog(false);
}

// when window becomes focused, enable the refresh in the visible panels
void KRslots::windowActive()
{
    if (MAIN_VIEW != 0) {  /* CRASH FIX: it's possible that the method is called after destroying the main view */
        MAIN_VIEW->left->panelActive();
        MAIN_VIEW->right->panelActive();
    }
}

// when another application becomes focused, do a windows-commander style refresh: don't
// refresh at all until krusader becomes focused again
void KRslots::windowInactive()
{
    if (MAIN_VIEW != 0) {  /* CRASH FIX: it's possible that the method is called after destroying the main view */
        MAIN_VIEW->left->panelInactive();
        MAIN_VIEW->right->panelInactive();
    }
}

void KRslots::slotLocationBar()
{
    ACTIVE_PANEL->origin->lineEdit()->selectAll();
    ACTIVE_PANEL->origin->setFocus();
}

void KRslots::slotJumpBack()
{
    ACTIVE_PANEL->jumpBack();
}

void KRslots::slotSetJumpBack()
{
    ACTIVE_PANEL->setJumpBack(ACTIVE_PANEL->virtualPath());
}

void KRslots::emptyTrash()
{
    KrTrashHandler::emptyTrash();
}

#define OPEN_ID        100001
#define EMPTY_TRASH_ID 100002

void KRslots::trashBin()
{
    QMenu trashMenu(krApp);
    QAction * act = trashMenu.addAction(krLoader->loadIcon("document-open", KIconLoader::Panel), i18n("Open trashbin"));
    act->setData(QVariant(OPEN_ID));
    act = trashMenu.addAction(krLoader->loadIcon("trash-empty", KIconLoader::Panel), i18n("Empty trashbin"));
    act->setData(QVariant(EMPTY_TRASH_ID));

    int result = -1;
    QAction *res = trashMenu.exec(QCursor::pos());
    if (res && res->data().canConvert<int> ())
        result = res->data().toInt();

    if (result == OPEN_ID) {
        ACTIVE_FUNC->openUrl(KUrl("trash:/"));
    } else if (result == EMPTY_TRASH_ID) {
        KrTrashHandler::emptyTrash();
    }
}

//shows the JavaScript-Console
void KRslots::jsConsole()
{
#ifdef __KJSEMBED__
    if (! krJS)
        krJS = new KrJS();
    krJS->view()->show();
#endif
}

void KRslots::bookmarkCurrent()
{
    krBookMan->bookmarkCurrent(ACTIVE_PANEL->virtualPath());
}

#include "krslots.moc"
