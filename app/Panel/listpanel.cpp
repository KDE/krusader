/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "listpanel.h"

// QtCore
#include <QEvent>
#include <QList>
#include <QMimeData>
#include <QRegExp>
#include <QStringList>
#include <QTimer>
#include <QUrl>
// QtGui
#include <QBitmap>
#include <QDrag>
#include <QDropEvent>
#include <QHideEvent>
#include <QImage>
#include <QKeyEvent>
#include <QPixmap>
#include <QShowEvent>
// QtWidgets
#include <QFrame>
#include <QHBoxLayout>
#include <QMenu>
#include <QSplitter>
#include <QTabBar>

#include <KCursor>
#include <KFilePlacesModel>
#include <KIO/DropJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KUrlComboBox>
#include <KUrlMimeData>
#include <utility>

#include "dirhistoryqueue.h"
#include "krcolorcache.h"
#include "krerrordisplay.h"
#include "krlayoutfactory.h"
#include "krpreviewpopup.h"
#include "krsearchbar.h"
#include "listpanelactions.h"
#include "panelfunc.h"
#include "sidebar.h"
#include "viewactions.h"

#include "PanelView/krview.h"
#include "PanelView/krviewfactory.h"
#include "PanelView/krviewitem.h"

#include "../defaults.h"
#include "../icon.h"
#include "../krservices.h"
#include "../krslots.h"
#include "../krusader.h"
#include "../krusaderview.h"

#include "../Archive/krarchandler.h"
#include "../BookMan/krbookmarkbutton.h"
#include "../Dialogs/krdialogs.h"
#include "../Dialogs/krspwidgets.h"
#include "../Dialogs/krsqueezedtextlabel.h"
#include "../Dialogs/percentalsplitter.h"
#include "../Dialogs/popularurls.h"
#include "../FileSystem/fileitem.h"
#include "../FileSystem/filesystem.h"
#include "../FileSystem/krpermhandler.h"
#include "../FileSystem/sizecalculator.h"
#include "../GUI/dirhistorybutton.h"
#include "../GUI/kcmdline.h"
#include "../GUI/mediabutton.h"
#include "../GUI/terminaldock.h"
#include "../MountMan/kmountman.h"
#include "../UserAction/useractionpopupmenu.h"

class ActionButton : public QToolButton
{
public:
    ActionButton(QWidget *parent, ListPanel *panel, QAction *action, const QString &text = QString())
        : QToolButton(parent)
        , panel(panel)
        , action(action)
    {
        setText(text);
        setAutoRaise(true);
        if (KConfigGroup(krConfig, "ListPanelButtons").readEntry("Icons", false) || text.isEmpty())
            setIcon(action->icon());
        setToolTip(action->toolTip());
    }

protected:
    void mousePressEvent(QMouseEvent *) override
    {
        panel->slotFocusOnMe();
        action->trigger();
    }

    ListPanel *panel;
    QAction *action;
};

/////////////////////////////////////////////////////
//      The list panel constructor       //
/////////////////////////////////////////////////////
ListPanel::ListPanel(QWidget *parent, AbstractPanelManager *manager, const KConfigGroup &cfg)
    : QWidget(parent)
    , KrPanel(manager, this, new ListPanelFunc(this))
    , panelType(-1)
    , colorMask(255)
    , compareMode(false)
    , previewJob(nullptr)
    , inlineRefreshJob(nullptr)
    , searchBar(nullptr)
    , cdRootButton(nullptr)
    , cdUpButton(nullptr)
    , sidebarButton(nullptr)
    , sidebar(nullptr)
    , fileSystemError(nullptr)
    , _tabState(TabState::DEFAULT)
{
    if (cfg.isValid())
        panelType = cfg.readEntry("Type", -1);
    if (panelType == -1)
        panelType = defaultPanelType();

    _actions = krApp->listPanelActions();

    setAcceptDrops(true);

    QHash<QString, QWidget *> widgets;

#define ADD_WIDGET(widget) widgets.insert(#widget, widget);

    // media button
    mediaButton = new MediaButton(this);
    connect(mediaButton, &MediaButton::aboutToShow, this, [=]() {
        slotFocusOnMe();
    });
    connect(mediaButton, &MediaButton::openUrl, [=](const QUrl &_t1) {
        func->openUrl(_t1);
    });
    connect(mediaButton, &MediaButton::newTab, this, [=](const QUrl &url) {
        duplicateTab(url);
    });
    ADD_WIDGET(mediaButton);

    // status bar
    status = new KrSqueezedTextLabel(this);
    KConfigGroup group(krConfig, "Look&Feel");
    status->setFont(group.readEntry("Filelist Font", _FilelistFont));
    status->setAutoFillBackground(false);
    status->setText(""); // needed for initialization code!
    status->setWhatsThis(
        i18n("The statusbar displays information about the filesystem "
             "which holds your current folder: total size, free space, "
             "type of filesystem, etc."));
    ADD_WIDGET(status);

    // back button
    backButton = new ActionButton(this, this, _actions->actHistoryBackward);
    ADD_WIDGET(backButton);

    // forward button
    forwardButton = new ActionButton(this, this, _actions->actHistoryForward);
    ADD_WIDGET(forwardButton);

    // ... create the history button
    historyButton = new DirHistoryButton(func->history, this);
    connect(historyButton, &DirHistoryButton::aboutToShow, this, [=]() {
        slotFocusOnMe();
    });
    connect(historyButton, &DirHistoryButton::gotoPos, func, &ListPanelFunc::historyGotoPos);
    ADD_WIDGET(historyButton);

    // bookmarks button
    bookmarksButton = new KrBookmarkButton(this);
    connect(bookmarksButton, &KrBookmarkButton::aboutToShow, this, [=]() {
        slotFocusOnMe();
    });
    connect(bookmarksButton, &KrBookmarkButton::openUrl, this, [this](const QUrl &_t1) {
        func->openUrl(_t1);
    });
    bookmarksButton->setWhatsThis(
        i18n("Open menu with bookmarks. You can also add "
             "current location to the list, edit bookmarks "
             "or add subfolder to the list."));
    ADD_WIDGET(bookmarksButton);

    // url input field
    urlNavigator = new KUrlNavigator(new KFilePlacesModel(this), QUrl(), this);
    urlNavigator->setWhatsThis(
        i18n("Name of folder where you are. You can also "
             "enter name of desired location to move there. "
             "Use of Net protocols like ftp or fish is possible."));
    // handle certain key events here in event filter
    urlNavigator->editor()->installEventFilter(this);
    urlNavigator->setUrlEditable(isNavigatorEditModeSet());
    urlNavigator->setShowFullPath(group.readEntry("Navigator Full Path", false));
    connect(urlNavigator, &KUrlNavigator::returnPressed, this, [=]() {
        slotFocusOnMe();
    });
    connect(urlNavigator, &KUrlNavigator::urlChanged, this, &ListPanel::slotNavigatorUrlChanged);
    connect(urlNavigator->editor()->lineEdit(), &QLineEdit::editingFinished, this, &ListPanel::resetNavigatorMode);
    connect(urlNavigator, &KUrlNavigator::tabRequested, this, [=](const QUrl &url) {
        ListPanel::duplicateTab(url);
    });
    connect(urlNavigator, &KUrlNavigator::urlsDropped, this, QOverload<const QUrl &, QDropEvent *>::of(&ListPanel::handleDrop));
    ADD_WIDGET(urlNavigator);

    // toolbar
    QWidget *toolbar = new QWidget(this);
    auto *toolbarLayout = new QHBoxLayout(toolbar);
    toolbarLayout->setContentsMargins(0, 0, 0, 0);
    toolbarLayout->setSpacing(0);
    ADD_WIDGET(toolbar);

    fileSystemError = new KrErrorDisplay(this);
    fileSystemError->setWordWrap(true);
    fileSystemError->hide();
    ADD_WIDGET(fileSystemError);

    // client area
    clientArea = new QWidget(this);
    auto *clientLayout = new QVBoxLayout(clientArea);
    clientLayout->setSpacing(0);
    clientLayout->setContentsMargins(0, 0, 0, 0);
    ADD_WIDGET(clientArea);

    // totals label
    totals = new KrSqueezedTextLabel(this);
    totals->setFont(group.readEntry("Filelist Font", _FilelistFont));
    totals->setAutoFillBackground(false);
    totals->setWhatsThis(
        i18n("The totals bar shows how many files exist, "
             "how many selected and the bytes math"));
    ADD_WIDGET(totals);

    // free space label
    freeSpace = new KrSqueezedTextLabel(this);
    freeSpace->setFont(group.readEntry("Filelist Font", _FilelistFont));
    freeSpace->setAutoFillBackground(false);
    freeSpace->setText("");
    freeSpace->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ADD_WIDGET(freeSpace);

    // progress indicator and cancel button for the quick calc size
    quickSizeCalcProgress = new QProgressBar(this);
    quickSizeCalcProgress->hide();
    ADD_WIDGET(quickSizeCalcProgress);
    cancelQuickSizeCalcButton = new QToolButton(this);
    cancelQuickSizeCalcButton->hide();
    cancelQuickSizeCalcButton->setIcon(Icon("dialog-cancel"));
    cancelQuickSizeCalcButton->setToolTip(i18n("Cancel folder space calculation"));
    ADD_WIDGET(cancelQuickSizeCalcButton);

    // make sure to reserve enough vertical space for the progress indicator to become visible
    quickSizeCalcProgress->adjustSize();
    cancelQuickSizeCalcButton->adjustSize();
    totals->setMinimumHeight(std::max(quickSizeCalcProgress->height(), cancelQuickSizeCalcButton->height()));

    // progress indicator for the preview job
    previewProgress = new QProgressBar(this);
    previewProgress->hide();
    ADD_WIDGET(previewProgress);

    // a cancel button for the filesystem refresh and preview job
    cancelProgressButton = new QToolButton(this);
    cancelProgressButton->hide();
    cancelProgressButton->setIcon(Icon("dialog-cancel"));
    connect(cancelProgressButton, &QToolButton::clicked, this, &ListPanel::cancelProgress);
    ADD_WIDGET(cancelProgressButton);

    // button for changing the panel sidebar position in the panel
    sidebarPositionButton = new QToolButton(this);
    sidebarPositionButton->hide();
    sidebarPositionButton->setAutoRaise(true);
    sidebarPositionButton->setIcon(Icon("exchange-positions"));
    sidebarPositionButton->setToolTip(i18n("Move Sidebar clockwise"));
    connect(sidebarPositionButton, &QToolButton::clicked, this, [this]() {
        // moving position clockwise
        setSidebarPosition((sidebarPosition() + 1) % 4);
    });
    ADD_WIDGET(sidebarPositionButton);

    // a quick button to open the sidebar
    sidebarButton = new QToolButton(this);
    sidebarButton->setAutoRaise(true);
    sidebarButton->setIcon(Icon("arrow-up"));
    connect(sidebarButton, &QToolButton::clicked, this, &ListPanel::toggleSidebar);
    sidebarButton->setToolTip(i18n("Open the Sidebar"));
    ADD_WIDGET(sidebarButton);

#undef ADD_WIDGET

    // toolbar buttons
    cdOtherButton = new ActionButton(toolbar, this, _actions->actCdToOther, "=");
    toolbarLayout->addWidget(cdOtherButton);

    cdUpButton = new ActionButton(toolbar, this, _actions->actDirUp, "..");
    toolbarLayout->addWidget(cdUpButton);

    cdHomeButton = new ActionButton(toolbar, this, _actions->actHome, "~");
    toolbarLayout->addWidget(cdHomeButton);

    cdRootButton = new ActionButton(toolbar, this, _actions->actRoot, "/");
    toolbarLayout->addWidget(cdRootButton);

    // create the button for sync-browsing
    syncBrowseButton = new QToolButton(toolbar);
    syncBrowseButton->setIcon(Icon("kr_syncbrowse_off"));
    syncBrowseButton->setCheckable(true);

    const QString syncBrowseText = i18n(
        "This button toggles the sync-browse mode.\n"
        "When active, each folder change is performed in the\n"
        "active and inactive panel - if possible.");
    syncBrowseButton->setText(syncBrowseText);
    syncBrowseButton->setToolTip(syncBrowseText);
    connect(syncBrowseButton, &QToolButton::toggled, syncBrowseButton, [this](bool checked) {
        syncBrowseButton->setIcon(Icon(checked ? "kr_syncbrowse_on" : "kr_syncbrowse_off"));
    });
    syncBrowseButton->setAutoRaise(true);
    toolbarLayout->addWidget(syncBrowseButton);

    setButtons();

    // create a splitter to hold the view and the sidebar
    sidebarSplitter = new PercentalSplitter(clientArea);
    sidebarSplitter->setChildrenCollapsible(true);
    sidebarSplitter->setOrientation(Qt::Horizontal);
    // expand vertical if splitter orientation is horizontal
    QSizePolicy sizePolicy = sidebarSplitter->sizePolicy();
    sizePolicy.setVerticalPolicy(QSizePolicy::Expanding);
    sidebarSplitter->setSizePolicy(sizePolicy);
    clientLayout->addWidget(sidebarSplitter);

    // view
    createView();

    // search (in folder) bar
    searchBar = new KrSearchBar(view, clientArea);
    searchBar->hide();
    bool top = group.readEntry("Quicksearch Position", "bottom") == "top";
    clientLayout->insertWidget(top ? 0 : -1, searchBar);

    // create the layout
    KrLayoutFactory fact(this, widgets);
    QLayout *layout = fact.createLayout();

    if (!layout) { // fallback: create a layout by ourself
        auto *v = new QVBoxLayout;
        v->setContentsMargins(0, 0, 0, 0);
        v->setSpacing(0);

        auto *h = new QHBoxLayout;
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(0);
        h->addWidget(urlNavigator);
        h->addWidget(toolbar);
        h->addStretch();
        v->addLayout(h);

        h = new QHBoxLayout;
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(0);
        h->addWidget(mediaButton);
        h->addWidget(status);
        h->addWidget(backButton);
        h->addWidget(forwardButton);
        h->addWidget(historyButton);
        h->addWidget(bookmarksButton);
        v->addLayout(h);

        v->addWidget(fileSystemError);
        v->addWidget(clientArea);

        h = new QHBoxLayout;
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(0);
        h->addWidget(totals);
        h->addWidget(freeSpace);
        h->addWidget(quickSizeCalcProgress);
        h->addWidget(cancelQuickSizeCalcButton);
        h->addWidget(previewProgress);
        h->addWidget(cancelProgressButton);
        h->addWidget(sidebarButton);
        v->addLayout(h);

        layout = v;
    }

    setLayout(layout);

    connect(&KrColorCache::getColorCache(), &KrColorCache::colorsRefreshed, this, &ListPanel::slotRefreshColors);
    connect(krApp, &Krusader::shutdown, this, &ListPanel::cancelProgress);
}

ListPanel::~ListPanel()
{
    view->widget()->removeEventFilter(this);
    urlNavigator->editor()->removeEventFilter(this);
    cancelProgress();
    delete view;
    view = nullptr;
    delete func;
    delete status;
    delete bookmarksButton;
    delete totals;
    delete urlNavigator;
    delete cdRootButton;
    delete cdHomeButton;
    delete cdUpButton;
    delete cdOtherButton;
    delete syncBrowseButton;
    //     delete layout;
}

void ListPanel::reparent(QWidget *parent, AbstractPanelManager *manager)
{
    setParent(parent);
    _manager = manager;
}

int ListPanel::defaultPanelType()
{
    KConfigGroup group(krConfig, "Look&Feel");
    return group.readEntry("Default Panel Type", KrViewFactory::defaultViewId());
}

bool ListPanel::isNavigatorEditModeSet()
{
    KConfigGroup group(krConfig, "Look&Feel");
    return group.readEntry("Navigator Edit Mode", false);
}

void ListPanel::createView()
{
    view = KrViewFactory::createView(panelType, sidebarSplitter, krConfig);
    view->init();
    view->setMainWindow(krApp);

    // KrViewFactory may create a different view type than requested
    panelType = view->instance()->id();

    if (this == ACTIVE_PANEL)
        view->prepareForActive();
    else
        view->prepareForPassive();
    view->refreshColors();

    sidebarSplitter->insertWidget(sidebarPosition() < 2 ? 1 : 0, view->widget());

    view->widget()->installEventFilter(this);

    connect(view->op(), &KrViewOperator::quickCalcSpace, func, &ListPanelFunc::quickCalcSpace);
    connect(view->op(), &KrViewOperator::goHome, func, &ListPanelFunc::home);
    connect(view->op(), &KrViewOperator::dirUp, func, &ListPanelFunc::dirUp);
    connect(view->op(), &KrViewOperator::defaultDeleteFiles, func, &ListPanelFunc::defaultDeleteFiles);
    connect(view->op(), &KrViewOperator::middleButtonClicked, this, QOverload<KrViewItem *>::of(&ListPanel::duplicateTab));
    connect(view->op(), &KrViewOperator::currentChanged, this, &ListPanel::slotCurrentChanged);
    connect(view->op(), &KrViewOperator::renameItem, func, QOverload<const QString &, const QString &>::of(&ListPanelFunc::rename));
    connect(view->op(), &KrViewOperator::executed, func, &ListPanelFunc::execute);
    connect(view->op(), &KrViewOperator::goInside, func, &ListPanelFunc::goInside);
    connect(view->op(), &KrViewOperator::needFocus, this, [=]() {
        slotFocusOnMe();
    });
    connect(view->op(), &KrViewOperator::selectionChanged, this, &ListPanel::slotUpdateTotals);
    connect(view->op(), &KrViewOperator::itemDescription, krApp, &Krusader::statusBarUpdate);
    connect(view->op(), &KrViewOperator::contextMenu, this, &ListPanel::popRightClickMenu);
    connect(view->op(), &KrViewOperator::emptyContextMenu, this, &ListPanel::popEmptyRightClickMenu);
    connect(view->op(), &KrViewOperator::letsDrag, this, &ListPanel::startDragging);
    connect(view->op(), &KrViewOperator::gotDrop, this, [this](QDropEvent *event) {
        handleDrop(event);
    });
    connect(view->op(), &KrViewOperator::previewJobStarted, this, &ListPanel::slotPreviewJobStarted);
    connect(view->op(), &KrViewOperator::refreshActions, krApp->viewActions(), &ViewActions::refreshActions);
    connect(view->op(), &KrViewOperator::currentChanged, func->history, &DirHistoryQueue::saveCurrentItem);
    connect(view->op(), &KrViewOperator::goBack, func, &ListPanelFunc::historyBackward);
    connect(view->op(), &KrViewOperator::goForward, func, &ListPanelFunc::historyForward);

    view->setFiles(func->files());

    func->refreshActions();
}

void ListPanel::changeType(int type)
{
    if (panelType != type) {
        QString current = view->getCurrentItem();
        QList<QUrl> selection = view->selectedUrls();
        bool filterApplysToDirs = view->properties()->filterApplysToDirs;
        KrViewProperties::FilterSpec filter = view->filter();
        FilterSettings filterSettings = view->properties()->filterSettings;

        panelType = type;

        KrView *oldView = view;
        createView();
        searchBar->setView(view);
        delete oldView;

        view->setFilter(filter, filterSettings, filterApplysToDirs);
        view->setSelectionUrls(selection);
        view->setCurrentItem(current);
        view->makeItemVisible(view->getCurrentKrViewItem());
    }
}

int ListPanel::getProperties()
{
    int props = 0;
    if (syncBrowseButton->isChecked()) {
        props |= PROP_SYNC_BUTTON_ON;
    }
    if (isLocked()) {
        props |= PROP_LOCKED;
    } else if (isPinned()) {
        props |= PROP_PINNED;
    }
    return props;
}

void ListPanel::setProperties(int prop)
{
    syncBrowseButton->setChecked(prop & PROP_SYNC_BUTTON_ON);
    if (prop & PROP_LOCKED) {
        _tabState = TabState::LOCKED;
    } else if (prop & PROP_PINNED) {
        _tabState = TabState::PINNED;
    } else {
        _tabState = TabState::DEFAULT;
    }
}

bool ListPanel::eventFilter(QObject *watched, QEvent *e)
{
    if (view && watched == view->widget()) {
        if (e->type() == QEvent::FocusIn && this != ACTIVE_PANEL && !isHidden())
            slotFocusOnMe();
        else if (e->type() == QEvent::ShortcutOverride) {
            auto *ke = dynamic_cast<QKeyEvent *>(e);
            if (ke->key() == Qt::Key_Escape && ke->modifiers() == Qt::NoModifier) {
                // if the cancel refresh action has no shortcut assigned,
                // we need this event ourselves to cancel refresh
                if (_actions->actCancelRefresh->shortcut().isEmpty()) {
                    e->accept();
                    return true;
                }
            }
        }
    }
    // handle URL navigator key events
    else if (urlNavigator && watched == urlNavigator->editor()) {
        // override default shortcut for panel focus
        if (e->type() == QEvent::ShortcutOverride) {
            auto *ke = dynamic_cast<QKeyEvent *>(e);
            if ((ke->key() == Qt::Key_Escape) && (ke->modifiers() == Qt::NoModifier)) {
                e->accept(); // we will get the key press event now
                return true;
            }
        } else if (e->type() == QEvent::KeyPress) {
            auto *ke = dynamic_cast<QKeyEvent *>(e);
            if ((ke->key() == Qt::Key_Down) && (ke->modifiers() == Qt::ControlModifier)) {
                slotFocusOnMe();
                return true;
            } else if ((ke->key() == Qt::Key_Escape) && (ke->modifiers() == Qt::NoModifier)) {
                // reset navigator
                urlNavigator->editor()->setUrl(urlNavigator->locationUrl());
                slotFocusOnMe();
                return true;
            }
        }
    }
    return false;
}

void ListPanel::toggleSidebar()
{
    if (!sidebar) {
        sidebar = new Sidebar(sidebarSplitter);
        // fix vertical grow of splitter (and entire window) if its content
        // demands more space
        QSizePolicy sizePolicy = sidebar->sizePolicy();
        sizePolicy.setVerticalPolicy(QSizePolicy::Ignored);
        sidebar->setSizePolicy(sizePolicy);
        connect(this, &ListPanel::pathChanged, sidebar, &Sidebar::onPanelPathChange);
        connect(sidebar, &Sidebar::urlActivated, SLOTS, &KrSlots::refresh);
        sidebarSplitter->insertWidget(0, sidebar);
    }

    if (sidebar->isHidden()) {
        if (sidebarSplitterSizes.count() > 0) {
            sidebarSplitter->setSizes(sidebarSplitterSizes);
        } else { // on the first time, resize to 50%
            QList<int> lst;
            lst << height() / 2 << height() / 2;
            sidebarSplitter->setSizes(lst);
        }

        sidebar->show();
        sidebarButton->setIcon(Icon("arrow-down"));
        sidebarButton->setToolTip(i18n("Close the Sidebar"));
        sidebarPositionButton->show();
    } else {
        sidebarSplitterSizes.clear();
        sidebarSplitterSizes = sidebarSplitter->sizes();
        sidebar->hide();
        sidebarButton->setIcon(Icon("arrow-up"));
        sidebarButton->setToolTip(i18n("Open the Sidebar"));
        sidebarPositionButton->hide();

        QList<int> lst;
        lst << height() << 0;
        sidebarSplitter->setSizes(lst);
        if (ACTIVE_PANEL)
            ACTIVE_PANEL->gui->slotFocusOnMe();
    }
}

QString ListPanel::lastLocalPath() const
{
    return _lastLocalPath;
}

void ListPanel::setButtons()
{
    KConfigGroup group(krConfig, "Look&Feel");

    mediaButton->setVisible(group.readEntry("Media Button Visible", true));
    backButton->setVisible(group.readEntry("Back Button Visible", false));
    forwardButton->setVisible(group.readEntry("Forward Button Visible", false));
    historyButton->setVisible(group.readEntry("History Button Visible", true));
    bookmarksButton->setVisible(group.readEntry("Bookmarks Button Visible", true));

    if (group.readEntry("Panel Toolbar visible", _PanelToolBar)) {
        cdRootButton->setVisible(group.readEntry("Root Button Visible", _cdRoot));
        cdHomeButton->setVisible(group.readEntry("Home Button Visible", _cdHome));
        cdUpButton->setVisible(group.readEntry("Up Button Visible", _cdUp));
        cdOtherButton->setVisible(group.readEntry("Equal Button Visible", _cdOther));
        syncBrowseButton->setVisible(group.readEntry("SyncBrowse Button Visible", _syncBrowseButton));
    } else {
        cdRootButton->hide();
        cdHomeButton->hide();
        cdUpButton->hide();
        cdOtherButton->hide();
        syncBrowseButton->hide();
    }
}

void ListPanel::slotUpdateTotals()
{
    totals->setText(view->statistics());
}

void ListPanel::compareDirs(bool otherPanelToo)
{
    // Performs a check in order to avoid that the next code is executed twice
    if (otherPanelToo == true) {
        // If both panels are showing the same directory
        if (_manager->currentPanel()->virtualPath() == otherPanel()->virtualPath()) {
            if (KMessageBox::warningContinueCancel(this, i18n("Warning: The left and the right side are showing the same folder.")) != KMessageBox::Continue) {
                return;
            }
        }
    }

    KConfigGroup pg(krConfig, "Private");
    int compareMode = pg.readEntry("Compare Mode", 0);
    KConfigGroup group(krConfig, "Look&Feel");
    bool selectDirs = group.readEntry("Mark Dirs", false);

    KrViewItem *item, *otherItem;

    for (item = view->getFirst(); item != nullptr; item = view->getNext(item)) {
        if (item->name() == "..")
            continue;

        for (otherItem = otherPanel()->view->getFirst(); otherItem != nullptr && otherItem->name() != item->name();
             otherItem = otherPanel()->view->getNext(otherItem))
            ;

        bool isSingle = (otherItem == nullptr), isDifferent = false, isNewer = false;

        if (func->getFileItem(item)->isDir() && !selectDirs) {
            item->setSelected(false);
            continue;
        }

        if (otherItem) {
            if (!func->getFileItem(item)->isDir())
                isDifferent = otherPanel()->func->getFileItem(otherItem)->getSize() != func->getFileItem(item)->getSize();
            isNewer = func->getFileItem(item)->getModificationTime() > otherPanel()->func->getFileItem(otherItem)->getModificationTime();
        }

        switch (compareMode) {
        case 0:
            item->setSelected(isNewer || isSingle);
            break;
        case 1:
            item->setSelected(isNewer);
            break;
        case 2:
            item->setSelected(isSingle);
            break;
        case 3:
            item->setSelected(isDifferent || isSingle);
            break;
        case 4:
            item->setSelected(isDifferent);
            break;
        }
    }

    view->updateView();

    if (otherPanelToo)
        otherPanel()->gui->compareDirs(false);
}

void ListPanel::slotRefreshColors()
{
    view->refreshColors();
    emit signalRefreshColors(this == ACTIVE_PANEL);
}

void ListPanel::slotFocusOnMe(bool focus)
{
    if (focus && _manager->currentPanel() != this) {
        // ignore focus request if this panel is not shown
        return;
    }

    krApp->setUpdatesEnabled(false);

    if (focus) {
        emit activate();
        _actions->activePanelChanged();
        func->refreshActions();
        slotCurrentChanged(view->getCurrentKrViewItem());
        view->prepareForActive();
        otherPanel()->gui->slotFocusOnMe(false);
    } else {
        // in case a new url was entered but not refreshed to,
        // reset url navigator to the current url
        setNavigatorUrl(virtualPath());
        view->prepareForPassive();
    }

    urlNavigator->setActive(focus);
    slotRefreshColors();

    krApp->setUpdatesEnabled(true);
}

// this is used to start the panel
//////////////////////////////////////////////////////////////////
void ListPanel::start(const QUrl &url)
{
    QUrl startUrl(url);

    if (!startUrl.isValid())
        startUrl = QUrl::fromLocalFile(ROOT_DIR);

    _lastLocalPath = startUrl.isLocalFile() ? startUrl.path() : ROOT_DIR;

    func->openUrl(startUrl);

    setJumpBack(startUrl);
}

void ListPanel::slotStartUpdate(bool directoryChange)
{
    if (inlineRefreshJob)
        inlineRefreshListResult(nullptr);

    setCursor(Qt::BusyCursor);

    const QUrl currentUrl = virtualPath();
    if (directoryChange) {
        if (this == ACTIVE_PANEL && !MAIN_VIEW->terminalDock()->hasFocus()) {
            slotFocusOnMe();
        }

        if (currentUrl.isLocalFile())
            _lastLocalPath = currentUrl.path();

        setNavigatorUrl(currentUrl);

        emit pathChanged(currentUrl);

        krApp->popularUrls()->addUrl(currentUrl);

        searchBar->hideBar();
    }

    if (compareMode)
        otherPanel()->view->refresh();

    // return cursor to normal arrow
    setCursor(Qt::ArrowCursor);

    slotUpdateTotals();
}

void ListPanel::updateFilesystemStats(const QString &metaInfo, const QString &fsType, KIO::filesize_t total, KIO::filesize_t free)
{
    QString statusText, mountPoint, freeSpaceText;

    if (!metaInfo.isEmpty()) {
        statusText = metaInfo;
        mountPoint = freeSpaceText = "";
    } else {
        const int perc = total == 0 ? 0 : (int)(((float)free / (float)total) * 100.0);
        mountPoint = func->files()->mountPoint();
        statusText = i18nc(
            "%1=free space,%2=total space,%3=percentage of usage, "
            "%4=mountpoint,%5=filesystem type",
            "%1 free out of %2 (%3%) on %4 [(%5)]",
            KIO::convertSize(free),
            KIO::convertSize(total),
            perc,
            mountPoint,
            fsType);

        freeSpaceText = "    " + i18n("%1 free", KIO::convertSize(free));
    }

    status->setText(statusText);
    freeSpace->setText(freeSpaceText);
    mediaButton->updateIcon(mountPoint);
}

void ListPanel::handleDrop(QDropEvent *event, QWidget *targetFrame)
{
    // check what was dropped
    const QList<QUrl> urls = KUrlMimeData::urlsFromMimeData(event->mimeData());
    if (urls.isEmpty()) {
        event->ignore(); // not for us to handle!
        return;
    }

    // find dropping destination
    QString destinationDir = "";
    const bool dragFromThisPanel = event->source() == this;
    const KrViewItem *item = !targetFrame ? view->getKrViewItemAt(event->position().toPoint()) : nullptr;
    if (item) {
        const FileItem *file = item->getFileItem();
        if (file && !file->isDir() && dragFromThisPanel) {
            event->ignore(); // dragging on files in same panel, ignore
            return;
        } else if (!file || file->isDir()) { // item is ".." dummy or a directory
            destinationDir = item->name();
        }
    } else if (dragFromThisPanel) {
        event->ignore(); // dragged from this panel onto an empty spot in this panel, ignore
        return;
    }

    QUrl destination = QUrl(virtualPath());
    destination.setPath(destination.path() + '/' + destinationDir);

    QWidget *targetWidget = targetFrame ? targetFrame : view->widget();

    func->files()->dropFiles(destination, event, targetWidget);

    if (KConfigGroup(krConfig, "Look&Feel").readEntry("UnselectBeforeOperation", _UnselectBeforeOperation)) {
        KrPanel *p = dragFromThisPanel ? this : otherPanel();
        p->view->saveSelection();
        p->view->unselectAll();
    }
}

void ListPanel::handleDrop(const QUrl &destination, QDropEvent *event)
{
    func->files()->dropFiles(destination, event, view->widget());
}

void ListPanel::startDragging(const QStringList &names, const QPixmap &px)
{
    if (names.isEmpty()) { // avoid dragging empty urls
        return;
    }

    QList<QUrl> urls = func->files()->getUrls(names);

    auto *drag = new QDrag(this);
    auto *mimeData = new QMimeData;
    drag->setPixmap(px);
    mimeData->setUrls(urls);
    drag->setMimeData(mimeData);

    drag->exec(Qt::MoveAction | Qt::CopyAction | Qt::LinkAction);
}

// pops a right-click menu for items
void ListPanel::popRightClickMenu(const QPoint &loc)
{
    // run it, on the mouse location
    int j = QFontMetrics(font()).height() * 2;
    auto menu = PanelContextMenu::run(QPoint(loc.x() + 5, loc.y() + j), this);
    _contextMenu.reset(menu);
}

void ListPanel::popEmptyRightClickMenu(const QPoint &loc)
{
    auto menu = PanelContextMenu::run(loc, this);
    _contextMenu.reset(menu);
}

QString ListPanel::getCurrentName() const
{
    const QString name = view->getCurrentItem();
    return name == ".." ? QString() : name;
}

QStringList ListPanel::getSelectedNames()
{
    QStringList fileNames;
    view->getSelectedItems(&fileNames);
    return fileNames;
}

void ListPanel::prepareToDelete()
{
    const bool skipCurrent = (view->numSelected() == 0);
    view->setNameToMakeCurrent(view->firstUnmarkedBelowCurrent(skipCurrent));
}

void ListPanel::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (e->modifiers() & Qt::ControlModifier) {
            if (e->modifiers() & Qt::AltModifier) {
                FileItem *fileitem = func->files()->getFileItem(view->getCurrentKrViewItem()->name());
                if (fileitem && fileitem->isDir())
                    duplicateTab(fileitem->getUrl(), true);
            } else {
                SLOTS->insertFileName((e->modifiers() & Qt::ShiftModifier) != 0);
            }
        } else {
            e->ignore();
        }
        break;
    case Qt::Key_Right:
    case Qt::Key_Left:
        if (e->modifiers() == Qt::ControlModifier) {
            // user pressed CTRL+Right/Left - refresh other panel to the selected path if it's a
            // directory otherwise as this one
            if ((isLeft() && e->key() == Qt::Key_Right) || (!isLeft() && e->key() == Qt::Key_Left)) {
                if (KrViewItem *it = view->getCurrentKrViewItem()) {
                    QUrl newPath;
                    if (it->name() == "..") {
                        newPath = KIO::upUrl(virtualPath());
                    } else {
                        FileItem *v = func->getFileItem(it);
                        // If it's a directory different from ".."
                        if (v && v->isDir() && v->getName() != "..") {
                            newPath = v->getUrl();
                        } else {
                            // If it's a supported compressed file
                            if (v && KrArcHandler::arcSupported(v->getMime())) {
                                newPath = func->browsableArchivePath(v->getUrl().fileName());
                            } else {
                                newPath = virtualPath();
                            }
                        }
                    }
                    otherPanel()->func->openUrl(newPath);
                }
            } else {
                func->openUrl(otherPanel()->virtualPath());
            }
            return;
        } else
            e->ignore();
        break;
    case Qt::Key_Down:
        if (e->modifiers() == Qt::ControlModifier) { // give the keyboard focus to the command line
            if (MAIN_VIEW->cmdLine()->isVisible())
                MAIN_VIEW->cmdLineFocus();
            else
                MAIN_VIEW->focusTerminalEmulator();
            return;
        } else if (e->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) { // give the keyboard focus to TE
            MAIN_VIEW->focusTerminalEmulator();
        } else
            e->ignore();
        break;

    case Qt::Key_Up:
        if (e->modifiers() == Qt::ControlModifier) { // give the keyboard focus to the url navigator
            editLocation();
            return;
        } else
            e->ignore();
        break;

    case Qt::Key_Escape:
        cancelProgress();
        break;

    default:
        // if we got this, it means that the view is not doing
        // the quick search thing, so send the characters to the commandline, if normal key
        if (e->modifiers() == Qt::NoModifier)
            MAIN_VIEW->cmdLine()->addText(e->text());

        // e->ignore();
    }
}

void ListPanel::showEvent(QShowEvent *e)
{
    panelVisible();
    QWidget::showEvent(e);
}

void ListPanel::hideEvent(QHideEvent *e)
{
    panelHidden();
    QWidget::hideEvent(e);
}

void ListPanel::panelVisible()
{
    func->setPaused(false);
}

void ListPanel::panelHidden()
{
    func->setPaused(true);
}

void ListPanel::slotPreviewJobStarted(KJob *job)
{
    previewJob = job;
    connect(job, &KJob::percentChanged, this, &ListPanel::slotPreviewJobPercent);
    connect(job, &KJob::result, this, &ListPanel::slotPreviewJobResult);
    cancelProgressButton->setMaximumHeight(sidebarButton->height());
    cancelProgressButton->show();
    previewProgress->setValue(0);
    previewProgress->setFormat(i18n("loading previews: %p%"));
    previewProgress->setMaximumHeight(cancelProgressButton->height());
    previewProgress->show();
}

void ListPanel::slotPreviewJobPercent(KJob * /*job*/, unsigned long percent)
{
    previewProgress->setValue(static_cast<int>(percent));
}

void ListPanel::slotPreviewJobResult(KJob * /*job*/)
{
    previewJob = nullptr;
    previewProgress->hide();
    if (!inlineRefreshJob)
        cancelProgressButton->hide();
}

void ListPanel::slotRefreshJobStarted(KIO::Job *job)
{
    // disable the parts of the panel we don't want touched
    status->setEnabled(false);
    urlNavigator->setEnabled(false);
    cdRootButton->setEnabled(false);
    cdHomeButton->setEnabled(false);
    cdUpButton->setEnabled(false);
    cdOtherButton->setEnabled(false);
    sidebarButton->setEnabled(false);
    if (sidebar)
        sidebar->setEnabled(false);
    bookmarksButton->setEnabled(false);
    historyButton->setEnabled(false);
    syncBrowseButton->setEnabled(false);

    // connect to the job interface to provide in-panel refresh notification
    connect(job, &KIO::Job::infoMessage, this, &ListPanel::inlineRefreshInfoMessage);
    connect(job, &KIO::Job::percentChanged, this, &ListPanel::inlineRefreshPercent);
    connect(job, &KIO::Job::result, this, &ListPanel::inlineRefreshListResult);

    inlineRefreshJob = job;

    totals->setText(i18n(">> Reading..."));
    cancelProgressButton->show();
}

void ListPanel::cancelProgress()
{
    if (inlineRefreshJob) {
        disconnect(inlineRefreshJob, nullptr, this, nullptr);
        inlineRefreshJob->kill(KJob::EmitResult);
        inlineRefreshListResult(nullptr);
    }
    if (previewJob) {
        disconnect(previewJob, nullptr, this, nullptr);
        previewJob->kill(KJob::EmitResult);
        slotPreviewJobResult(nullptr);
    }
}

void ListPanel::setNavigatorUrl(const QUrl &url)
{
    _navigatorUrl = url;
    urlNavigator->setLocationUrl(url);
}

void ListPanel::inlineRefreshPercent(KJob *, unsigned long perc)
{
    QString msg = i18n(">> Reading: %1 % complete...", perc);
    totals->setText(msg);
}

void ListPanel::inlineRefreshInfoMessage(KJob *, const QString &msg)
{
    totals->setText(i18n(">> Reading: %1", msg));
}

void ListPanel::inlineRefreshListResult(KJob *)
{
    if (inlineRefreshJob)
        disconnect(inlineRefreshJob, nullptr, this, nullptr);
    inlineRefreshJob = nullptr;
    // reenable everything
    status->setEnabled(true);
    urlNavigator->setEnabled(true);
    cdRootButton->setEnabled(true);
    cdHomeButton->setEnabled(true);
    cdUpButton->setEnabled(true);
    cdOtherButton->setEnabled(true);
    sidebarButton->setEnabled(true);
    if (sidebar)
        sidebar->setEnabled(true);
    bookmarksButton->setEnabled(true);
    historyButton->setEnabled(true);
    syncBrowseButton->setEnabled(true);

    if (!previewJob)
        cancelProgressButton->hide();
}

void ListPanel::jumpBack()
{
    func->openUrl(_jumpBackURL);
}

void ListPanel::setJumpBack(QUrl url)
{
    _jumpBackURL = std::move(url);
}

void ListPanel::slotFilesystemError(const QString &msg)
{
    slotRefreshColors();
    fileSystemError->setText(i18n("Error: %1", msg));
    fileSystemError->show();
}

void ListPanel::showButtonMenu(QToolButton *b)
{
    if (this != ACTIVE_PANEL)
        slotFocusOnMe();

    if (b->isHidden())
        b->menu()->exec(mapToGlobal(clientArea->pos()));
    else
        b->click();
}

void ListPanel::openBookmarks()
{
    showButtonMenu(bookmarksButton);
}

void ListPanel::openHistory()
{
    showButtonMenu(historyButton);
}

void ListPanel::openMedia()
{
    showButtonMenu(mediaButton);
}

void ListPanel::rightclickMenu()
{
    if (view->getCurrentKrViewItem())
        popRightClickMenu(mapToGlobal(view->getCurrentKrViewItem()->itemRect().topLeft()));
}

void ListPanel::toggleSyncBrowse()
{
    syncBrowseButton->toggle();
}

void ListPanel::editLocation()
{
    urlNavigator->setUrlEditable(true);
    urlNavigator->setFocus();
    urlNavigator->editor()->lineEdit()->selectAll();
}

void ListPanel::navigateLocation()
{
    if (urlNavigator->editor()->isVisible())
        urlNavigator->setUrlEditable(false);
}

void ListPanel::showSearchBar()
{
    searchBar->showBar(KrSearchBar::MODE_SEARCH);
}

void ListPanel::showSearchBarSelection()
{
    searchBar->showBar(KrSearchBar::MODE_SELECT);
}

void ListPanel::showSearchBarFilter()
{
    searchBar->showBar(KrSearchBar::MODE_FILTER);
}

void ListPanel::saveSettings(KConfigGroup cfg, bool saveHistory)
{
    QUrl url = virtualPath();
    url.setPassword(QString()); // make sure no password is saved
    cfg.writeEntry("Url", url.toString());
    cfg.writeEntry("Type", getType());
    cfg.writeEntry("Properties", getProperties());
    cfg.writeEntry("PinnedUrl", pinnedUrl().toString());
    if (saveHistory)
        func->history->save(KConfigGroup(&cfg, "History"));
    view->saveSettings(KConfigGroup(&cfg, "View"));

    // splitter/sidebar state
    if (sidebar && !sidebar->isHidden()) {
        sidebar->saveSettings(KConfigGroup(&cfg, "PanelPopup"));
        cfg.writeEntry("PopupPosition", sidebarPosition());
        cfg.writeEntry("SplitterSizes", sidebarSplitter->saveState());
        cfg.writeEntry("PopupPage", sidebar->currentPage());
    } else {
        cfg.deleteEntry("PopupPosition");
        cfg.deleteEntry("SplitterSizes");
        cfg.deleteEntry("PopupPage");
    }
}

void ListPanel::restoreSettings(KConfigGroup cfg)
{
    changeType(cfg.readEntry("Type", defaultPanelType()));
    view->restoreSettings(KConfigGroup(&cfg, "View"));

    // "locked" property must be set after URL path is restored!
    // This panel can be reused when loading a profile,
    // so we reset its properties before calling openUrl().
    setProperties(0);

    _lastLocalPath = ROOT_DIR;
    if (func->history->restore(KConfigGroup(&cfg, "History"))) {
        func->refresh();
    } else {
        QUrl url(cfg.readEntry("Url", "invalid"));
        if (!url.isValid())
            url = QUrl::fromLocalFile(ROOT_DIR);
        func->openUrl(url);
    }

    setJumpBack(func->history->currentUrl());

    setProperties(cfg.readEntry("Properties", 0));

    if (isPinned()) {
        QUrl pinnedUrl(cfg.readEntry("PinnedUrl", "invalid"));
        if (!pinnedUrl.isValid()) {
            pinnedUrl = func->history->currentUrl();
        }
        func->openUrl(pinnedUrl);
        setPinnedUrl(pinnedUrl);
    }

    if (cfg.hasKey("PopupPosition")) { // sidebar was visible, restore
        toggleSidebar(); // create and show
        sidebar->restoreSettings(KConfigGroup(&cfg, "PanelPopup"));
        setSidebarPosition(cfg.readEntry("PopupPosition", 42 /* dummy */));
        sidebarSplitter->restoreState(cfg.readEntry("SplitterSizes", QByteArray()));
        sidebar->setCurrentPage(cfg.readEntry("PopupPage", 0));
    }
}

void ListPanel::slotCurrentChanged(KrViewItem *item)
{
    // update status bar
    if (item)
        krApp->statusBarUpdate(item->description());

    // update sidebar; which panel to display on?
    Sidebar *p;
    if (sidebar && !sidebar->isHidden()) {
        p = sidebar;
    } else if (otherPanel()->gui->sidebar && !otherPanel()->gui->sidebar->isHidden()) {
        p = otherPanel()->gui->sidebar;
    } else {
        return;
    }

    p->update(item ? func->files()->getFileItem(item->name()) : nullptr);
}

void ListPanel::otherPanelChanged()
{
    func->syncURL = QUrl();
}

void ListPanel::getFocusCandidates(QVector<QWidget *> &widgets)
{
    widgets << urlNavigator->editor();
    if (view->widget()->isVisible())
        widgets << view->widget();
    if (sidebar && sidebar->isVisible())
        widgets << sidebar;
}

void ListPanel::updateButtons()
{
    backButton->setEnabled(func->history->canGoBack());
    forwardButton->setEnabled(func->history->canGoForward());
    historyButton->setEnabled(func->history->count() > 1);
    cdRootButton->setEnabled(!virtualPath().matches(QUrl::fromLocalFile(ROOT_DIR), QUrl::StripTrailingSlash));
    cdUpButton->setEnabled(!func->files()->isRoot());
    cdHomeButton->setEnabled(!func->atHome());
}

void ListPanel::duplicateTab(KrViewItem *it)
{
    if (!it)
        return;
    else if (it->name() == "..") {
        duplicateTab(KIO::upUrl(virtualPath()), true);
    } else if (func->getFileItem(it)->isDir()) {
        QUrl url = virtualPath();
        url = url.adjusted(QUrl::StripTrailingSlash);
        url.setPath(url.path() + '/' + (it->name()));
        duplicateTab(url, true);
    }
}

void ListPanel::duplicateTab(const QUrl &url, bool nextToThis)
{
    _manager->duplicateTab(url, nextToThis ? this : nullptr);
}

void ListPanel::slotNavigatorUrlChanged(const QUrl &url)
{
    if (url == _navigatorUrl)
        return; // this is the URL we just set ourself

    if (!isNavigatorEditModeSet()) {
        urlNavigator->setUrlEditable(false);
    }

    func->openUrl(KrServices::escapeFileUrl(url), QString(), true);
}

void ListPanel::resetNavigatorMode()
{
    if (isNavigatorEditModeSet())
        return;

    // set to "navigate" mode if url wasn't changed
    if (urlNavigator->uncommittedUrl().matches(virtualPath(), QUrl::StripTrailingSlash)) {
        // NOTE: this also sets focus to the navigator
        urlNavigator->setUrlEditable(false);
        slotFocusOnMe();
    }
}

int ListPanel::sidebarPosition() const
{
    int pos = sidebarSplitter->orientation() == Qt::Vertical ? 1 : 0;
    return pos + (qobject_cast<Sidebar *>(sidebarSplitter->widget(0)) == NULL ? 2 : 0);
}

void ListPanel::setSidebarPosition(int pos)
{
    sidebarSplitter->setOrientation(pos % 2 == 0 ? Qt::Horizontal : Qt::Vertical);
    if ((pos < 2) != (qobject_cast<Sidebar *>(sidebarSplitter->widget(0)) != NULL)) {
        sidebarSplitter->insertWidget(0, sidebarSplitter->widget(1)); // swapping widgets in splitter
    }
}

void ListPanel::connectQuickSizeCalculator(SizeCalculator *sizeCalculator)
{
    connect(sizeCalculator, &SizeCalculator::started, this, [=]() {
        quickSizeCalcProgress->reset();
        quickSizeCalcProgress->show();
        cancelQuickSizeCalcButton->show();
    });
    connect(cancelQuickSizeCalcButton, &QToolButton::clicked, sizeCalculator, &SizeCalculator::cancel);
    connect(sizeCalculator, &SizeCalculator::progressChanged, quickSizeCalcProgress, &QProgressBar::setValue);
    connect(sizeCalculator, &SizeCalculator::finished, this, [=]() {
        cancelQuickSizeCalcButton->hide();
        quickSizeCalcProgress->hide();
    });
}
