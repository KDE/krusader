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

const QString & KrColorCache::getTextValue(const QString & textName) const
{
   QString * text = textCache.find(textName);
   if (!text)
   {
      krConfig->setGroup("Colors");
      text = new QString("");
      *text = krConfig->readEntry(textName, "");
      ((KrColorCache *)this)->textCache.replace(textName, text);
      if (textCache.count() >= textCache.size())
         ((KrColorCache *)this)->textCache.resize(textCache.count()*2+1);
   }
   return *text;
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

const QColor * KrColorCache::getForegroundColor(bool isActive) const
{
   const QColor * color = isActive?0:getColor("Inactive Foreground");
   return color?color:getColor("Foreground");
}

const QColor * KrColorCache::getDirectoryForegroundColor(bool isActive) const
{
   const QColor * color = isActive?0:getColor("Inactive Directory Foreground");
   return color?color:getColor("Directory Foreground");
}

const QColor * KrColorCache::getExecutableForegroundColor(bool isActive) const
{
   const QColor * color = isActive?0:getColor("Inactive Executable Foreground");
   return color?color:getColor("Executable Foreground");
}

const QColor * KrColorCache::getSymlinkForegroundColor(bool isActive) const
{
   const QColor * color = isActive?0:getColor("Inactive Symlink Foreground");
   return color?color:getColor("Symlink Foreground");
}

const QColor * KrColorCache::getInvalidSymlinkForegroundColor(bool isActive) const
{
   const QColor * color = isActive?0:getColor("Inactive Invalid Symlink Foreground");
   return color?color:getColor("Invalid Symlink Foreground");
}

const QColor * KrColorCache::getMarkedForegroundColor(bool isActive) const
{
   const QColor * color = isActive?0:getColor("Inactive Marked Foreground");
   return color?color:getColor("Marked Foreground");
}

const QColor * KrColorCache::getMarkedBackgroundColor(bool isActive) const
{
   const QColor * color = isActive?0:getColor("Inactive Marked Background");
   return color?color:getColor("Marked Background");
}

const QColor * KrColorCache::getCurrentForegroundColor(bool isActive) const
{
   const QColor * color = isActive?0:getColor("Inactive Current Foreground");
   return color?color:getColor("Current Foreground");
}

const QColor * KrColorCache::getCurrentBackgroundColor(bool isActive) const
{
   const QColor * color = isActive?0:getColor("Inactive Current Background");
   return color?color:getColor("Current Background");
}

const QColor * KrColorCache::getBackgroundColor(bool isActive) const
{
   const QColor * color = isActive?0:getColor("Inactive Background");
   return color?color:getColor("Background");
}

const QColor * KrColorCache::getAlternateBackgroundColor(bool isActive) const
{
   const QColor * color = isActive?0:getColor("Inactive Alternate Background");
   return color?color:getColor("Alternate Background");
}

const QColor * KrColorCache::getAlternateMarkedBackgroundColor(bool isActive) const
{
   const QColor * color = isActive?0:getColor("Inactive Alternate Marked Background");
   return color?color:getColor("Alternate Marked Background");
}

void KrColorCache::refreshColors()
{
   colorCache.clear();
   textCache.clear();
   krConfig->setGroup("Colors");
   kdeDefault = krConfig->readBoolEntry("KDE Default", _KDEDefaultColors);
   alternateBackgroundEnabled = krConfig->readBoolEntry("Enable Alternate Background", _AlternateBackground);
   showCurrentItemAlways = krConfig->readBoolEntry("Show Current Item Always", _ShowCurrentItemAlways);
   colorsRefreshed();
}
