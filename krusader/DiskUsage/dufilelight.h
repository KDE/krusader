/***************************************************************************
                         dufilelight.h  -  description
                             -------------------
    copyright            : (C) 2004 by Csaba Karai
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

#ifndef __DU_FILELIGHT_H__
#define __DU_FILELIGHT_H__

#include "diskusage.h"
#include "radialMap/widget.h"
#include "filelightParts/Config.h"

class DUFilelight : public RadialMap::Widget
{
  Q_OBJECT
  
public:
  DUFilelight( DiskUsage *usage, const char *name );

public slots:
  void slotDirChanged( Directory * );
  void clear();
  void slotChanged( File * );
  void slotRefresh();

protected slots:  
  void slotActivated( const KURL& );
  
  void schemeRainbow()        { setScheme( Filelight::Rainbow ); }
  void schemeHighContrast()   { setScheme( Filelight::HighContrast ); }
  void schemeKDE()            { setScheme( Filelight::KDE ); }
  
  void increaseContrast();
  void decreaseContrast();
  void changeAntiAlias();
  void showSmallFiles();
  void varyLabelFontSizes();
  void minFontSize();
  
protected:
  virtual void mousePressEvent( QMouseEvent* );
  
  void setScheme( Filelight::MapScheme );

  DiskUsage *diskUsage;  
  Directory *currentDir;
  
private:
  bool refreshNeeded;
};

#endif /* __DU_FILELIGHT_H__ */

