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
#include <kglobalsettings.h> 


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

const QColor & KrColorCache::getColor(const QString & colorName) const
{
   QColor * color = colorCache.find(colorName);
   if (!color)
   {
      color = new QColor();
      krConfig->setGroup("Colors");
      *color = krConfig->readColorEntry(colorName);
      ((KrColorCache *)this)->colorCache.replace(colorName, color);
      if (colorCache.count() >= colorCache.size())
         ((KrColorCache *)this)->colorCache.resize(colorCache.count()*2+1);
   }
   return * color;
}


// Macro: set target = col, if col is valid
#define SETCOLOR(target, col) { if (col.isValid()) target = col; }

QColor KrColorCache::getForegroundColor(bool isActive) const
{
   QColor color = KGlobalSettings::textColor();
   SETCOLOR(color, getColor("Foreground"))
   if (!isActive) SETCOLOR(color, getColor("Inactive Foreground"))
   return color;
}

QColor KrColorCache::getDirectoryForegroundColor(bool isActive) const
{
   if (!isActive && getTextValue("Inactive Directory Background") == "Inactive Foreground")
      return getForegroundColor(false);
   QColor color = getColor("Directory Foreground");
   if (!isActive) SETCOLOR(color, getColor("Inactive Directory Foreground"))
   if (!color.isValid())
      return getForegroundColor(isActive);
   return color;
}

QColor KrColorCache::getExecutableForegroundColor(bool isActive) const
{
   if (!isActive && getTextValue("Inactive Executable Background") == "Inactive Foreground")
      return getForegroundColor(false);
   QColor color = getColor("Executable Foreground");
   if (!isActive) SETCOLOR(color, getColor("Inactive Executable Foreground"))
   if (!color.isValid())
      return getForegroundColor(isActive);
   return color;
}

QColor KrColorCache::getSymlinkForegroundColor(bool isActive) const
{
   if (!isActive && getTextValue("Inactive Symlink Background") == "Inactive Foreground")
      return getForegroundColor(false);
   QColor color = getColor("Symlink Foreground");
   if (!isActive) SETCOLOR(color, getColor("Inactive Symlink Foreground"))
   if (!color.isValid())
      return getForegroundColor(isActive);
   return color;
}

QColor KrColorCache::getInvalidSymlinkForegroundColor(bool isActive) const
{
   if (!isActive && getTextValue("Inactive Invalid Symlink Background") == "Inactive Foreground")
      return getForegroundColor(false);
   QColor color = getColor("Invalid Symlink Foreground");
   if (!isActive) SETCOLOR(color, getColor("Inactive Invalid Symlink Foreground"))
   if (!color.isValid())
      return getForegroundColor(isActive);
   return color;
}

QColor KrColorCache::getMarkedForegroundColor(bool isActive) const
{
   QString colorName = isActive?"Marked Foreground":"Inactive Marked Foreground";
   if (getTextValue(colorName) == "transparent")
       return QColor();
   if (isActive && getTextValue(colorName) == "")
      return KGlobalSettings::highlightedTextColor();
   if (!isActive && getTextValue(colorName) == "")
      return getMarkedForegroundColor(true);
   return getColor(colorName);
}

QColor KrColorCache::getMarkedBackgroundColor(bool isActive) const
{
   if (isActive && getTextValue("Marked Background") == "")
      return KGlobalSettings::highlightColor();
   if (isActive && getTextValue("Marked Background") == "Background")
      return getBackgroundColor(true);
   if (!isActive && getTextValue("Inactive Marked Background") == "")
      return getMarkedBackgroundColor(true);
   if (!isActive && getTextValue("Inactive Marked Background") == "Inactive Background")
      return getBackgroundColor(false);
   return isActive?KrColorCache::getColor("Marked Background"):KrColorCache::getColor("Inactive Marked Background");
}

QColor KrColorCache::getCurrentForegroundColor(bool isActive) const
{
   QColor color = getColor("Current Foreground");
   if (!isActive) SETCOLOR(color, getColor("Inactive Current Foreground"))
   return color;
}

QColor KrColorCache::getCurrentBackgroundColor(bool isActive) const
{
   if (isActive && getTextValue("Current Background") == "")
      return QColor();
   if (isActive && getTextValue("Current Background") == "Background")
      return getBackgroundColor(true);
   if (!isActive && getTextValue("Inactive Current Background") == "")
      return getCurrentBackgroundColor(true);
   if (!isActive && getTextValue("Inactive Current Background") == "Inactive Background")
      return getBackgroundColor(false);
   return isActive?getColor("Current Background"):getColor("Inactive Current Background");
}

QColor KrColorCache::getCurrentMarkedForegroundColor(bool isActive) const
{
   QString colorName = isActive?"Marked Current Foreground":"Inactive Marked Current Foreground";
   if (getTextValue(colorName) == "")
       return QColor();
   return getColor(colorName);
}

QColor KrColorCache::getBackgroundColor(bool isActive) const
{
   QColor color = KGlobalSettings::baseColor();
   SETCOLOR(color, getColor("Background"))
   if (!isActive) SETCOLOR(color, getColor("Inactive Background"))
   return color;
}

QColor KrColorCache::getAlternateBackgroundColor(bool isActive) const
{
   if (isActive && getTextValue("Alternate Background") == "Background")
      return getBackgroundColor(true);
   if (!isActive && getTextValue("Inactive Alternate Background") == "")
      return getAlternateBackgroundColor(true);
   if (!isActive && getTextValue("Inactive Alternate Background") == "Inactive Background")
      return getBackgroundColor(false);
   QColor color = isActive?getColor("Alternate Background"):getColor("Inactive Alternate Background");
   if (!color.isValid())
      color = KGlobalSettings::alternateBackgroundColor();
   if (!color.isValid())
      color = KGlobalSettings::baseColor();
   return color;
}

QColor KrColorCache::getAlternateMarkedBackgroundColor(bool isActive) const
{
   if (isActive && getTextValue("Alternate Marked Background") == "Alternate Background")
      return getAlternateBackgroundColor(true);
   if (isActive && getTextValue("Alternate Marked Background") == "")
      return getMarkedBackgroundColor(true);
   if (!isActive && getTextValue("Inactive Alternate Marked Background") == "")
      return getAlternateMarkedBackgroundColor(true);
   if (!isActive && getTextValue("Inactive Alternate Marked Background") == "Inactive Alternate Background")
      return getAlternateBackgroundColor(false);
   if (!isActive && getTextValue("Inactive Alternate Marked Background") == "Inactive Marked Background")
      return getMarkedBackgroundColor(false);
   return isActive?KrColorCache::getColor("Alternate Marked Background"):KrColorCache::getColor("Inactive Alternate Marked Background");
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

