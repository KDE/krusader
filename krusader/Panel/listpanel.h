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
#include <qpopupmenu.h>
#include <qdir.h>
#include <qpixmapcache.h>
#include <qiconset.h>
#include <qptrstack.h>
#include <qtextbrowser.h>
#include <keditcl.h>
#include <klineedit.h>
#include <qguardedptr.h>
#include <kdiskfreesp.h>
#include "krview.h"
#include "../Dialogs/krsqueezedtextlabel.h"

class vfs;
class vfile;
class KRdirWatch;
class PanelFunc;
class KrView;
class KURLRequester;
class BookmarksButton;
class KrQuickSearch;
class DirHistoryButton;
class DirHistoryQueue;
class PanelPopup;
class SyncBrowseButton;

class ListPanel : public QWidget {
   friend class ListPanelFunc;
   Q_OBJECT
public:
   //enum FilterSpec{ALL,EXEC,CUSTOM};

   // constructor create the panel, but DOESN'T fill it with data, use start()
   ListPanel( QWidget *parent, bool &left, const char *name = 0 );
   ~ListPanel();
   void start( QString path = QString::null );
   inline QString getPath() {
      return virtualPath;
   }
   QString getCurrentName();
   void getSelectedNames( QStringList* fileNames ) {
      view->getSelectedItems( fileNames );
   }
   void setPanelToolbar();

public slots:
   void gotStats( const QString &mountPoint, unsigned long kBSize, unsigned long kBUsed, unsigned long kBAvail); // displays filesystem status
   void popRightClickMenu( const QPoint& );
   void select( bool, bool );      // see doc in ListPanel
   void invertSelection();       // see doc in ListPanel
   void slotFocusOnMe(); // give this VFS the focus (the path bar)
   void slotUpdate();			                  // when the vfs finish to update...
   void slotUpdateTotals();
   void slotStartUpdate();                   // internal
   void slotGetStats( QString path );          // get the disk-free stats
   void setFilter( KrView::FilterSpec f );
   void slotFocusAndCDRoot();
   void slotFocusAndCDHome();
   void slotFocusAndCDup();
   void slotFocusAndCDOther();
	// for signals from vfs' dirwatch
	void slotItemAdded(vfile *vf);
	void slotItemDeleted(const QString& name);
	void slotItemUpdated(vfile *vf);
	

   ///////////////////////// service functions - called internally ////////////////////////
   inline void setOther( ListPanel *panel ) {
      otherPanel = panel;
   }
   void prepareToDelete();                   // internal use only

protected:
   virtual void keyPressEvent( QKeyEvent *e );

protected slots:
   void handleDropOnView(QDropEvent *, QWidget *destWidget=0); // handles drops on the view only
   void handleDropOnTotals( QDropEvent * );                   // handles drops on the totals line
   void handleDropOnStatus( QDropEvent * );                   // handles drops on the status line
   void startDragging( QStringList, QPixmap );
   void togglePanelPopup();

signals:
   void signalStatus( QString msg );       // emmited when we need to update the status bar
   void cmdLineUpdate( QString p );	      // emitted when we need to update the command line
   void pathChanged( ListPanel *panel );
   void activePanelChanged( ListPanel *p ); // emitted when the user changes panels
   void finishedDragging();              // currently

public:
   ListPanelFunc	*func;
   KrView *view;
   ListPanel	*otherPanel;
   QString virtualPath;
   QString realPath;
   int colorMask;
   bool compareMode;
   //FilterSpec	   filter;
   QString	filterMask;
   QPixmap currDragPix;
   QListViewItem *currDragItem;
   KDiskFreeSp* statsAgent;
   KrSqueezedTextLabel *status, *totals;
   KrQuickSearch *quickSearch;
   KURLRequester *origin;
   QGridLayout *layout;
   QToolButton *cdRootButton;
   QToolButton *cdHomeButton;
   QToolButton *cdUpButton;
   QToolButton *cdOtherButton;
	QToolButton *popupBtn;
	PanelPopup *popup;
   BookmarksButton *bookmarksButton;
   DirHistoryQueue* dirHistoryQueue;
   DirHistoryButton* historyButton;
   SyncBrowseButton *syncBrowseButton;

private:
   bool &_left;
	QValueList<int> popupSizes;
};

#endif
