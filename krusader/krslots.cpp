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

// QtCore
#include <QDir>
#include <QPoint>
#include <QStringList>
#include <QList>
#include <QEvent>
#include <QTemporaryFile>
// QtGui
#include <QPixmapCache>
#include <QKeyEvent>
// QtWidgets
#include <QApplication>

#include <KConfigCore/KSharedConfig>
#include <KCoreAddons/KShell>
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>
#include <KWidgetsAddons/KToggleAction>
#include <KWidgetsAddons/KMessageBox>

#ifdef __KJSEMBED__
#include <kjsembed/jsconsolewidget.h>
#include "KrJS/krjs.h"
#endif

#include "KViewer/krviewer.h"
#include "Panel/krviewfactory.h"
#include "krusader.h"
#include "kractions.h"
#include "krusaderview.h"
#include "Panel/listpanel.h"
#include "Panel/krselectionmode.h"
#include "Dialogs/krdialogs.h"
#include "Dialogs/krspwidgets.h"
#include "GUI/krusaderstatus.h"
#include "Panel/panelfunc.h"
#include "Konfigurator/konfigurator.h"
#include "MountMan/kmountman.h"
#include "defaults.h"
#include "GUI/kfnkeys.h"
#include "GUI/kcmdline.h"
#include "GUI/terminaldock.h"
#include "GUI/syncbrowsebutton.h"
#include "GUI/mediabutton.h"
#include "GUI/dirhistorybutton.h"
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
#include "Dialogs/krspecialwidgets.h"
#include "DiskUsage/diskusagegui.h"
#include "krservices.h"
#include "Panel/krviewitem.h"
#include "krtrashhandler.h"
#include "BookMan/krbookmarkhandler.h"
#include "BookMan/krbookmarkbutton.h"

#ifdef SYNCHRONIZER_ENABLED
    #include "Synchronizer/synchronizergui.h"
#endif


#define ACTIVE_VIEW _mainWindow->activeView()


KRslots::KRslots(QObject *parent) : QObject(parent), _mainWindow(krApp)
{
}

void KRslots::sendFileByEmail(const QList<QUrl> &urls)
{
    if (urls.count() == 0) {
        KMessageBox::error(0, i18n("No selected files to send."));
        return;
    }

    QString mailProg;
    QStringList lst = KrServices::supportedTools();
    if (lst.contains("MAIL")) mailProg = lst[lst.indexOf("MAIL") + 1];
    else {
        KMessageBox::error(0, i18n("Krusader cannot find a supported mail client. Please install one to your path. Hint: Krusader supports KMail."));
        return;
    }

    QString subject, separator;
    foreach(const QUrl &url, urls) {
        subject += separator + url.fileName();
        separator = ',';
    }
    subject = i18np("Sending file: %2", "Sending files: %2", urls.count(), subject);

    KProcess proc;

    QString executable = QUrl::fromLocalFile(mailProg).fileName();
    if (executable == QStringLiteral("kmail")) {
        proc << mailProg << "--subject"
        << subject;
        foreach(const QUrl &url2, urls)
        proc << "--attach" << url2.toDisplayString();
    } else if (executable == QStringLiteral("thunderbird")) {
        QString param = "attachment=\'";
        separator = "";
        foreach(const QUrl &url2, urls) {
            param += separator + url2.toDisplayString();
            separator = ',';
        }
        param += "\',subject=\'" + subject + "\'";
        proc << mailProg << "--compose" << param;
    } else if (executable == QStringLiteral("evolution")) {
        QString param = "mailto:?cc=&subject=" + subject + "&attach=";
        separator = "";
        foreach(const QUrl &url2, urls) {
            param += separator + url2.toDisplayString();
            separator = "&attach=";
        }
        proc << mailProg << param + "";
    }

    if (!proc.startDetached())
        KMessageBox::error(0, i18n("Error executing %1.", mailProg));
}

void KRslots::compareContent()
{
    QStringList lstLeft, lstRight;
    QStringList* lstActive;
    QUrl name1, name2;

    LEFT_PANEL->getSelectedNames(&lstLeft);
    RIGHT_PANEL->getSelectedNames(&lstRight);
    lstActive = (ACTIVE_PANEL->gui->isLeft() ? &lstLeft : &lstRight);

    if (lstLeft.count() == 1 && lstRight.count() == 1) {
        // first, see if we've got exactly 1 selected file in each panel:
        name1 = LEFT_PANEL->func->files()->getUrl(lstLeft[0]);
        name2 = RIGHT_PANEL->func->files()->getUrl(lstRight[0]);
    } else if (lstActive->count() == 2) {
        // next try: are in the current panel exacty 2 files selected?
        name1 = ACTIVE_PANEL->func->files()->getUrl((*lstActive)[0]);
        name2 = ACTIVE_PANEL->func->files()->getUrl((*lstActive)[1]);
    } else if (ACTIVE_PANEL->otherPanel()->func->files()->getVfile(ACTIVE_VIEW->getCurrentItem())) {
        // next try: is in the other panel a file with the same name?
        name1 = ACTIVE_PANEL->func->files()->getUrl(ACTIVE_VIEW->getCurrentItem());
        name2 = ACTIVE_PANEL->otherPanel()->func->files()->getUrl(ACTIVE_VIEW->getCurrentItem());
    } else  {
        // if we got here, then we can't be sure what file to diff
        KMessageBox::detailedError(0, i18n("Do not know which files to compare."), "<qt>" + i18n("To compare two files by content, you can either:<ul><li>Select one file in the left panel, and one in the right panel.</li><li>Select exactly two files in the active panel.</li><li>Make sure there is a file in the other panel, with the same name as the current file in the active panel.</li></ul>") + "</qt>");

        return;
    }

    // else implied: all ok, let's call an external program to compare files
    // but if any of the files isn't local, download it first
    compareContent(name1, name2);
}

bool downloadToTemp(const QUrl &url, QString &dest) {
    QTemporaryFile tmpFile;
    tmpFile.setAutoRemove(false);
    if (tmpFile.open()) {
        dest = tmpFile.fileName();
        KIO::Job* job = KIO::file_copy(url, QUrl::fromLocalFile(dest), -1,
                                       KIO::Overwrite | KIO::HideProgressInfo);
        if(!job->exec()) {
            KMessageBox::error(krApp, i18n("Krusader is unable to download %1", url.fileName()));
            return false;
        }
        return true;
    }
    return false;
}

void KRslots::compareContent(QUrl url1, QUrl url2)
{
    QString diffProg;
    QStringList lst = KrServices::supportedTools();
    if (lst.contains("DIFF")) diffProg = lst[lst.indexOf("DIFF") + 1];
    else {
        KMessageBox::error(0, i18n("Krusader cannot find any of the supported diff-frontends. Please install one to your path. Hint: Krusader supports Kompare, KDiff3 and Xxdiff."));
        return;
    }

    QString tmp1;
    QString tmp2;

    // kdiff3 sucks with spaces
    if (QUrl::fromLocalFile(diffProg).fileName() == "kdiff3" && !url1.toDisplayString().contains(" ") && !url2.toDisplayString().contains(" ")) {
        tmp1 = url1.toDisplayString();
        tmp2 = url2.toDisplayString();
    } else {
        if (!url1.isLocalFile()) {
            if (!downloadToTemp(url1, tmp1)) {
                return;
            }
        } else tmp1 = url1.path();
        if (!url2.isLocalFile()) {
            if (!downloadToTemp(url2, tmp2)) {
                if (tmp1 != url1.path())
                    QFile::remove(tmp1);
                return;
            }
        } else tmp2 = url2.path();
    }

    KrProcess *p = new KrProcess(tmp1 != url1.path() ? tmp1 : QString(),
                                 tmp2 != url2.path() ? tmp2 : QString());
    *p << diffProg << tmp1 << tmp2;
    p->start();
    if (!p->waitForStarted())
        KMessageBox::error(0, i18n("Error executing %1.", diffProg));
}

// GUI toggle slots
void KRslots::toggleFnkeys()
{
    if (MAIN_VIEW->fnKeys()->isVisible())
        MAIN_VIEW->fnKeys()->hide();
    else MAIN_VIEW->fnKeys()->show();
}

void KRslots::toggleCmdline()
{
    if (MAIN_VIEW->cmdLine()->isVisible()) MAIN_VIEW->cmdLine()->hide();
    else MAIN_VIEW->cmdLine()->show();
}

void KRslots::updateStatusbarVisibility()
{
    krApp->statusBar()->setVisible(KrActions::actShowStatusBar->isChecked());
}

void KRslots::toggleTerminal()
{
    if (MAIN_VIEW->terminalDock()->isVisible()) MAIN_VIEW->slotTerminalEmulator(false);
    else MAIN_VIEW->slotTerminalEmulator(true);
}

void KRslots::insertFileName(bool full_path)
{
    QString filename = ACTIVE_VIEW->getCurrentItem();
    if (filename.isEmpty()) {
        return;
    }

    if (full_path) {
        QString path = vfs::ensureTrailingSlash(ACTIVE_FUNC->files()->currentDirectory()).toDisplayString(QUrl::PreferLocalFile);
        filename = path + filename;
    }

    filename = KrServices::quote(filename);

    if (MAIN_VIEW->cmdLine()->isVisible() || !MAIN_VIEW->terminalDock()->isTerminalVisible()) {
        QString current = MAIN_VIEW->cmdLine()->text();
        if (current.length() != 0 && !current.endsWith(' '))
            current += ' ';
        MAIN_VIEW->cmdLine()->setText(current + filename);
        MAIN_VIEW->cmdLine()->setFocus();
    } else if (MAIN_VIEW->terminalDock()->isTerminalVisible()) {
        filename = QChar(' ') + filename + QChar(' ');
        MAIN_VIEW->terminalDock()->sendInput(filename);
        MAIN_VIEW->terminalDock()->setFocus();
    }
}

void KRslots::refresh(const QUrl &u)
{
    ACTIVE_FUNC->openUrl(u);
}

void KRslots::runKonfigurator(bool firstTime)
{
    Konfigurator *konfigurator = new Konfigurator(firstTime);
    connect(konfigurator, SIGNAL(configChanged(bool)), SLOT(configChanged(bool)));

    //FIXME - no need to exec
    konfigurator->exec();

    delete konfigurator;
}

void KRslots::configChanged(bool isGUIRestartNeeded)
{
    krConfig->sync();

    if (isGUIRestartNeeded) {
        krApp->setUpdatesEnabled(false);
        KConfigGroup group(krConfig, "Look&Feel");
        vfile::vfile_loadUserDefinedFolderIcons(group.readEntry("Load User Defined Folder Icons", _UserDefinedFolderIcons));

        bool leftActive = ACTIVE_PANEL->gui->isLeft();
        MAIN_VIEW->leftManager()->slotRecreatePanels();
        MAIN_VIEW->rightManager()->slotRecreatePanels();
        if(leftActive)
            LEFT_PANEL->slotFocusOnMe();
        else
            RIGHT_PANEL->slotFocusOnMe();
        MAIN_VIEW->fnKeys()->updateShortcuts();
        KrSelectionMode::resetSelectionHandler();
        krApp->setUpdatesEnabled(true);
    }

    // really ugly, but reload the Fn keys just in case - csaba: any better idea?
    MAIN_VIEW->fnKeys()->updateShortcuts();

    bool showHidden = KConfigGroup(krConfig, "Look&Feel").readEntry("Show Hidden", KrActions::actToggleHidden->isChecked());

    if (showHidden != KrActions::actToggleHidden->isChecked()) {
        KrActions::actToggleHidden->setChecked(showHidden);
        MAIN_VIEW->leftManager()->refreshAllTabs();
        MAIN_VIEW->rightManager()->refreshAllTabs();
    }
}

void KRslots::showHiddenFiles(bool show)
{
    KConfigGroup group(krConfig, "Look&Feel");
    group.writeEntry("Show Hidden", show);

    MAIN_VIEW->leftManager()->refreshAllTabs();
    MAIN_VIEW->rightManager()->refreshAllTabs();
}

void KRslots::swapPanels()
{
    QUrl leftURL = LEFT_PANEL->func->files()->currentDirectory();
    QUrl rightURL = RIGHT_PANEL->func->files()->currentDirectory();

    LEFT_PANEL->func->openUrl(rightURL);
    RIGHT_PANEL->func->openUrl(leftURL);
}

void KRslots::toggleSwapSides()
{
    MAIN_VIEW->swapSides();
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
        KMessageBox::error(krApp, i18n("Cannot find the 'locate' command. Please install the "
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

void KRslots::runTerminal(const QString & dir)
{
    KProcess proc;
    proc.setWorkingDirectory(dir);
    KConfigGroup group(krConfig, "General");
    QString term = group.readEntry("Terminal", _Terminal);
    QStringList sepdArgs = KShell::splitArgs(term, KShell::TildeExpand);
    if (sepdArgs.isEmpty()) {
        KMessageBox::error(krMainWindow,
                           i18nc("Arg is a string containing the bad quoting.",
                                 "Bad quoting in terminal command:\n%1", term));
        return;
    }
    for (int i = 0; i < sepdArgs.size(); i++) {
        if (sepdArgs[i] == "%d") {
            sepdArgs[i] = dir;
        }
    }
    proc << sepdArgs;
    if (!proc.startDetached())
        KMessageBox::sorry(krApp, i18n("Error executing %1.", term));
}

void KRslots::homeTerminal()
{
    runTerminal(QDir::homePath());
}

void KRslots::sysInfo()
{
    KProcess proc;
    proc << KrServices::fullPathName("kcmshell") << "System/ksysctrl";
    if (!proc.startDetached()) {
        KMessageBox::sorry(krApp, i18n("Cannot find \"KsysCtrl\". Please install KDE admin package"));
    }
}

void KRslots::multiRename()
{
    QStringList lst = KrServices::supportedTools();
    int i = lst.indexOf("RENAME");
    if (i == -1) {
        KMessageBox::sorry(krApp, i18n("Cannot find a batch rename tool.\nYou can get KRename at http://www.krename.net"));
        return;
    }
    QString pathToRename = lst[i+1];

    QStringList names;
    ACTIVE_PANEL->gui->getSelectedNames(&names);

    if (names.isEmpty()) {
        return;
    }

    KProcess proc;
    proc << pathToRename;

    for (const QString name: names) {
        vfile *file = ACTIVE_FUNC->files()->getVfile(name);
        if (!file)
            continue;
        const QUrl url = file->vfile_getUrl();
        // KRename only supports the recursive option combined with a local directory path
        if (file->vfile_isDir() && url.scheme() == "file") {
            proc << "-r" << url.path();
        } else {
            proc << url.toString();
        }
    }

    if (!proc.startDetached())
        KMessageBox::error(0, i18n("Error executing '%1'.", proc.program().join(" ")));
}

void KRslots::rootKrusader()
{
    if (!KrServices::cmdExist("krusader") || !KrServices::cmdExist("kdesu")) {
        KMessageBox::sorry(krApp, i18n("Cannot start root mode Krusader, because Krusader or kdesu is missing from the path. Please configure the dependencies in Konfigurator."));
        return;
    }

    KProcess proc;
    proc << KrServices::fullPathName("kdesu") << "-c" << KrServices::fullPathName("krusader")
    + " --left=" + KrServices::quote(LEFT_PANEL->func->files()->currentDirectory().toDisplayString(QUrl::PreferLocalFile))
    + " --right=" + KrServices::quote(RIGHT_PANEL->func->files()->currentDirectory().toDisplayString(QUrl::PreferLocalFile));

    if (!proc.startDetached())
        KMessageBox::error(0, i18n("Error executing %1.", proc.program()[0]));
}

void KRslots::slotSplit()
{
    QStringList list;
    QString name;

    ACTIVE_PANEL->gui->getSelectedNames(&list);

    // first, see if we've got exactly 1 selected file, if not, try the current one
    if (list.count() == 1) name = list[0];

    if (name.isEmpty()) {
        // if we got here, then one of the panel can't be sure what file to diff
        KMessageBox::error(0, i18n("Do not know which file to split."));
        return;
    }

    QUrl fileURL = ACTIVE_FUNC->files()->getUrl(name);
    if (fileURL.isEmpty())
        return;

    if (ACTIVE_FUNC->files()->getVfile(name)->vfile_isDir()) {
        KMessageBox::sorry(krApp, i18n("You cannot split a folder."));
        return ;
    }

    QUrl destDir = ACTIVE_PANEL->otherPanel()->func->files()->currentDirectory();

    SplitterGUI splitterGUI(MAIN_VIEW, fileURL, destDir);

    if (splitterGUI.exec() == QDialog::Accepted) {
        bool splitToOtherPanel = splitterGUI.getDestinationDir().matches(ACTIVE_PANEL->otherPanel()->virtualPath(),
                                                                         QUrl::StripTrailingSlash);

        Splitter split(MAIN_VIEW, fileURL, splitterGUI.getDestinationDir(), splitterGUI.overWriteFiles());
        split.split(splitterGUI.getSplitSize());

        if (splitToOtherPanel)
            ACTIVE_PANEL->otherPanel()->func->refresh();
    }
}

void KRslots::slotCombine()
{
    QStringList   list;
    QUrl          baseURL;
    bool          unixStyle = false;
    bool          windowsStyle = false;
    QString       commonName;
    int           commonLength = 0;

    ACTIVE_PANEL->gui->getSelectedNames(&list);
    if (list.isEmpty()) {
        KMessageBox::error(0, i18n("Do not know which files to combine."));
        return;
    }

    /* checking splitter names */
    for (QStringList::Iterator it = list.begin(); it != list.end(); ++it) {
        QUrl url = ACTIVE_FUNC->files()->getUrl(*it);
        if (url.isEmpty())
            return;

        if (ACTIVE_FUNC->files()->getVfile(*it)->vfile_isDir()) {
            KMessageBox::sorry(krApp, i18n("You cannot combine a folder."));
            return ;
        }

        if (!unixStyle) {
            QString name = url.fileName();
            int extPos = name.lastIndexOf('.');
            QString ext = name.mid(extPos + 1);
            name.truncate(extPos);
            url = url.adjusted(QUrl::RemoveFilename);
            url.setPath(url.path() + name);

            bool isExtInt;
            ext.toInt(&isExtInt, 10);

            if (extPos < 1 || ext.isEmpty() || (ext != "crc" && !isExtInt)) {
                if (windowsStyle) {
                    KMessageBox::error(0, i18n("Not a split file: %1.", url.toDisplayString(QUrl::PreferLocalFile)));
                    return;
                }
                unixStyle = true;
            } else {

                if (ext != "crc")
                    windowsStyle = true;

                if (baseURL.isEmpty())
                    baseURL = url;
                else if (baseURL != url) {
                    KMessageBox::error(0, i18n("Select only one split file."));
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

                            if (ACTIVE_FUNC->files()->getVfile(testFile) == 0)
                                break;
                            else {
                                commonName = shorter;
                                baseURL = ACTIVE_FUNC->files()->currentDirectory().adjusted(QUrl::StripTrailingSlash);
                                baseURL.setPath(baseURL.path() + '/' + (testFile));
                            }
                        }

                        error = (commonName == shortName);
                    } else if (commonLength == shortName.length() && shortName.startsWith(commonName))
                        error = false;
                }
            } while (false);

            if (error) {
                KMessageBox::error(0, i18n("Not a split file: %1.", url.toDisplayString(QUrl::PreferLocalFile)));
                return;
            }
        }
    }

    // ask the user for the copy dest
    QUrl dest = KChooseDir::getDir(i18n("Combining %1.* to folder:", baseURL.toDisplayString(QUrl::PreferLocalFile)),
                                   ACTIVE_PANEL->otherPanel()->virtualPath(), ACTIVE_PANEL->virtualPath());
    if (dest.isEmpty()) return ;   // the user canceled

    bool combineToOtherPanel = (dest.matches(ACTIVE_PANEL->otherPanel()->virtualPath(), QUrl::StripTrailingSlash));

    Combiner combine(MAIN_VIEW, baseURL, dest, unixStyle);
    combine.combine();

    if (combineToOtherPanel)
        ACTIVE_PANEL->otherPanel()->func->refresh();
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

#ifdef SYNCHRONIZER_ENABLED
void KRslots::slotSynchronizeDirs(QStringList selected)
{
    new SynchronizerGUI(0, LEFT_PANEL->func->files()->currentDirectory(),
                        RIGHT_PANEL->func->files()->currentDirectory(), selected);
}
#endif

void KRslots::compareSetup()
{
    for (int i = 0; KrActions::compareArray[i] != 0; i++)
        if ((*KrActions::compareArray[i])->isChecked()) {
            KConfigGroup group(krConfig, "Private");
            group.writeEntry("Compare Mode", i);
            break;
        }
}

/** called by actions actExec* to choose the built-in command line mode  */
void KRslots::execTypeSetup()
{
    for (int i = 0; KrActions::execTypeArray[i] != 0; i++)
        if ((*KrActions::execTypeArray[i])->isChecked()) {
            if (*KrActions::execTypeArray[i] == KrActions::actExecTerminalEmbedded) {
                // if commands are to be executed in the TE, it must be loaded
                MAIN_VIEW->terminalDock()->initialise();
            }
            KConfigGroup grp(krConfig,  "Private");
            grp.writeEntry("Command Execution Mode", i);
            break;
        }
}

void KRslots::slotDiskUsage()
{
    DiskUsageGUI du(ACTIVE_FUNC->files()->currentDirectory(), MAIN_VIEW);
}

void KRslots::applicationStateChanged()
{
    if (MAIN_VIEW == 0) {  /* CRASH FIX: it's possible that the method is called after destroying the main view */
        return;
    }
    if(qApp->applicationState() == Qt::ApplicationActive) {
        LEFT_PANEL->panelActive();
        RIGHT_PANEL->panelActive();
    } else {
        LEFT_PANEL->panelInactive();
        RIGHT_PANEL->panelInactive();
    }
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
    QAction * act = trashMenu.addAction(krLoader->loadIcon("document-open", KIconLoader::Panel), i18n("Open trash bin"));
    act->setData(QVariant(OPEN_ID));
    act = trashMenu.addAction(krLoader->loadIcon("trash-empty", KIconLoader::Panel), i18n("Empty trash bin"));
    act->setData(QVariant(EMPTY_TRASH_ID));

    int result = -1;
    QAction *res = trashMenu.exec(QCursor::pos());
    if (res && res->data().canConvert<int> ())
        result = res->data().toInt();

    if (result == OPEN_ID) {
        ACTIVE_FUNC->openUrl(QUrl(QStringLiteral("trash:/")));
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

void KRslots::addBookmark()
{
    krBookMan->bookmarkCurrent(ACTIVE_PANEL->virtualPath());
}

void KRslots::cmdlinePopup()
{
    MAIN_VIEW->cmdLine()->popup();
}

