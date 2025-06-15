/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krviewer.h"

// QtCore
#include <QDataStream>
#include <QDebug>
#include <QEvent>
#include <QFile>
#include <QTimer>
// QtGui
#include <QKeyEvent>
// QtWidgets
#include <QMenuBar>
#include <QStatusBar>
#include <QGuiApplication>

#include <KWindowSystem>
#include <kwaylandextras.h>
#include <kx11extras.h>
#include <KActionCollection>
#include <KFileItem>
#include <KLocalizedString>
#include <KMessageBox>
#include <KFileItem>
#include <KParts/Part>
#include <KProcess>
#include <KSharedConfig>
#include <KShell>
#include <KShortcutsDialog>
#include <KStandardAction>
#include <KToolBar>
#include <kxmlgui_version.h>
#include <utility>
#include <KParts/NavigationExtension>

#include "../defaults.h"
#include "../icon.h"
#include "panelviewer.h"
#include "viewertabbar.h"

#define VIEW_ICON "document-preview"
#define EDIT_ICON "document-edit"
#define MODIFIED_ICON "document-save-as"

#define CHECK_MODFIED_INTERVAL 500

/*
NOTE: Currently the code expects PanelViewer::openUrl() to be called only once
      in the panel viewer's life time - otherwise unexpected things might happen.
*/

QList<KrViewer *> KrViewer::viewers;

KrViewer::KrViewer(QWidget *parent)
    : KParts::MainWindow(parent, Qt::WindowFlags())
    , manager(this, this)
    , tabWidget(this)
    , sizeX(-1)
    , sizeY(-1)
{
    // setWFlags(Qt::WType_TopLevel | WDestructiveClose);
    setHelpMenuEnabled(false);

    connect(&manager, &KParts::PartManager::activePartChanged, this, &KrViewer::createGUI);
    connect(&tabWidget, &QTabWidget::currentChanged, this, &KrViewer::tabChanged);
    connect(&tabWidget, &QTabWidget::tabCloseRequested, this, [=](int index) {
        tabCloseRequest(index, false);
    });

    tabWidget.setDocumentMode(true);
    tabWidget.setMovable(true);
    if (ViewerTabBar *tabBar = tabWidget.tabBar())
        connect(tabBar, &ViewerTabBar::closeTabSignal, this, [=](int index) {
            tabCloseRequest(index, false);
        });
    setCentralWidget(&tabWidget);

    printAction = KStandardAction::print(this, SLOT(print()), nullptr);
    copyAction = KStandardAction::copy(this, SLOT(copy()), nullptr);

    viewerMenu = new QMenu(this);
    QAction *tempAction;
    KActionCollection *ac = actionCollection();

#define addCustomMenuAction(name, text, slot, shortcut)                                                                                                        \
    tempAction = ac->addAction(name, this, slot);                                                                                                              \
    tempAction->setText(text);                                                                                                                                 \
    ac->setDefaultShortcut(tempAction, shortcut);                                                                                                              \
    viewerMenu->addAction(tempAction);

    addCustomMenuAction("genericViewer", i18n("&Generic Viewer"), SLOT(viewGeneric()), Qt::CTRL | Qt::SHIFT | Qt::Key_G);
    addCustomMenuAction("textViewer", i18n("&Text Viewer"), SLOT(viewText()), Qt::CTRL | Qt::SHIFT | Qt::Key_T);
    addCustomMenuAction("hexViewer", i18n("&Hex Viewer"), SLOT(viewHex()), Qt::CTRL | Qt::SHIFT | Qt::Key_H);
    addCustomMenuAction("lister", i18n("&Lister"), SLOT(viewLister()), Qt::CTRL | Qt::SHIFT | Qt::Key_L);
    viewerMenu->addSeparator();

    addCustomMenuAction("textEditor", i18n("Text &Editor"), SLOT(editText()), Qt::CTRL | Qt::SHIFT | Qt::Key_E);
    viewerMenu->addSeparator();

    const QList<QAction *> actList = menuBar()->actions();
    bool hasPrint = false, hasCopy = false;
    for (QAction *a : actList) {
        if (a->shortcut().matches(printAction->shortcut()) != QKeySequence::NoMatch)
            hasPrint = true;
        if (a->shortcut().matches(copyAction->shortcut()) != QKeySequence::NoMatch)
            hasCopy = true;
    }
    QAction *printAct = viewerMenu->addAction(printAction->icon(), printAction->text(), this, SLOT(print()));
    if (hasPrint)
        printAct->setShortcut(printAction->shortcut());
    QAction *copyAct = viewerMenu->addAction(copyAction->icon(), copyAction->text(), this, SLOT(copy()));
    if (hasCopy)
        copyAct->setShortcut(copyAction->shortcut());
    viewerMenu->addSeparator();

    configKeysAction = ac->addAction(KStandardAction::KeyBindings, this, SLOT(configureShortcuts()));
    viewerMenu->addAction(configKeysAction);
    viewerMenu->addSeparator();

    detachAction = ac->addAction("detachTab", this, SLOT(detachTab()));
    detachAction->setText(i18n("&Detach Tab"));
    // no point in detaching only one tab..
    detachAction->setEnabled(false);
    ac->setDefaultShortcut(detachAction, Qt::META | Qt::Key_D);
    viewerMenu->addAction(detachAction);

    quitAction = ac->addAction(KStandardAction::Quit, this, SLOT(close()));
    viewerMenu->addAction(quitAction);

    QList<QKeySequence> shortcuts;

    tabCloseAction = ac->addAction("closeTab", this, SLOT(tabCloseRequest()));
    tabCloseAction->setText(i18n("&Close Current Tab"));
    shortcuts = KStandardShortcut::close();
    shortcuts.append(Qt::Key_Escape);
    ac->setDefaultShortcuts(tabCloseAction, shortcuts);

    tabNextAction = ac->addAction("nextTab", this, SLOT(nextTab()));
    tabNextAction->setText(i18n("&Next Tab"));
    shortcuts = KStandardShortcut::tabNext();
    shortcuts.append(Qt::CTRL | Qt::Key_Tab); // reenforce QTabWidget shortcut
    ac->setDefaultShortcuts(tabNextAction, shortcuts);

    tabPrevAction = ac->addAction("prevTab", this, SLOT(prevTab()));
    tabPrevAction->setText(i18n("&Previous Tab"));
    shortcuts = KStandardShortcut::tabPrev();
    shortcuts.append(Qt::CTRL | Qt::SHIFT | Qt::Key_Tab); // reenforce QTabWidget shortcut
    ac->setDefaultShortcuts(tabPrevAction, shortcuts);

    tabWidget.setTabsClosable(true);

    checkModified();

    KConfigGroup group(krConfig, "KrViewerWindow");
    int sx = group.readEntry("Window Width", -1);
    int sy = group.readEntry("Window Height", -1);

    if (sx != -1 && sy != -1)
        resize(sx, sy);
    else
        resize(900, 700);

    if (group.readEntry("Window Maximized", false)) {
        setWindowState(windowState() | Qt::WindowMaximized);
    }

    // filtering out the key events
    menuBar()->installEventFilter(this);

    setupGUI(ToolBar | StatusBar | Save | Create, "krviewer.rc");
}

KrViewer::~KrViewer()
{
    disconnect(&manager, SIGNAL(activePartChanged(KParts::Part *)), this, SLOT(createGUI(KParts::Part *)));

    viewers.removeAll(this);

    // close tabs before deleting tab bar - this avoids Qt bug 26115
    // https://bugreports.qt-project.org/browse/QTBUG-26115
    while (tabWidget.count())
        tabCloseRequest(tabWidget.currentIndex(), true);

    delete printAction;
    delete copyAction;
}

void KrViewer::configureDeps()
{
    PanelEditor::configureDeps();
}

void KrViewer::createGUI(KParts::Part *part)
{
    KParts::MainWindow::createGUI(part);

    updateActions();

    // the KParts part may override the viewer shortcuts. We prevent it
    // by installing an event filter on the menuBar() and the part
    reservedKeys.clear();
    reservedKeyActions.clear();

    QList<QAction *> list = viewerMenu->actions();
    // also add the actions that are not in the menu...
    list << tabCloseAction << tabNextAction << tabPrevAction;
    // getting the key sequences of the viewer menu
    for (int w = 0; w != list.count(); w++) {
        QAction *act = list[w];
        const QList<QKeySequence> sequences = act->shortcuts();
        for (QKeySequence keySeq : sequences) {
            for (int i = 0; i < keySeq.count(); ++i) {
                reservedKeys.push_back(keySeq[i]);
                reservedKeyActions.push_back(act); // the same action many times in case of multiple shortcuts
            }
        }
    }

    // and "fix" the menubar
    viewerMenu->setTitle(i18n("&KrViewer"));
    QAction *act = menuBar()->addMenu(viewerMenu);
    act->setData(QVariant(70));
    menuBar()->show();
}

void KrViewer::configureShortcuts()
{
#if KXMLGUI_VERSION >= QT_VERSION_CHECK(5, 84, 0)
    KShortcutsDialog::showDialog(actionCollection(), KShortcutsEditor::LetterShortcutsAllowed, this);
#else
    KShortcutsDialog::configure(actionCollection(), KShortcutsEditor::LetterShortcutsAllowed, this);
#endif
}

bool KrViewer::eventFilter(QObject * /* watched */, QEvent *e)
{
    // TODO: after porting to Qt5/KF5 we never catch *ANY* KeyPress or ShortcutOverride events here anymore.
    // Should look into if there is any way to fix it. Currently if a KPart has same shortcut as KrViewer then
    // it causes a conflict, messagebox shown to user and no action triggered.
    if (e->type() == QEvent::ShortcutOverride) {
        auto *ke = dynamic_cast<QKeyEvent *>(e);
        if (reservedKeys.contains(ke->key())) {
            ke->accept();

            QAction *act = reservedKeyActions[reservedKeys.indexOf(ke->key())];
            if (act != nullptr) {
                // don't activate the close functions immediately!
                // it can cause crash
                if (act == tabCloseAction || act == quitAction) {
                    QTimer::singleShot(0, act, &QAction::trigger);
                } else {
                    act->activate(QAction::Trigger);
                }
            }
            return true;
        }
    } else if (e->type() == QEvent::KeyPress) {
        auto *ke = dynamic_cast<QKeyEvent *>(e);
        if (reservedKeys.contains(ke->key())) {
            ke->accept();
            return true;
        }
    }
    return false;
}

void KrViewer::activateWindow(QWidget *window)
{
    auto focusWindow = qGuiApp->focusWindow();
    if (focusWindow && KrGlobal::isWaylandPlatform) {
        // Wayland(and KWin) doesn't allow for a window to come to foreground and
        // take focus away. The proper way to do this is for the window that has
        // focus to request a new activation token and pass it to the window that
        // will come to foreground(eg Viewer). The window to be brought to fore
        // will then call KWindowSystem::setCurrentXdgActivationToken which will
        // enable  KWindowSystem::activateWindow() to work. Without the call to
        // setCurrentXdgActivationToken, the call to
        // KWindowSystem::activateWindow() will just cause the window to blink in
        // the task bar requesting attention, but never get focus or come to the
        // foreground.
        const int launchedSerial = KWaylandExtras::lastInputSerial(focusWindow);
        auto conn = std::make_shared<QMetaObject::Connection>();
        *conn = connect(KWaylandExtras::self(),
                        &KWaylandExtras::xdgActivationTokenArrived,
                        window,
                        [window, launchedSerial, conn](int tokenSerial, const QString &token) {
                            if (tokenSerial == launchedSerial) {
                                disconnect(*conn);
                                KWindowSystem::setCurrentXdgActivationToken(token);
                                // activateWindow will only work if a new token from the focused window has been set otherwise it will only request attn
                                KWindowSystem::activateWindow(window->windowHandle());
                            }
                        });
        KWaylandExtras::requestXdgActivationToken(focusWindow, launchedSerial, {});
    }
    if (KrGlobal::isX11Platform) {
        KX11Extras::forceActiveWindow(window->winId());
    }
}
KrViewer *KrViewer::getViewer(bool new_window)
{
    if (!new_window) {
        if (viewers.isEmpty()) {
            viewers.prepend(new KrViewer()); // add to first (active)
        } else {
            if (viewers.first()->isMinimized()) { // minimized? -> show it again
                viewers.first()->setWindowState((viewers.first()->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);
                viewers.first()->show();
            }
            viewers.first()->raise();
            activateWindow(viewers.first());
        }
        return viewers.first();
    } else {
        auto *newViewer = new KrViewer();
        viewers.prepend(newViewer);
        return newViewer;
    }
}

void KrViewer::view(QUrl url, QWidget *parent)
{
    KConfigGroup group(krConfig, "General");
    bool defaultWindow = group.readEntry("View In Separate Window", _ViewInSeparateWindow);

    view(std::move(url), Default, defaultWindow, parent);
}

void KrViewer::view(QUrl url, Mode mode, bool new_window, QWidget *parent)
{
    KrViewer *viewer = getViewer(new_window);
    viewer->viewInternal(std::move(url), mode, parent);
    viewer->show();
}

void KrViewer::edit(QUrl url, QWidget *parent)
{
    edit(std::move(url), Text, -1, parent);
}

void KrViewer::edit(const QUrl &url, Mode mode, int new_window, QWidget *parent)
{
    KConfigGroup group(krConfig, "General");
    QString editor = group.readEntry("Editor", _Editor);

    if (new_window == -1)
        new_window = group.readEntry("View In Separate Window", _ViewInSeparateWindow);

    if (editor != "internal editor" && !editor.isEmpty()) {
        KProcess proc;
        QStringList cmdArgs = KShell::splitArgs(editor, KShell::TildeExpand);
        if (cmdArgs.isEmpty()) {
            KMessageBox::error(krMainWindow, i18nc("Arg is a string containing the bad quoting.", "Bad quoting in editor command:\n%1", editor));
            return;
        }
        // if the file is local, pass a normal path and not a url. this solves
        // the problem for editors that aren't url-aware
        proc << cmdArgs << url.toDisplayString(QUrl::PreferLocalFile);
        if (!proc.startDetached())
            KMessageBox::error(krMainWindow, i18n("Can not open \"%1\"", editor));
        return;
    }

    KrViewer *viewer = getViewer(new_window);
    viewer->editInternal(url, mode, parent);
    viewer->show();
}

void KrViewer::addTab(PanelViewerBase *pvb)
{
    int tabIndex = tabWidget.addTab(pvb, makeTabIcon(pvb), makeTabText(pvb));
    tabWidget.setCurrentIndex(tabIndex);
    tabWidget.setTabToolTip(tabIndex, makeTabToolTip(pvb));

    updateActions();

    // now we can offer the option to detach tabs (we have more than one)
    if (tabWidget.count() > 1)
        detachAction->setEnabled(true);
    tabWidget.adjustViewerTabBarVisibility();

    tabWidget.show();

    connect(pvb, &PanelViewerBase::openUrlFinished, this, &KrViewer::openUrlFinished);

    connect(pvb, &PanelViewerBase::urlChanged, this, &KrViewer::tabURLChanged);
}

void KrViewer::tabURLChanged(PanelViewerBase *pvb, const QUrl &url)
{
    Q_UNUSED(url)
    refreshTab(pvb);
}

void KrViewer::openUrlFinished(PanelViewerBase *pvb, bool success)
{
    if (success) {
        KParts::ReadOnlyPart *part = pvb->part();
        if (part) {
            if (!isPartAdded(part))
                addPart(part);
            if (tabWidget.currentWidget() == pvb) {
                manager.setActivePart(part);
                if (part->widget())
                    part->widget()->setFocus();
            }
        }
    } else {
        tabCloseRequest(tabWidget.currentIndex(), false);
    }
}

void KrViewer::tabChanged(int index)
{
    QWidget *w = tabWidget.widget(index);
    if (!w)
        return;
    KParts::ReadOnlyPart *part = dynamic_cast<PanelViewerBase *>(w)->part();
    if (part && isPartAdded(part)) {
        manager.setActivePart(part);
        if (part->widget())
            part->widget()->setFocus();
    } else
        manager.setActivePart(nullptr);

    // set this viewer to be the main viewer
    if (viewers.removeAll(this))
        viewers.prepend(this); // move to first
}

void KrViewer::tabCloseRequest(int index, bool force)
{
    // important to save as returnFocusTo will be cleared at removePart
    QWidget *returnFocusToThisWidget = returnFocusTo;

    auto *pvb = dynamic_cast<PanelViewerBase *>(tabWidget.widget(index));
    if (!pvb)
        return;

    if (!force && !pvb->queryClose())
        return;

    if (pvb->part() && isPartAdded(pvb->part()))
        removePart(pvb->part());

    disconnect(pvb, nullptr, this, nullptr);

    pvb->closeUrl();

    tabWidget.removeTab(index);

    delete pvb;
    pvb = nullptr;

    if (tabWidget.count() <= 0) {
        if (returnFocusToThisWidget) {
            returnFocusToThisWidget->raise();
            activateWindow(returnFocusToThisWidget);

        } else {
            krMainWindow->raise();
            activateWindow(krMainWindow);
        }

        QTimer::singleShot(0, this, &KrViewer::close);
    } else if (tabWidget.count() == 1) {
        // no point in detaching only one tab..
        detachAction->setEnabled(false);
        tabWidget.adjustViewerTabBarVisibility();
    }
}

void KrViewer::tabCloseRequest()
{
    tabCloseRequest(tabWidget.currentIndex());
}

bool KrViewer::queryClose()
{
    KConfigGroup group(krConfig, "KrViewerWindow");

    group.writeEntry("Window Width", sizeX);
    group.writeEntry("Window Height", sizeY);
    group.writeEntry("Window Maximized", isMaximized());

    for (int i = 0; i != tabWidget.count(); i++) {
        auto *pvb = dynamic_cast<PanelViewerBase *>(tabWidget.widget(i));
        if (!pvb)
            continue;

        tabWidget.setCurrentIndex(i);

        if (!pvb->queryClose())
            return false;
    }
    return true;
}

void KrViewer::viewGeneric()
{
    auto *pvb = dynamic_cast<PanelViewerBase *>(tabWidget.currentWidget());
    if (pvb)
        viewInternal(pvb->url(), Generic);
}

void KrViewer::viewText()
{
    auto *pvb = dynamic_cast<PanelViewerBase *>(tabWidget.currentWidget());
    if (pvb)
        viewInternal(pvb->url(), Text);
}

void KrViewer::viewLister()
{
    auto *pvb = dynamic_cast<PanelViewerBase *>(tabWidget.currentWidget());
    if (pvb)
        viewInternal(pvb->url(), Lister);
}

void KrViewer::viewHex()
{
    auto *pvb = dynamic_cast<PanelViewerBase *>(tabWidget.currentWidget());
    if (pvb)
        viewInternal(pvb->url(), Hex);
}

void KrViewer::editText()
{
    auto *pvb = dynamic_cast<PanelViewerBase *>(tabWidget.currentWidget());
    if (pvb)
        editInternal(pvb->url(), Text);
}

void KrViewer::checkModified()
{
    QTimer::singleShot(CHECK_MODFIED_INTERVAL, this, &KrViewer::checkModified);

    auto *pvb = dynamic_cast<PanelViewerBase *>(tabWidget.currentWidget());
    if (pvb)
        refreshTab(pvb);
}

void KrViewer::refreshTab(PanelViewerBase *pvb)
{
    int ndx = tabWidget.indexOf(pvb);
    tabWidget.setTabText(ndx, makeTabText(pvb));
    tabWidget.setTabIcon(ndx, makeTabIcon(pvb));
    tabWidget.setTabToolTip(ndx, makeTabToolTip(pvb));
}

void KrViewer::nextTab()
{
    int index = (tabWidget.currentIndex() + 1) % tabWidget.count();
    tabWidget.setCurrentIndex(index);
}

void KrViewer::prevTab()
{
    int index = (tabWidget.currentIndex() - 1) % tabWidget.count();
    while (index < 0)
        index += tabWidget.count();
    tabWidget.setCurrentIndex(index);
}

void KrViewer::detachTab()
{
    auto *pvb = dynamic_cast<PanelViewerBase *>(tabWidget.currentWidget());
    if (!pvb)
        return;

    KrViewer *viewer = getViewer(true);

    bool wasPartAdded = false;
    KParts::ReadOnlyPart *part = pvb->part();

    if (part && isPartAdded(part)) {
        wasPartAdded = true;
        removePart(part);
    }

    disconnect(pvb, nullptr, this, nullptr);

    tabWidget.removeTab(tabWidget.indexOf(pvb));

    if (tabWidget.count() == 1) {
        // no point in detaching only one tab..
        detachAction->setEnabled(false);
        tabWidget.adjustViewerTabBarVisibility();
    }

    pvb->setParent(&viewer->tabWidget);
    pvb->move(QPoint(0, 0));

    viewer->addTab(pvb);

    if (wasPartAdded) {
        viewer->addPart(part);
        if (part->widget())
            part->widget()->setFocus();
    }

    viewer->show();
}

void KrViewer::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::ActivationChange && isActiveWindow())
        if (viewers.removeAll(this))
            viewers.prepend(this); // move to first
}

void KrViewer::print()
{
    auto *pvb = dynamic_cast<PanelViewerBase *>(tabWidget.currentWidget());
    if (!pvb || !pvb->part() || !isPartAdded(pvb->part()))
        return;

    KParts::NavigationExtension *ext = KParts::NavigationExtension::childObject(pvb->part());
    if (ext && ext->isActionEnabled("print"))
        Invoker(ext, SLOT(print())).invoke();
}

void KrViewer::copy()
{
    auto *pvb = dynamic_cast<PanelViewerBase *>(tabWidget.currentWidget());
    if (!pvb || !pvb->part() || !isPartAdded(pvb->part()))
        return;

    KParts::NavigationExtension *ext = KParts::NavigationExtension::childObject(pvb->part());
    if (ext && ext->isActionEnabled("copy"))
        Invoker(ext, SLOT(copy())).invoke();
}

void KrViewer::updateActions()
{
    const QList<QAction *> actList = toolBar()->actions();
    bool hasPrint = false, hasCopy = false;
    for (QAction *a : actList) {
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

bool KrViewer::isPartAdded(KParts::Part *part)
{
    return manager.parts().contains(part);
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

    return pvb->isEditor() ? // clang-format off
        i18nc("filename (filestate)", "%1 (Editing)", fileName) :
        i18nc("filename (filestate)", "%1 (Viewing)", fileName); // clang-format on
}

QString KrViewer::makeTabToolTip(PanelViewerBase *pvb)
{
    QString url = pvb->url().toDisplayString(QUrl::PreferLocalFile);
    return pvb->isEditor() ? i18nc("filestate: filename", "Editing: %1", url) : i18nc("filestate: filename", "Viewing: %1", url);
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

    return Icon(iconName);
}

void KrViewer::addPart(KParts::ReadOnlyPart *part)
{
    Q_ASSERT(part);
    Q_ASSERT(!isPartAdded(part));

    if (isPartAdded(part)) {
        qDebug() << "part already added:" << part;
        return;
    }

    connect(part, SIGNAL(setStatusBarText(QString)), this, SLOT(slotSetStatusBarText(QString)));
    // filtering out the key events
    part->installEventFilter(this);

    manager.addPart(part, false); // don't automatically set active part
}

void KrViewer::removePart(KParts::ReadOnlyPart *part)
{
    Q_ASSERT(part);
    Q_ASSERT(isPartAdded(part));

    if (isPartAdded(part)) {
        disconnect(part, nullptr, this, nullptr);
        part->removeEventFilter(this);
        manager.removePart(part);
    } else
        qDebug() << "part hasn't been added:" << part;
}

void KrViewer::viewInternal(QUrl url, Mode mode, QWidget *parent)
{
    returnFocusTo = parent;

    PanelViewerBase *viewWidget = new PanelViewer(&tabWidget, mode);

    addTab(viewWidget);
    viewWidget->openUrl(std::move(url));
}

void KrViewer::editInternal(QUrl url, Mode mode, QWidget *parent)
{
    returnFocusTo = parent;

    PanelViewerBase *editWidget = new PanelEditor(&tabWidget, mode);

    addTab(editWidget);
    editWidget->openUrl(std::move(url));
}
