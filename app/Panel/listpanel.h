/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LISTPANEL_H
#define LISTPANEL_H

// QtCore
#include <QDir>
#include <QEvent>
#include <QList>
#include <QPointer>
#include <QString>
#include <QUrl>
// QtGui
#include <QDropEvent>
#include <QHideEvent>
#include <QIcon>
#include <QKeyEvent>
#include <QPixmap>
#include <QPixmapCache>
#include <QShowEvent>
// QtWidgets
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QProgressBar>
#include <QSplitter>
#include <QToolButton>
#include <QWidget>

#include <KConfigGroup>
#include <KFileItem>
#include <KIO/Job>
#include <KLineEdit>
#include <KUrlNavigator>

#include "krpanel.h"
#include "panelcontextmenu.h"

#define PROP_SYNC_BUTTON_ON 1
#define PROP_LOCKED 2
#define PROP_PINNED 4

class DirHistoryButton;
class KrBookmarkButton;
class KrErrorDisplay;
class KrSqueezedTextLabel;
class KrSearchBar;
class KrView;
class KrViewItem;
class ListPanelActions;
class ListPanelFunc;
class MediaButton;
class Sidebar;
class SizeCalculator;

class ListPanel : public QWidget, public KrPanel
{
    friend class ListPanelFunc;
    Q_OBJECT
public:
    enum TabState {
        DEFAULT,
        LOCKED,
        // locked tab with changeable address
        PINNED
    };

    // constructor create the panel, but DOESN'T fill it with data, use start()
    ListPanel(QWidget *parent, AbstractPanelManager *manager, const KConfigGroup &cfg = KConfigGroup());
    ~ListPanel() override;

    void otherPanelChanged() override;

    void start(const QUrl &url = QUrl());

    void reparent(QWidget *parent, AbstractPanelManager *manager);

    int getType()
    {
        return panelType;
    }
    void changeType(int);
    bool isLocked()
    {
        return _tabState == TabState::LOCKED;
    }
    bool isPinned()
    {
        return _tabState == TabState::PINNED;
    }
    void setTabState(TabState tabState)
    {
        _tabState = tabState;
    }

    ListPanelActions *actions()
    {
        return _actions;
    }
    QString lastLocalPath() const;
    QString getCurrentName() const;
    QStringList getSelectedNames();
    void setButtons();
    void setJumpBack(QUrl url);

    int getProperties();
    void setProperties(int);

    void getFocusCandidates(QVector<QWidget *> &widgets);

    void saveSettings(KConfigGroup cfg, bool saveHistory);
    void restoreSettings(KConfigGroup cfg);

    void setPinnedUrl(QUrl &pinnedUrl)
    {
        _pinnedUrl = pinnedUrl;
    };
    QUrl pinnedUrl() const
    {
        return _pinnedUrl;
    };

public slots:
    void handleDrop(QDropEvent *event, QWidget *targetFrame = nullptr); // handle drops on frame or view
    void handleDrop(const QUrl &destination, QDropEvent *event); // handle drops with destination
    void popRightClickMenu(const QPoint &);
    void popEmptyRightClickMenu(const QPoint &);
    void compareDirs(bool otherPanelToo = true);
    void slotFocusOnMe(bool focus = true);
    void slotUpdateTotals();
    // react to file changes in filesystem (path change or refresh)
    void slotStartUpdate(bool directoryChange);
    void toggleSidebar();
    void panelVisible(); // called when the panel becomes active
    void panelHidden(); // called when panel becomes inactive
    void slotRefreshColors();
    void cancelProgress(); // cancel filesystem refresh and/or preview (if running)
    void setNavigatorUrl(const QUrl &url);

    void openMedia();
    void openHistory();
    void openBookmarks();
    void rightclickMenu();
    void toggleSyncBrowse();
    void editLocation();
    void navigateLocation();
    void showSearchBar();
    void showSearchBarSelection();
    void showSearchBarFilter();
    void jumpBack();
    void setJumpBack()
    {
        setJumpBack(virtualPath());
    }

    ///////////////////////// service functions - called internally ////////////////////////
    void prepareToDelete(); // internal use only

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *) override
    {
        slotFocusOnMe();
    }
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;
    bool eventFilter(QObject *watched, QEvent *e) override;

    void showButtonMenu(QToolButton *b);
    void createView();
    void updateButtons();

    static int defaultPanelType();
    static bool isNavigatorEditModeSet(); // return the navigator edit mode setting

protected slots:
    void slotCurrentChanged(KrViewItem *item);
    void startDragging(const QStringList &, const QPixmap &);
    void slotPreviewJobStarted(KJob *job);
    void slotPreviewJobPercent(KJob *job, unsigned long percent);
    void slotPreviewJobResult(KJob *job);
    // those handle the in-panel refresh notifications
    void slotRefreshJobStarted(KIO::Job *job);
    void inlineRefreshInfoMessage(KJob *job, const QString &msg);
    void inlineRefreshListResult(KJob *job);
    void inlineRefreshPercent(KJob *, unsigned long);
    void slotFilesystemError(const QString &msg);
    void duplicateTab(KrViewItem *item);
    void duplicateTab(const QUrl &url, bool nextToThis = false);
    void slotNavigatorUrlChanged(const QUrl &url);
    void resetNavigatorMode(); // set navigator mode after focus was lost
    // update filesystem meta info, disk-free and mount status
    void updateFilesystemStats(const QString &metaInfo, const QString &fsType, KIO::filesize_t total, KIO::filesize_t free);

signals:
    void signalStatus(QString msg); // emitted when we need to update the status bar
    void pathChanged(const QUrl &url); // directory changed or refreshed
    void activate(); // emitted when the user changes panels
    void finishedDragging(); // NOTE: currently not used
    void signalRefreshColors(bool active);

protected:
    int panelType;
    QString _lastLocalPath;
    QUrl _jumpBackURL;
    int colorMask;
    bool compareMode;
    // FilterSpec    filter;
    KJob *previewJob;
    KIO::Job *inlineRefreshJob;
    ListPanelActions *_actions;

    QPixmap currDragPix;
    QWidget *clientArea;
    QSplitter *sidebarSplitter;
    KUrlNavigator *urlNavigator;
    KrSearchBar *searchBar;
    QToolButton *backButton, *forwardButton;
    QToolButton *cdRootButton;
    QToolButton *cdHomeButton;
    QToolButton *cdUpButton;
    QToolButton *cdOtherButton;
    QToolButton *sidebarPositionButton;
    QToolButton *sidebarButton;
    Sidebar *sidebar; // lazy initialized
    KrBookmarkButton *bookmarksButton;
    KrSqueezedTextLabel *status, *totals, *freeSpace;

    QProgressBar *quickSizeCalcProgress;
    QToolButton *cancelQuickSizeCalcButton;
    QProgressBar *previewProgress;
    DirHistoryButton *historyButton;
    MediaButton *mediaButton;
    QToolButton *syncBrowseButton;
    QToolButton *cancelProgressButton; // for thumbnail previews and filesystem refresh
    KrErrorDisplay *fileSystemError;

private:
    bool handleDropInternal(QDropEvent *event, const QString &dir);
    int sidebarPosition() const; // 0: West, 1: North, 2: East, 3: South
    void setSidebarPosition(int);
    void connectQuickSizeCalculator(SizeCalculator *sizeCalculator);

private:
    QUrl _navigatorUrl; // distinguish between new user set URL and new custom set URL
    QUrl _pinnedUrl; // only for TabState::PINNED
    TabState _tabState;
    QList<int> sidebarSplitterSizes;
    QScopedPointer<PanelContextMenu> _contextMenu;
};

#endif
