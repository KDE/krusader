/***************************************************************************
                                krviewitem.h
                             -------------------
    copyright            : (C) 2000-2002 by Shie Erlich & Rafi Yanai
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
#ifndef KRVIEWITEM_H
#define KRVIEWITEM_H

#include <sys/types.h>
#include <kio/global.h>

class QString;
class QPixmap;

class KrViewItem {
public:
  KrViewItem() {}
  virtual ~KrViewItem() {}
  virtual QString name() const = 0;
  virtual bool isDir() const = 0;
  virtual bool isExecutable() const = 0;
  virtual KIO::filesize_t size() const = 0;
  virtual QString dateTime() const = 0;
  virtual time_t getTime_t() const = 0;
  virtual QString mime() const = 0;
  virtual QString symlinkDest() const = 0;
  virtual bool isSymLink() const = 0;
  virtual QString description() const = 0;
  virtual bool isSelected() const = 0;
  virtual void setSelected(bool s) = 0;
  virtual QPixmap& icon() = 0;
};

#endif
