/***************************************************************************
                   krcolorcache.cpp
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

                                          S o u r c e    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
#include "krcolorcache.h"
#include "../krusader.h"
#include "../defaults.h"


KrColorCache * KrColorCache::instance = 0;

KrColorCache::KrColorCache() : colorCache(23)
{
   colorCache.setAutoDelete(true);
}

KrColorCache & KrColorCache::getColorCache()
{
   if (!instance)
   {
      instance = new KrColorCache();
      instance->refreshColors();
   }
   return *instance;
}

const QColor * KrColorCache::getColor(const QString & colorName) const
{
   QColor * color = colorCache.find(colorName);
   if (!color)
   {
      krConfig->setGroup("Colors");
      color = new QColor();
      *color = krConfig->readColorEntry(colorName);
      ((KrColorCache *)this)->colorCache.replace(colorName, color);
      if (colorCache.count() >= colorCache.size())
         ((KrColorCache *)this)->colorCache.resize(colorCache.count()*2+1);
   }
   if (color->isValid())
      return color;
   return 0;
}

const QColor * KrColorCache::getForegroundColor() const
{
   return getColor("Foreground");
}

const QColor * KrColorCache::getDirectoryForegroundColor() const
{
   return getColor("Directory Foreground");
}

const QColor * KrColorCache::getExecutableForegroundColor() const
{
   return getColor("Executable Foreground");
}

const QColor * KrColorCache::getSymlinkForegroundColor() const
{
   return getColor("Symlink Foreground");
}

const QColor * KrColorCache::getInvalidSymlinkForegroundColor() const
{
   return getColor("Invalid Symlink Foreground");
}

const QColor * KrColorCache::getMarkedForegroundColor() const
{
   return getColor("Marked Foreground");
}

const QColor * KrColorCache::getMarkedBackgroundColor() const
{
   return getColor("Marked Background");
}

const QColor * KrColorCache::getCurrentForegroundColor() const
{
   return getColor("Current Foreground");
}

const QColor * KrColorCache::getCurrentBackgroundColor() const
{
   return getColor("Current Background");
}

const QColor * KrColorCache::getBackgroundColor() const
{
   return getColor("Background");
}

const QColor * KrColorCache::getAlternateBackgroundColor() const
{
   return getColor("Alternate Background");
}

const QColor * KrColorCache::getAlternateMarkedBackgroundColor() const
{
   return getColor("Alternate Marked Background");
}

void KrColorCache::refreshColors()
{
   colorCache.clear();
   krConfig->setGroup("Colors");
   kdeDefault = krConfig->readBoolEntry("KDE Default", _KDEDefaultColors);
   alternateBackgroundEnabled = krConfig->readBoolEntry("Enable Alternate Background", _AlternateBackground);
   showCurrentItemAlways = krConfig->readBoolEntry("Show Current Item Always", _ShowCurrentItemAlways);
   colorsRefreshed();
}
