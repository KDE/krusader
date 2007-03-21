/***************************************************************************
                       synchronizergui.h  -  description
                             -------------------
    copyright            : (C) 2003 + by Csaba Karai
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

#ifndef __SYNCHRONIZERGUI_H__
#define __SYNCHRONIZERGUI_H__

#include "synchronizer.h"
#include "../GUI/profilemanager.h"
#include "../Filter/filtertabs.h"
#include "../Filter/generalfilter.h"
#include <qdialog.h>
#include <qlistview.h>
#include <kcombobox.h>
#include <qpixmap.h>
#include <qcheckbox.h>
#include <qmap.h>
#include <qlabel.h>
#include <qtabwidget.h>

class QSpinBox;

class SynchronizerGUI : QDialog
{
   Q_OBJECT

public:
  class SyncViewItem : public QListViewItem
  {
    private:
      SynchronizerFileItem *syncItemRef;
      SyncViewItem         *lastItemRef;
      QColor                textColor;
      QColor                baseColor;
            
    public:
      SyncViewItem( SynchronizerFileItem *item, QColor txt, QColor base, QListView * parent, QListViewItem *after, QString label1,
                    QString label2 = QString::null, QString label3 = QString::null, QString label4 = QString::null,
                    QString label5 = QString::null, QString label6 = QString::null,
                    QString label7 = QString::null, QString label8 = QString::null ) :
                      QListViewItem( parent, after, label1, label2, label3, label4, label5, label6,
                                     label7, label8 ), syncItemRef( item ), lastItemRef( 0 ), textColor( txt ), baseColor( base )
      {
        item->setUserData( (void *)this );
        setDragEnabled( true );
      }
      
      SyncViewItem( SynchronizerFileItem *item, QColor txt, QColor base, QListViewItem * parent, QListViewItem *after, QString label1,
                    QString label2 = QString::null, QString label3 = QString::null, QString label4 = QString::null,
                    QString label5 = QString::null, QString label6 = QString::null,
                    QString label7 = QString::null, QString label8 = QString::null ) :
                      QListViewItem( parent, after, label1, label2, label3, label4, label5, label6,
                                     label7, label8 ), syncItemRef( item ), lastItemRef( 0 ), textColor( txt ), baseColor( base )
      {
        item->setUserData( (void *)this );
        setDragEnabled( true );
      }

      ~SyncViewItem()
      {
        syncItemRef->setUserData( 0 );
      }

      inline SynchronizerFileItem * synchronizerItemRef()       {return syncItemRef;}
      inline SyncViewItem         * lastItem()                  {return lastItemRef;}
      inline void                   setLastItem(SyncViewItem*s) {lastItemRef = s;}
      
      void setColors( QColor fore, QColor back ) {
        textColor = fore;
        baseColor = back;
      }
      
      void paintCell(QPainter *p, const QColorGroup &cg, int column, int width, int align)
      {
        QColorGroup _cg = cg;
        if( textColor.isValid() )
          _cg.setColor(QColorGroup::Text, textColor );
        if( baseColor.isValid() )
          _cg.setColor(QColorGroup::Base, baseColor );
        QListViewItem::paintCell( p, _cg, column, width, align );
      };
  };
   
public:
  // if rightDirectory is null, leftDirectory is actually the profile name to load
  SynchronizerGUI(QWidget* parent,  KURL leftDirectory, KURL rightDirectory = QString::null, QStringList selList = QStringList() );
  SynchronizerGUI(QWidget* parent,  QString profile );
  ~SynchronizerGUI();

  inline bool wasSynchronization()    {return wasSync;}

public slots:
  void rightMouseClicked(QListViewItem *);
  void doubleClicked(QListViewItem *);
  void compare();
  void synchronize();
  void stop();
  void feedToListBox();
  void closeDialog();
  void refresh();
  void swapSides();
  void loadFromProfile( QString );
  void saveToProfile( QString );
  
protected slots:
  void reject();
  void addFile( SynchronizerFileItem * );
  void markChanged( SynchronizerFileItem *, bool );
  void setScrolling( bool );
  void statusInfo( QString );
  void subdirsChecked( bool );
  void setPanelLabels();
  void setCompletion();
  void checkExcludeURLValidity( QString &text, QString &error );
  void connectFilters( const QString & );  

private:
  void initGUI(QWidget* parent, QString profile, KURL leftURL, KURL rightURL, QStringList selList);
  
  QString convertTime(time_t time) const;
  void    setMarkFlags();
  void    disableMarkButtons();
  void    enableMarkButtons();
  void    copyToClipboard( bool isLeft );
  
  int     convertToSeconds( int time, int unit );
  void    convertFromSeconds( int &time, int &unit, int second );

protected:
  virtual void keyPressEvent( QKeyEvent * );
  virtual void resizeEvent( QResizeEvent *e );
  
  ProfileManager *profileManager;
  FilterTabs     *filterTabs;
  GeneralFilter  *generalFilter;
  
  QTabWidget    *synchronizerTabs;  
  
  KHistoryCombo *leftLocation;
  KHistoryCombo *rightLocation;
  KHistoryCombo *fileFilter;
  
  QListView     *syncList;
  Synchronizer   synchronizer;
  
  QCheckBox     *cbSubdirs;
  QCheckBox     *cbSymlinks;
  QCheckBox     *cbByContent;
  QCheckBox     *cbIgnoreDate;
  QCheckBox     *cbAsymmetric;
  QCheckBox     *cbIgnoreCase;
  
  QPushButton   *btnSwapSides;
  QPushButton   *btnCompareDirs;
  QPushButton   *btnStopComparing;
  QPushButton   *btnSynchronize;
  QPushButton   *btnFeedToListBox;
  QPushButton   *btnScrollResults;
  
  QPushButton   *btnLeftToRight;
  QPushButton   *btnEquals;
  QPushButton   *btnDifferents;
  QPushButton   *btnRightToLeft;
  QPushButton   *btnDeletable;
  QPushButton   *btnDuplicates;
  QPushButton   *btnSingles;

  QLabel        *statusLabel;
  QLabel        *leftDirLabel;
  QLabel        *rightDirLabel;
  
  QStringList    selectedFiles;
  
  QSpinBox      *parallelThreadsSpinBox;
  QSpinBox      *equalitySpinBox;
  QComboBox     *equalityUnitCombo;
  QSpinBox      *timeShiftSpinBox;
  QComboBox     *timeShiftUnitCombo;
  QCheckBox     *ignoreHiddenFilesCB;

private:
  QPixmap        fileIcon;
  QPixmap        folderIcon;
  bool           isComparing;
  bool           wasClosed;
  bool           wasSync;
  bool           firstResize;
  bool           hasSelectedFiles;
  SyncViewItem  *lastItem;
  
  int            sizeX;
  int            sizeY;
  
  QColor         foreGrounds[ TT_MAX ];
  QColor         backGrounds[ TT_MAX ];
};

#endif /* __SYNCHRONIZERGUI_H__ */
