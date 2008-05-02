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

#include <kpropsdlg.h>
#include <kfileitem.h>
#include <kurl.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qtoolbutton.h>
#include <qdir.h>
#include <qpixmapcache.h>
#include <qicon.h>
#include <q3textbrowser.h>
#include <QDropEvent>
#include <QShowEvent>
#include <QGridLayout>
#include <QList>
#include <QHideEvent>
#include <QKeyEvent>
#include <QEvent>
#include <klineedit.h>
#include <qpointer.h>
#include "krview.h"
#include "../Dialogs/krsqueezedtextlabel.h"

#define PROP_SYNC_BUTTON_ON               1

class vfs;
class vfile;
class KrView;
class KUrlRequester;
class BookmarksButton;
class KrQuickSearch;
class DirHistoryButton;
class DirHistoryQueue;
class MediaButton;
class PanelPopup;
class SyncBrowseButton;
class KrBookmarkButton;
class KPushButton;
class ListPanelFunc;
class QSplitter;
class KDiskFreeSpace;

class ListPanel : public QWidget {
   friend class ListPanelFunc;
   Q_OBJECT
public:
	#define ITEM2VFILE(PANEL_PTR, KRVIEWITEM)		PANEL_PTR->func->files()->vfs_search(KRVIEWITEM->name())
	#define NAME2VFILE(PANEL_PTR, STRING_NAME)	PANEL_PTR->func->files()->vfs_search(STRING_NAME)
   // constructor create the panel, but DOESN'T fill it with data, use start()
   ListPanel( int panelType, QWidget *parent, bool &left );
   ~ListPanel();
   void start( KUrl url = KUrl(), bool immediate = false );
   
   int getType() { return panelType; }
   void changeType( int );
   
   KUrl virtualPath() const;
	QString realPath() const;
   QString getCurrentName();
   void getSelectedNames( QStringList* fileNames ) {
      view->getSelectedItems( fileNames );
   }
   void setPanelToolbar();
   bool isLeft() {return _left;}
   void jumpBack();
   void setJumpBack( KUrl url );

   int  getProperties();
   void setProperties( int );

public slots:
   void gotStats( const QString &mountPoint, quint64 kBSize, quint64 kBUsed, quint64 kBAvail); // displays filesystem status
   void popRightClickMenu( const QPoint& );
   void popEmptyRightClickMenu( const QPoint & );
   void select( KRQuery query, bool select);
   void select( bool, bool );      // see doc in ListPanel
   void invertSelection();       // see doc in ListPanel
   void compareDirs();
   void slotFocusOnMe(); // give this VFS the focus (the path bar)
   void slotUpdate();			                  // when the vfs finish to update...
   void slotUpdateTotals();
   void slotStartUpdate();                   // internal
   void slotGetStats( const KUrl& url );          // get the disk-free stats
   void setFilter( KrViewProperties::FilterSpec f );
   void slotFocusAndCDRoot();
   void slotFocusAndCDHome();
   void slotFocusAndCDup();
   void slotFocusAndCDOther();
	void togglePanelPopup();
	// for signals from vfs' dirwatch
	void slotItemAdded(vfile *vf);
	void slotItemDeleted(const QString& name);
	void slotItemUpdated(vfile *vf);
	void slotCleared();        
	void panelActive(); // called when the panel becomes active
	void panelInactive(); // called when panel becomes inactive
	

   ///////////////////////// service functions - called internally ////////////////////////
   inline void setOther( ListPanel *panel ) {
      otherPanel = panel;
   }
   void prepareToDelete();                   // internal use only

protected:
   virtual void keyPressEvent( QKeyEvent *e );
   virtual void showEvent( QShowEvent * );
   virtual void hideEvent( QHideEvent * );
   virtual bool eventFilter ( QObject * watched, QEvent * e );
   
   void createView();

protected slots:
   void handleDropOnView(QDropEvent *, QWidget *destWidget=0); // handles drops on the view only
   void handleDropOnTotals( QDropEvent * );                   // handles drops on the totals line
   void handleDropOnStatus( QDropEvent * );                   // handles drops on the status line
   void startDragging( QStringList, QPixmap );
	// those handle the in-panel refresh notifications
	void slotJobStarted(KIO::Job* job);
	void inlineRefreshInfoMessage( KJob* job, const QString &msg );
	void inlineRefreshListResult(KJob* job);
	void inlineRefreshPercent( KJob*, unsigned long );
	void inlineRefreshCancel();

signals:
   void signalStatus( QString msg );       // emmited when we need to update the status bar
   void cmdLineUpdate( QString p );	      // emitted when we need to update the command line
   void pathChanged( ListPanel *panel );
   void activePanelChanged( ListPanel *p ); // emitted when the user changes panels
   void finishedDragging();              // currently

public:
   int panelType;
   ListPanelFunc	*func;
   KrView *view;
   ListPanel	*otherPanel;
   int colorMask;
   bool compareMode;
   //FilterSpec	   filter;
   KRQuery filterMask;
   QPixmap currDragPix;
   KDiskFreeSpace* statsAgent;
   KrSqueezedTextLabel *status, *totals;
   KrQuickSearch *quickSearch;
   KUrlRequester *origin;
   QGridLayout *layout;
   QToolButton *cdRootButton;
   QToolButton *cdHomeButton;
   QToolButton *cdUpButton;
   QToolButton *cdOtherButton;
	QToolButton *popupBtn;
	QToolButton *clearOrigin;
	PanelPopup *popup;
   KrBookmarkButton *bookmarksButton;
   DirHistoryQueue* dirHistoryQueue;
   DirHistoryButton* historyButton;
   MediaButton *mediaButton;
   SyncBrowseButton *syncBrowseButton;
	KPushButton *inlineRefreshCancelButton;
	KIO::Job *inlineRefreshJob;
	QSplitter *splt;

protected:
   KUrl _realPath; // named with _ to keep realPath() compatability
   KUrl _jumpBackURL;
   
	
private:
   bool &_left;
   QList<int> popupSizes;
};

#endif
