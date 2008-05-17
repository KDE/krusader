/***************************************************************************
                          mediabutton.h  -  description
                             -------------------
    copyright            : (C) 2006 + by Csaba Karai
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

#ifndef MEDIABUTTON_H
#define MEDIABUTTON_H

#include <qwidget.h>
#include <qtoolbutton.h>
#include <QEvent>
#include <QMenu>
#include <kurl.h>
#include <kio/jobclasses.h>
#include <qlist.h>
#include <kmountpoint.h>
#include <qtimer.h>
#include <qmap.h>

/**
  *@author Csaba Karai
  */

class QMenu;
class KMountPoint;

class MediaButton : public QToolButton  {
   Q_OBJECT
public: 
  MediaButton(QWidget *parent=0);
  ~MediaButton();

  QString detectType( KMountPoint *mp );

public slots:
  void slotAboutToShow();
  void slotAboutToHide();
  void slotTimeout();
  void slotPopupActivated( QAction * );
  void gettingSpaceData(const QString &mountPoint, quint64 kBSize, quint64 kBUsed, quint64 kBAvail);
  void showMenu();
  void slotEntries( KIO::Job*, const KIO::UDSEntryList& );
  void slotListResult( KJob* );

signals:
  void openUrl(const KUrl&);
  void aboutToShow();

protected:
  bool eventFilter( QObject *o, QEvent *e );

private:
  void createListWithMedia();
  void createListWithoutMedia();

  KUrl getLocalPath( const KUrl &, KMountPoint::List * list = 0 );
  bool mount( int );
  bool umount( int );
  bool eject( int );

  void rightClickMenu( int );

  void addMountPoint( KMountPoint *mp, bool isMounted );

  QMenu *popupMenu;
  QMenu *rightMenu;

  bool        hasMedia;
  bool        busy;

  int         waitingForMount;
  bool        newTabAfterMount;
  int         maxMountWait;

  QList<KUrl>    urls;
  QList<KUrl>    mediaUrls;
  QList<QString> iconNames;
  QList<bool>    quasiMounted;

  QString extraSpaces;  //prevents from increasing the size of the widget

  QTimer      mountCheckerTimer;

  QMap<int,QAction *>  idActionMap;
};

#endif /* MEDIABUTTON_H */
