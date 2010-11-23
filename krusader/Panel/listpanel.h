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

#include <kfileitem.h>
#include <kurl.h>
#include <qwidget.h>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtCore/QString>
#include <QtGui/QPixmap>
#include <QtGui/QToolButton>
#include <QProgressBar>
#include <QtCore/QDir>
#include <qpixmapcache.h>
#include <QtGui/QIcon>
#include <QDropEvent>
#include <QShowEvent>
#include <QGridLayout>
#include <QList>
#include <QHideEvent>
#include <QKeyEvent>
#include <QEvent>
#include <klineedit.h>
#include <QtCore/QPointer>

#include "krpanel.h"
#include "krview.h"
#include "../Dialogs/krsqueezedtextlabel.h"

#define PROP_SYNC_BUTTON_ON               1
#define PROP_LOCKED                       2
#define PROP_PREVIEWS                     4

class vfs;
class vfile;
class KrView;
class UrlRequester;
class KrQuickSearch;
class QuickFilter;
class DirHistoryButton;
class MediaButton;
class PanelPopup;
class SyncBrowseButton;
class KrBookmarkButton;
class KPushButton;
class ListPanelFunc;
class QSplitter;
class KDiskFreeSpace;
class KrErrorDisplay;
class ListPanelActions;

class ListPanel : public QWidget, public KrPanel
{
    friend class ListPanelFunc;
    Q_OBJECT
public:
#define ITEM2VFILE(PANEL_PTR, KRVIEWITEM)  PANEL_PTR->func->files()->vfs_search(KRVIEWITEM->name())
#define NAME2VFILE(PANEL_PTR, STRING_NAME) PANEL_PTR->func->files()->vfs_search(STRING_NAME)
    // constructor create the panel, but DOESN'T fill it with data, use start()
    ListPanel(int panelType, QWidget *parent, bool &left, AbstractPanelManager *manager);
    ~ListPanel();
    void start(KUrl url = KUrl(), bool immediate = false);

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
    QString realPath() const;
    QString getCurrentName();
    void getSelectedNames(QStringList* fileNames) {
        view->getSelectedItems(fileNames);
    }
    void setButtons();
    bool isLeft() {
        return _left;
    }
    void setJumpBack(KUrl url);

    int  getProperties();
    void setProperties(int);

    void getFocusCandidates(QVector<QWidget*> &widgets);

    void otherPanelChanged();

    void saveSettings(KConfigGroup &cfg);

public slots:
    void gotStats(const QString &mountPoint, quint64 kBSize, quint64 kBUsed, quint64 kBAvail);  // displays filesystem status
    void popRightClickMenu(const QPoint&);
    void popEmptyRightClickMenu(const QPoint &);
    void compareDirs(bool otherPanelToo = true);
    void slotFocusOnMe(); // give this VFS the focus (the path bar)
    void slotUpdateTotals();
    void slotStartUpdate();                   // internal
    void slotGetStats(const KUrl& url);            // get the disk-free stats
    void slotFocusAndCDOther();
    void togglePanelPopup();
    void panelActive(); // called when the panel becomes active
    void panelInactive(); // called when panel becomes inactive
    void vfs_refresh(KJob *job);
    void refreshColors();
    void inlineRefreshCancel();

    void openMedia();
    void openHistory();
    void openBookmarks();
    void rightclickMenu();
    void toggleSyncBrowse();
    void editLocation();
    void jumpBack();
    void setJumpBack() {
        setJumpBack(virtualPath());
    }

    ///////////////////////// service functions - called internally ////////////////////////
    void prepareToDelete();                   // internal use only

protected:
    virtual void keyPressEvent(QKeyEvent *e);
    virtual void mousePressEvent(QMouseEvent*) {
        slotFocusOnMe();
    }
    virtual void showEvent(QShowEvent *);
    virtual void hideEvent(QHideEvent *);
    virtual bool eventFilter(QObject * watched, QEvent * e);

    void showButtonMenu(QToolButton *b);
    void createView();
    void updateButtons();

protected slots:
    void updatePopupPanel(KrViewItem *item);
    void handleDropOnView(QDropEvent *, QWidget *destWidget = 0); // handles drops on the view only
    void startDragging(QStringList, QPixmap);
    void slotPreviewJobStarted(KJob *job);
    void slotPreviewJobPercent(KJob *job, unsigned long percent);
    void slotPreviewJobResult(KJob *job);
    // those handle the in-panel refresh notifications
    void slotJobStarted(KIO::Job* job);
    void inlineRefreshInfoMessage(KJob* job, const QString &msg);
    void inlineRefreshListResult(KJob* job);
    void inlineRefreshPercent(KJob*, unsigned long);
    void slotVfsError(QString msg);

signals:
    void signalStatus(QString msg);         // emmited when we need to update the status bar
    void pathChanged(ListPanel *panel);
    void activePanelChanged(ListPanel *p);   // emitted when the user changes panels
    void finishedDragging();              // currently
    void refreshColors(bool active);

protected:
    int panelType;
    KUrl _realPath; // named with _ to keep realPath() compatibility
    KUrl _jumpBackURL;
    int colorMask;
    bool compareMode;
    //FilterSpec    filter;
    KDiskFreeSpace* statsAgent;
    KJob *previewJob;
    KIO::Job *inlineRefreshJob;
    ListPanelActions *_actions;

    QPixmap currDragPix;
    QWidget *clientArea;
    QSplitter *splt;
    UrlRequester *origin;
    KrQuickSearch *quickSearch;
    QuickFilter *quickFilter;
    QToolButton *backButton, *forwardButton;
    QToolButton *cdRootButton;
    QToolButton *cdHomeButton;
    QToolButton *cdUpButton;
    QToolButton *cdOtherButton;
    QToolButton *popupBtn;
    PanelPopup *popup;
    KrBookmarkButton *bookmarksButton;
    KrSqueezedTextLabel *status, *totals, *freeSpace;

    QProgressBar *previewProgress;
    DirHistoryButton* historyButton;
    MediaButton *mediaButton;
    SyncBrowseButton *syncBrowseButton;
    QToolButton *inlineRefreshCancelButton;
    KrErrorDisplay *vfsError;

private:
    bool _locked;
    QList<int> popupSizes;
};

#endif
