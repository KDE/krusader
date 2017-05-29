/***************************************************************************
                               listpanel.h
                            -------------------
   begin                : Thu May 4 2000
   copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
   e-mail               : krusader@users.sourceforge.net
   web site             : http://krusader.sourceforge.net
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

                                                    H e a d e r    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#ifndef LISTPANEL_H
#define LISTPANEL_H

// QtCore
#include <QString>
#include <QDir>
#include <QList>
#include <QEvent>
#include <QPointer>
#include <QUrl>
// QtGui
#include <QPixmap>
#include <QPixmapCache>
#include <QIcon>
#include <QDropEvent>
#include <QKeyEvent>
#include <QShowEvent>
#include <QHideEvent>
// QtWidgets
#include <QProgressBar>
#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QSplitter>
#include <QToolButton>
#include <QGridLayout>

#include <KCompletion/KLineEdit>
#include <KConfigCore/KConfigGroup>
#include <KIO/Job>
#include <KIOCore/KFileItem>
#include <KIOFileWidgets/KUrlNavigator>

#include "krpanel.h"

#define PROP_SYNC_BUTTON_ON               1
#define PROP_LOCKED                       2

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
class PanelPopup;
class SizeCalculator;

class ListPanel : public QWidget, public KrPanel
{
    friend class ListPanelFunc;
    Q_OBJECT
public:
    // constructor create the panel, but DOESN'T fill it with data, use start()
    ListPanel(QWidget *parent, AbstractPanelManager *manager, KConfigGroup cfg = KConfigGroup());
    ~ListPanel();

    virtual void otherPanelChanged() Q_DECL_OVERRIDE;

    void start(const QUrl &url = QUrl());

    void reparent(QWidget *parent, AbstractPanelManager *manager);

    int getType() {
        return panelType;
    }
    void changeType(int);
    bool isLocked() {
        return _locked;
    }
    void setLocked(bool lck) {
        _locked = lck;
    }

    ListPanelActions *actions() {
        return _actions;
    }
    QString lastLocalPath() const;
    QString getCurrentName();
    QStringList getSelectedNames();
    void setButtons();
    void setJumpBack(QUrl url);

    int  getProperties();
    void setProperties(int);

    void getFocusCandidates(QVector<QWidget*> &widgets);

    void saveSettings(KConfigGroup cfg, bool saveHistory);
    void restoreSettings(KConfigGroup cfg);

public slots:
    void popRightClickMenu(const QPoint&);
    void popEmptyRightClickMenu(const QPoint &);
    void compareDirs(bool otherPanelToo = true);
    void slotFocusOnMe(bool focus = true);
    void slotUpdateTotals();
    // react to file changes in filesystem (path change or refresh)
    void slotStartUpdate(bool directoryChange);
    void togglePanelPopup();
    void panelVisible(); // called when the panel becomes active
    void panelHidden(); // called when panel becomes inactive
    void refreshColors();
    void cancelProgress(); // cancel filesystem refresh and/or preview (if running)
    void setNavigatorUrl(const QUrl &url);

    void openMedia();
    void openHistory();
    void openBookmarks();
    void rightclickMenu();
    void toggleSyncBrowse();
    void editLocation();
    void showSearchBar();
    void showSearchFilter();
    void jumpBack();
    void setJumpBack() {
        setJumpBack(virtualPath());
    }

    ///////////////////////// service functions - called internally ////////////////////////
    void prepareToDelete();                   // internal use only

protected:
    virtual void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent*) Q_DECL_OVERRIDE {
        slotFocusOnMe();
    }
    virtual void showEvent(QShowEvent *) Q_DECL_OVERRIDE;
    virtual void hideEvent(QHideEvent *) Q_DECL_OVERRIDE;
    virtual bool eventFilter(QObject * watched, QEvent * e) Q_DECL_OVERRIDE;

    void showButtonMenu(QToolButton *b);
    void createView();
    void updateButtons();

    static int defaultPanelType();
    static bool isNavigatorEditModeSet(); // return the navigator edit mode setting

protected slots:
    void slotCurrentChanged(KrViewItem *item);
    void handleDrop(QDropEvent *event, bool onView = false); // handle drops on frame or view
    void handleDrop(const QUrl &destination, QDropEvent *event); // handle drops with destination
    void startDragging(QStringList, QPixmap);
    void slotPreviewJobStarted(KJob *job);
    void slotPreviewJobPercent(KJob *job, unsigned long percent);
    void slotPreviewJobResult(KJob *job);
    // those handle the in-panel refresh notifications
    void slotRefreshJobStarted(KIO::Job* job);
    void inlineRefreshInfoMessage(KJob* job, const QString &msg);
    void inlineRefreshListResult(KJob* job);
    void inlineRefreshPercent(KJob*, unsigned long);
    void slotFilesystemError(QString msg);
    void newTab(KrViewItem *item);
    void newTab(const QUrl &url, bool nextToThis = false);
    void slotNavigatorUrlChanged(const QUrl &url);
    void resetNavigatorMode(); // set navigator mode after focus was lost
    // update filesystem meta info, disk-free and mount status
    void updateFilesystemStats(const QString &metaInfo,  const QString &fsType,
                               KIO::filesize_t total, KIO::filesize_t free);

signals:
    void signalStatus(QString msg); // emmited when we need to update the status bar
    void pathChanged(const QUrl &url); // directory changed or refreshed
    void activate(); // emitted when the user changes panels
    void finishedDragging(); // NOTE: currently not used
    void refreshColors(bool active);

protected:
    int panelType;
    QString _lastLocalPath;
    QUrl _jumpBackURL;
    int colorMask;
    bool compareMode;
    //FilterSpec    filter;
    KJob *previewJob;
    KIO::Job *inlineRefreshJob;
    ListPanelActions *_actions;

    QPixmap currDragPix;
    QWidget *clientArea;
    QSplitter *splt;
    KUrlNavigator* urlNavigator;
    KrSearchBar* searchBar;
    QToolButton *backButton, *forwardButton;
    QToolButton *cdRootButton;
    QToolButton *cdHomeButton;
    QToolButton *cdUpButton;
    QToolButton *cdOtherButton;
    QToolButton *popupPositionBtn;
    QToolButton *popupBtn;
    PanelPopup *popup; // lazy initialized
    KrBookmarkButton *bookmarksButton;
    KrSqueezedTextLabel *status, *totals, *freeSpace;

    QProgressBar *quickSizeCalcProgress;
    QToolButton *cancelQuickSizeCalcButton;
    QProgressBar *previewProgress;
    DirHistoryButton* historyButton;
    MediaButton *mediaButton;
    QToolButton *syncBrowseButton;
    QToolButton *cancelProgressButton; // for thumbnail previews and filesystem refresh
    KrErrorDisplay *fileSystemError;

private:
    bool handleDropInternal(QDropEvent *event, const QString &dir);
    int popupPosition() const; // 0: West, 1: North, 2: East, 3: South
    void setPopupPosition(int);
    void connectQuickSizeCalculator(SizeCalculator *sizeCalculator);

private:
    QUrl _navigatorUrl; // distinguish between new user set URL and new custom set URL
    bool _locked;
    QList<int> popupSizes;
};

#endif
