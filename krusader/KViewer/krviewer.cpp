/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
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

#include "krviewer.h"

#include <QDataStream>
#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QKeyEvent>
#include <QEvent>

#include <kmenubar.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kparts/componentfactory.h>
#include <kmessagebox.h>
#include <klibloader.h>
#include <kio/netaccess.h>
#include <kio/jobclasses.h>
#include <kio/job.h>
#include <kstatusbar.h>
#include <kdebug.h>
#include <kde_file.h>
#include <khtml_part.h>
#include <kprocess.h>
#include <kfileitem.h>
#include <ktoolbar.h>
#include <kstandardaction.h>

#include "../krglobal.h"
#include "../defaults.h"
#include "../kicons.h"
#include "panelviewer.h"

#define VIEW_ICON     "zoom-original"
#define EDIT_ICON     "edit"
#define MODIFIED_ICON "document-save-as"

#define CHECK_MODFIED_INTERVAL 500

QList<KrViewer *> KrViewer::viewers;

KrViewer::KrViewer(QWidget *parent) :
        KParts::MainWindow(parent, (Qt::WindowFlags)KDE_DEFAULT_WINDOWFLAGS), manager(this, this), tabBar(this), returnFocusTo(0), returnFocusTab(0),
        reservedKeys(), reservedKeyActions(), sizeX(-1), sizeY(-1)
{

    //setWFlags(Qt::WType_TopLevel | WDestructiveClose);
    setXMLFile("krviewer.rc");   // kpart-related xml file
    setHelpMenuEnabled(false);

    connect(&manager, SIGNAL(activePartChanged(KParts::Part*)),
            this, SLOT(createGUI(KParts::Part*)));
    connect(&tabBar, SIGNAL(currentChanged(QWidget *)),
            this, SLOT(tabChanged(QWidget*)));
    connect(&tabBar, SIGNAL(closeRequest(QWidget *)),
            this, SLOT(tabCloseRequest(QWidget*)));

    tabBar.setDocumentMode(true);
    tabBar.setMovable(true);
    tabBar.setTabReorderingEnabled(false);
    tabBar.setAutomaticResizeTabs(true);
// "edit"
// "document-save-as"
    setCentralWidget(&tabBar);

    printAction = KStandardAction::print(this, SLOT(print()), 0);
    copyAction = KStandardAction::copy(this, SLOT(copy()), 0);

    viewerMenu = new QMenu(this);
    viewerMenu->addAction(i18n("&Generic Viewer"), this, SLOT(viewGeneric()))->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_G);
    viewerMenu->addAction(i18n("&Text Viewer"), this, SLOT(viewText()))->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_T);
    viewerMenu->addAction(i18n("&Hex Viewer"), this, SLOT(viewHex()))->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_H);
    viewerMenu->addAction(i18n("&Lister"), this, SLOT(viewLister()))->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_L);
    viewerMenu->addSeparator();
    viewerMenu->addAction(i18n("Text &Editor"), this, SLOT(editText()))->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_E);
    viewerMenu->addSeparator();
    viewerMenu->addAction(i18n("&Next Tab"), this, SLOT(nextTab()))->setShortcut(Qt::ALT + Qt::Key_Right);
    viewerMenu->addAction(i18n("&Previous Tab"), this, SLOT(prevTab()))->setShortcut(Qt::ALT + Qt::Key_Left);

    detachAction = viewerMenu->addAction(i18n("&Detach Tab"), this, SLOT(detachTab()));
    detachAction->setShortcut(Qt::META + Qt::Key_D);
    //no point in detaching only one tab..
    detachAction->setEnabled(false);
    viewerMenu->addSeparator();
    QList<QAction *> actList = menuBar()->actions();
    bool hasPrint = false, hasCopy = false;
    foreach(QAction *a, actList) {
        if (a->shortcut().matches(printAction->shortcut().primary()) != QKeySequence::NoMatch)
            hasPrint = true;
        if (a->shortcut().matches(copyAction->shortcut().primary()) != QKeySequence::NoMatch)
            hasCopy = true;
    }
    QAction *printAct = viewerMenu->addAction(printAction->icon(), printAction->text(), this, SLOT(print()));
    if (hasPrint)
        printAct->setShortcut(printAction->shortcut().primary());
    QAction *copyAct = viewerMenu->addAction(copyAction->icon(), copyAction->text(), this, SLOT(copy()));
    if (hasCopy)
        copyAct->setShortcut(copyAction->shortcut().primary());
    viewerMenu->addSeparator();
    (tabClose = viewerMenu->addAction(i18n("&Close Current Tab"), this, SLOT(tabCloseRequest())))->setShortcut(Qt::Key_Escape);
    (closeAct = viewerMenu->addAction(i18n("&Quit"), this, SLOT(close())))->setShortcut(Qt::CTRL + Qt::Key_Q);

    tabBar.setHoverCloseButton(true);

    checkModified();

    KConfigGroup group(krConfig, "KrViewerWindow");
    int sx = group.readEntry("Window Width", -1);
    int sy = group.readEntry("Window Height", -1);

    if (sx != -1 && sy != -1)
        resize(sx, sy);
    else
        resize(900, 700);

    if (group.readEntry("Window Maximized",  false)) {
        setWindowState(windowState() | Qt::WindowMaximized);
    }
}

KrViewer::~KrViewer()
{

    disconnect(&manager, SIGNAL(activePartChanged(KParts::Part*)),
               this, SLOT(createGUI(KParts::Part*)));

    viewers.removeAll(this);

    // close tabs before deleting tab bar - this avoids Qt bug 26115
    // https://bugreports.qt-project.org/browse/QTBUG-26115
    while(tabBar.count())
        tabCloseRequest();

    delete printAction;
    delete copyAction;
}

void KrViewer::configureDeps()
{
    PanelEditor::configureDeps();
}

void KrViewer::createGUI(KParts::Part* part)
{
    /* TODO: fix the toolbar bugs */
    if (part == 0)     /*     KHTMLPart calls this function with 0 at destruction.    */
        return ;        /*   Can cause crash after JavaScript self.close() if removed  */


    // and show the new part widget
    connect(part, SIGNAL(setStatusBarText(const QString&)),
            this, SLOT(slotSetStatusBarText(const QString&)));

    KParts::MainWindow::createGUI(part);

    PanelViewerBase *pvb = getPanelViewerBase(part);
    if (pvb)
        updateActions(pvb);

    toolBar() ->show();
    statusBar() ->show();

    // the KParts part may override the viewer shortcuts. We prevent it
    // by installing an event filter on the menuBar() and the part
    reservedKeys.clear();
    reservedKeyActions.clear();

    QList<QAction *> list = viewerMenu->actions();
    // getting the key sequences of the viewer menu
    for (int w = 0; w != list.count(); w++) {
        QAction *act = list[ w ];
        QList<QKeySequence> sequences = act->shortcuts();
        if (!sequences.isEmpty()) {
            for (int i = 0; i != sequences.count(); i++) {
                reservedKeys.push_back(sequences[ i ]);
                reservedKeyActions.push_back(act);
            }
        }
    }

    // and "fix" the menubar
    QList<QAction *> actList = menuBar()->actions();
    viewerMenu->setTitle(i18n("&KrViewer"));
    QAction * act = menuBar() ->addMenu(viewerMenu);
    act->setData(QVariant(70));
    menuBar() ->show();

    // filtering out the key events
    menuBar() ->installEventFilter(this);
    part->installEventFilter(this);
}

bool KrViewer::eventFilter(QObject * /* watched */, QEvent * e)
{
    if (e->type() == QEvent::ShortcutOverride) {
        QKeyEvent* ke = (QKeyEvent*) e;
        if (reservedKeys.contains(ke->key())) {
            ke->accept();

            QAction *act = reservedKeyActions[ reservedKeys.indexOf(ke->key())];
            if (act != 0) {
                // don't activate the close functions immediately!
                // it can cause crash
                if (act == tabClose)
                    QTimer::singleShot(0, this, SLOT(tabCloseRequest()));
                else if (act == closeAct)
                    QTimer::singleShot(0, this, SLOT(close()));
                else {
                    act->activate(QAction::Trigger);
                }
            }
            return true;
        }
    } else if (e->type() == QEvent::KeyPress) {
        QKeyEvent* ke = (QKeyEvent*) e;
        if (reservedKeys.contains(ke->key())) {
            ke->accept();
            return true;
        }
    }
    return false;
}
void KrViewer::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_F10:
        close();
        break;
    case Qt::Key_Escape:
        tabCloseRequest();
        break;
    default:
        e->ignore();
        break;
    }
}

KrViewer* KrViewer::getViewer(bool new_window)
{
    if (!new_window) {
        if (viewers.isEmpty()) {
            viewers.prepend(new KrViewer());   // add to first (active)
        } else {
            if (viewers.first()->isMinimized()) { // minimized? -> show it again
                viewers.first()->setWindowState((viewers.first()->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
                viewers.first()->show();
            }
            viewers.first()->raise();
            viewers.first()->activateWindow();
        }
        return viewers.first();
    } else {
        KrViewer *newViewer = new KrViewer();
        viewers.prepend(newViewer);
        return newViewer;
    }
}

void KrViewer::view(KUrl url, QWidget * parent)
{
    KConfigGroup group(krConfig, "General");
    bool defaultWindow = group.readEntry("View In Separate Window", _ViewInSeparateWindow);

    view(url, Default, defaultWindow, parent);
}

void KrViewer::view(KUrl url, Mode mode,  bool new_window, QWidget * parent)
{
    KrViewer* viewer = getViewer(new_window);

    PanelViewerBase* viewWidget = new PanelViewer(&viewer->tabBar);
    if (viewWidget->openUrl(url, mode)) {
        viewer->addTab(viewWidget);
        viewer->returnFocusTo = parent;
        viewer->returnFocusTab = viewWidget;
    } else {
        delete viewWidget;
        if(viewer->tabBar.count() <= 0) {
            viewer->close();
            if(!parent)
                parent = krMainWindow;
            parent->raise();
            parent->activateWindow();
        }
    }
}

void KrViewer::edit(KUrl url, QWidget * parent)
{
    edit(url, Text, -1, parent);
}

void KrViewer::edit(KUrl url, Mode mode, int new_window, QWidget * parent)
{
    KConfigGroup group(krConfig, "General");
    QString edit = group.readEntry("Editor", _Editor);

    if (new_window == -1)
        new_window = group.readEntry("View In Separate Window", _ViewInSeparateWindow);

    if (edit != "internal editor") {
        KProcess proc;
        // if the file is local, pass a normal path and not a url. this solves
        // the problem for editors that aren't url-aware
        proc << edit.split(' ') << url.pathOrUrl();
        if (!proc.startDetached())
            KMessageBox::sorry(krMainWindow, i18n("Can not open \"%1\"", edit));
        return ;
    }

    KrViewer* viewer = getViewer(new_window);
    viewer->returnFocusTo = parent;

    PanelViewerBase* editWidget = new PanelEditor(&viewer->tabBar);
    if (editWidget->openUrl(url, mode)) {
        viewer->addTab(editWidget);
        viewer->returnFocusTab = editWidget;
    } else {
        delete editWidget;
        if(viewer->tabBar.count() <= 0) {
            viewer->close();
            if(!parent)
                parent = krMainWindow;
            parent->raise();
            parent->activateWindow();
        }
    }
}

void KrViewer::addTab(PanelViewerBase *pvb)
{
    KParts::ReadOnlyPart *part = pvb->part();
    if (!part)
        return;

    manager.addPart(part, this);
    if (isValidPart(part))
        manager.setActivePart(part);

    tabBar.addTab(pvb, makeTabIcon(pvb), makeTabText(pvb));
    tabBar.setCurrentIndex(tabBar.indexOf(pvb));
    tabBar.setTabToolTip(tabBar.indexOf(pvb), makeTabToolTip(pvb));

    updateActions(pvb);

    // now we can offer the option to detach tabs (we have more than one)
    if (tabBar.count() > 1) {
        detachAction->setEnabled(true);
    }

    show();
    tabBar.show();

    connect(pvb, SIGNAL(urlChanged(PanelViewerBase *, const KUrl &)),
            this,  SLOT(tabURLChanged(PanelViewerBase *, const KUrl &)));

    if (part->widget())
        part->widget()->setFocus();
}

void KrViewer::tabURLChanged(PanelViewerBase *pvb, const KUrl & url)
{
    Q_UNUSED(url)
    refreshTab(pvb);
}

void KrViewer::tabChanged(QWidget* w)
{
    if (w == 0)
        return;

    if (isValidPart(static_cast<PanelViewerBase*>(w)->part()))
        manager.setActivePart(static_cast<PanelViewerBase*>(w)->part());

    if (static_cast<PanelViewerBase*>(w) != returnFocusTab) {
        returnFocusTo = 0;
        returnFocusTab = 0;
    }

    // set this viewer to be the main viewer
    if (viewers.removeAll(this)) viewers.prepend(this);      // move to first
}

void KrViewer::tabCloseRequest(QWidget *w)
{
    if (!w) return;

    // important to save as returnFocusTo will be cleared at removePart
    QWidget * returnFocusToThisWidget = returnFocusTo;

    PanelViewerBase* pvb = static_cast<PanelViewerBase*>(w);

    if (!pvb->queryClose())
        return;

    if (pvb->part() && isValidPart(pvb->part()))
        manager.removePart(pvb->part());

    pvb->closeUrl();

    tabBar.removePage(w);

    if (tabBar.count() <= 0) {
        if (returnFocusToThisWidget) {
            returnFocusToThisWidget->raise();
            returnFocusToThisWidget->activateWindow();
        } else {
            krMainWindow->raise();
            krMainWindow->activateWindow();
        }
        this->close();
        return;
    } else if (tabBar.count() == 1) {
        //no point in detaching only one tab..
        detachAction->setEnabled(false);
    }

    if (returnFocusToThisWidget) {
        returnFocusToThisWidget->raise();
        returnFocusToThisWidget->activateWindow();
    }
}

void KrViewer::tabCloseRequest()
{
    tabCloseRequest(tabBar.currentWidget());
}

bool KrViewer::queryClose()
{
    KConfigGroup group(krConfig, "KrViewerWindow");

    group.writeEntry("Window Width", sizeX);
    group.writeEntry("Window Height", sizeY);
    group.writeEntry("Window Maximized", isMaximized());

    for (int i = 0; i != tabBar.count(); i++) {
        PanelViewerBase* pvb = static_cast<PanelViewerBase*>(tabBar.widget(i));
        if (!pvb)
            continue;

        tabBar.setCurrentIndex(i);

        if (!pvb->queryClose())
            return false;
    }
    return true;
}

bool KrViewer::queryExit()
{
    return true; // don't let the reference counter reach zero
}

void KrViewer::viewGeneric()
{
    PanelViewerBase* pvb = static_cast<PanelViewerBase*>(tabBar.currentWidget());
    if (pvb) {
        PanelViewerBase* viewerWidget = new PanelViewer(&tabBar);
        if (viewerWidget->openUrl(pvb->url(), Generic))
            addTab(viewerWidget);
    }
}

void KrViewer::viewText()
{
    PanelViewerBase* pvb = static_cast<PanelViewerBase*>(tabBar.currentWidget());
    if (pvb) {
        PanelViewerBase* viewerWidget = new PanelViewer(&tabBar);
        if (viewerWidget->openUrl(pvb->url(), Text))
            addTab(viewerWidget);
    }
}

void KrViewer::viewLister()
{
    PanelViewerBase* pvb = static_cast<PanelViewerBase*>(tabBar.currentWidget());
    if (pvb) {
        PanelViewerBase* viewerWidget = new PanelViewer(&tabBar);
        if (viewerWidget->openUrl(pvb->url(), Lister))
            addTab(viewerWidget);
    }
}

void KrViewer::viewHex()
{
    PanelViewerBase* pvb = static_cast<PanelViewerBase*>(tabBar.currentWidget());
    if (pvb) {
        PanelViewerBase* viewerWidget = new PanelViewer(&tabBar);
        if (viewerWidget->openUrl(pvb->url(), Hex))
            addTab(viewerWidget);
    }
}

void KrViewer::editText()
{
    PanelViewerBase* pvb = static_cast<PanelViewerBase*>(tabBar.currentWidget());
    if (pvb) {
        PanelViewerBase* editWidget = new PanelEditor(&tabBar);
        if (editWidget->openUrl(pvb->url(), Text))
            addTab(editWidget);
    }
}

void KrViewer::checkModified()
{
    QTimer::singleShot(CHECK_MODFIED_INTERVAL, this, SLOT(checkModified()));

    PanelViewerBase* pvb = static_cast<PanelViewerBase*>(tabBar.currentWidget());
    if (pvb)
        refreshTab(pvb);
}

void KrViewer::refreshTab(PanelViewerBase* pvb)
{
    if (!pvb->part())
        return;

    //FIXME this belongs to PanelViewer
    if (!pvb->part()->url().equals(pvb->url(), KUrl::CompareWithoutTrailingSlash)) {
        pvb->setUrl(pvb->part()->url());
    }

    int ndx = tabBar.indexOf(pvb);
    tabBar.setTabText(ndx, makeTabText(pvb));
    tabBar.setTabIcon(ndx, makeTabIcon(pvb));
    tabBar.setTabToolTip(ndx, makeTabToolTip(pvb));
}

void KrViewer::nextTab()
{
    int index = (tabBar.currentIndex() + 1) % tabBar.count();
    tabBar.setCurrentIndex(index);
}

void KrViewer::prevTab()
{
    int index = (tabBar.currentIndex() - 1) % tabBar.count();
    while (index < 0) index += tabBar.count();
    tabBar.setCurrentIndex(index);
}

void KrViewer::detachTab()
{
    PanelViewerBase* pvb = static_cast<PanelViewerBase*>(tabBar.currentWidget());
    if (!pvb) return;

    KrViewer* viewer = getViewer(true);

    if (pvb->part() && isValidPart(pvb->part()))
        manager.removePart(pvb->part());
    tabBar.removePage(pvb);

    if (tabBar.count() == 1) {
        //no point in detaching only one tab..
        detachAction->setEnabled(false);
    }

    pvb->setParent(&viewer->tabBar);
    pvb->move(QPoint(0, 0));

    viewer->addTab(pvb);
}

void KrViewer::windowActivationChange(bool /* oldActive */)
{
    if (isActiveWindow())
        if (viewers.removeAll(this)) viewers.prepend(this);      // move to first
}

void KrViewer::print()
{
    PanelViewerBase* pvb = static_cast<PanelViewerBase*>(tabBar.currentWidget());
    if (!pvb || !pvb->part()) return;

    KParts::BrowserExtension * ext = KParts::BrowserExtension::childObject(pvb->part());
    if (ext && ext->isActionEnabled("print"))
        Invoker(ext, SLOT(print())).invoke();
}

void KrViewer::copy()
{
    PanelViewerBase* pvb = static_cast<PanelViewerBase*>(tabBar.currentWidget());
    if (!pvb || !pvb->part()) return;

    KParts::BrowserExtension * ext = KParts::BrowserExtension::childObject(pvb->part());
    if (ext && ext->isActionEnabled("copy"))
        Invoker(ext, SLOT(copy())).invoke();
}

PanelViewerBase * KrViewer::getPanelViewerBase(KParts::Part * part)
{
    for (int i = 0; i != tabBar.count(); i++) {
        PanelViewerBase *pvb = static_cast<PanelViewerBase*>(tabBar.widget(i));
        if (pvb && pvb->part() == part)
            return pvb;
    }
    return 0;
}

void KrViewer::updateActions(PanelViewerBase * /*pvb*/)
{
    QList<QAction *> actList = toolBar()->actions();
    bool hasPrint = false, hasCopy = false;
    foreach(QAction *a, actList) {
        if (a->text() == printAction->text())
            hasPrint = true;
        if (a->text() == copyAction->text())
            hasCopy = true;
    }
    if (!hasPrint)
        toolBar()->addAction(printAction->icon(), printAction->text(), this, SLOT(print()));
    if (!hasCopy)
        toolBar()->addAction(copyAction->icon(), copyAction->text(), this, SLOT(copy()));
}

bool KrViewer::isValidPart(KParts::Part* part)
{
    return manager.parts().contains(part);
}

void KrViewer::partDestroyed(PanelViewerBase * /*pvb*/)
{
    /* not yet used */
}

void KrViewer::resizeEvent(QResizeEvent *e)
{
    if (!isMaximized()) {
        sizeX = e->size().width();
        sizeY = e->size().height();
    }

    KParts::MainWindow::resizeEvent(e);
}

QString KrViewer::makeTabText(PanelViewerBase *pvb)
{
    QString fileName = pvb->url().fileName();
    if (pvb->isModified())
        fileName.prepend("*");

    return pvb->isEditor() ?
            i18nc("filename (filestate)", "%1 (Editing)", fileName) :
            i18nc("filename (filestate)", "%1 (Viewing)", fileName);
}

QString KrViewer::makeTabToolTip(PanelViewerBase *pvb)
{
    QString url = pvb->url().pathOrUrl();
    return pvb->isEditor() ?
            i18nc("filestate: filename", "Editing: %1", url) :
            i18nc("filestate: filename", "Viewing: %1", url);
}

QIcon KrViewer::makeTabIcon(PanelViewerBase *pvb)
{
    QString iconName;
    if (pvb->isModified())
        iconName = MODIFIED_ICON;
    else if (pvb->isEditor())
        iconName = EDIT_ICON;
    else
        iconName = VIEW_ICON;

    return QIcon(krLoader->loadIcon(iconName, KIconLoader::Small));
}

#include "krviewer.moc"
