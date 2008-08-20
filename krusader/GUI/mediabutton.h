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
#include <qlist.h>
#include <qmap.h>
#include <solid/device.h>
#include <solid/solidnamespace.h>
#include <qtimer.h>

/**
  *@author Csaba Karai
  */

class QMenu;
class KMountPoint;
class KIcon;

class MediaButton : public QToolButton  {
   Q_OBJECT
public: 
  MediaButton(QWidget *parent=0);
  ~MediaButton();

public slots:
  void slotAboutToShow();
  void slotAboutToHide();
  void slotPopupActivated( QAction * );
  void slotAccessibilityChanged(bool, const QString &);
  void slotDeviceAdded(const QString&);
  void slotDeviceRemoved(const QString&);
  void showMenu();
  void slotTimeout();

signals:
  void openUrl(const KUrl&);
  void aboutToShow();

protected:
  bool eventFilter( QObject *o, QEvent *e );
  bool getNameAndIcon( Solid::Device &, QString &, KIcon &);

private:
  void createMediaList();

  QList<Solid::Device> storageDevices;

  void mount( QString, bool open=false, bool newtab = false );
  void umount( QString );
  void eject( QString );

  void rightClickMenu( QString );

private slots:
  void slotSetupDone(Solid::ErrorType error, QVariant errorData, const QString &udi);
  void slotTeardownDone(Solid::ErrorType error, QVariant errorData, const QString &udi);

private:
  QMenu  *popupMenu;
  QMenu  *rightMenu;
  QString udiToOpen;
  bool    openInNewTab;
  QMap<QString, QString> udiNameMap;
  QTimer      mountCheckerTimer;
};

#endif /* MEDIABUTTON_H */
