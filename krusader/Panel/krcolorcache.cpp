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
#include <qfile.h> 


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

QColor KrColorCache::dimColor(const QColor & color, int dim, const QColor & targetColor)
{
   return QColor((targetColor.red() * (100 - dim) + color.red() * dim) / 100, 
		(targetColor.green() * (100 - dim) + color.green() * dim) / 100, 
		(targetColor.blue() * (100 - dim) + color.blue() * dim) / 100);
}

void KrColorCache::setColor(const QString & colorName, QColor * color, bool isActive, bool isBackgroundColor)
{
   krConfig->setGroup("Colors");
   int dimFactor = krConfig->readNumEntry("Dim Factor", 100);
   QColor defaultColor = QColor(255, 255, 255);
   QColor targetColor = krConfig->readColorEntry("Dim Target Color", & defaultColor);
   bool dimBackground = krConfig->readBoolEntry("Dim Inactive Colors", false);
   bool dim = dimFactor >= 0 && dimFactor < 100 && !isActive;
   if (dim && isBackgroundColor && !dimBackground)
      dim = false;
   if (dim)
      *color = dimColor(* color, dimFactor, targetColor);
   colorCache.replace(colorName, color);
   if (colorCache.count() >= colorCache.size())
      colorCache.resize(colorCache.count()*2+1);
}

QColor KrColorCache::getColor(const QString & colorName) const
{
    krConfig->setGroup("Colors");
    if( colorName.startsWith( "Inactive " ) && krConfig->readBoolEntry("Dim Inactive Colors", false) ) {
       QString dimmedColorName = colorName.mid( 9 ); // remove the Inactive prefix
       return krConfig->readColorEntry(dimmedColorName);
    }
    return krConfig->readColorEntry(colorName);
}


// Macro: set target = col, if col is valid
#define SETCOLOR(target, col) { if (col.isValid()) target = col; }

QColor KrColorCache::getForegroundColor_Int(bool isActive) const
{
   QColor color = KGlobalSettings::textColor();
   SETCOLOR(color, getColor("Foreground"))
   if (!isActive) SETCOLOR(color, getColor("Inactive Foreground"))
   return color;
}

QColor KrColorCache::getForegroundColor(bool isActive) const
{
   QString colorName = isActive?"Foreground":"Inactive Foreground";
   QColor * color = colorCache.find(colorName);
   if (!color)
   {
      color = new QColor();
      *color = getForegroundColor_Int(isActive);
      ((KrColorCache *)this)->setColor(colorName, color, isActive);
   }
   return * color;
}

QColor KrColorCache::getDirectoryForegroundColor_Int(bool isActive) const
{
   if (!isActive && getTextValue("Inactive Directory Foreground") == "Inactive Foreground")
      return getForegroundColor_Int(false);
   QColor color = getColor("Directory Foreground");
   if (!isActive) SETCOLOR(color, getColor("Inactive Directory Foreground"))
   if (!color.isValid())
      return getForegroundColor_Int(isActive);
   return color;
}

QColor KrColorCache::getDirectoryForegroundColor(bool isActive) const
{
   QString colorName = isActive?"Directory Foreground":"Inactive Directory Foreground";
   QColor * color = colorCache.find(colorName);
   if (!color)
   {
      color = new QColor();
      *color = getDirectoryForegroundColor_Int(isActive);
      ((KrColorCache *)this)->setColor(colorName, color, isActive);
   }
   return * color;
}

QColor KrColorCache::getExecutableForegroundColor_Int(bool isActive) const
{
   if (!isActive && getTextValue("Inactive Executable Foreground") == "Inactive Foreground")
      return getForegroundColor_Int(false);
   QColor color = getColor("Executable Foreground");
   if (!isActive) SETCOLOR(color, getColor("Inactive Executable Foreground"))
   if (!color.isValid())
      return getForegroundColor_Int(isActive);
   return color;
}

QColor KrColorCache::getExecutableForegroundColor(bool isActive) const
{
   QString colorName = isActive?"Executable Foreground":"Inactive Executable Foreground";
   QColor * color = colorCache.find(colorName);
   if (!color)
   {
      color = new QColor();
      *color = getExecutableForegroundColor_Int(isActive);
      ((KrColorCache *)this)->setColor(colorName, color, isActive);
   }
   return * color;
}

QColor KrColorCache::getSymlinkForegroundColor_Int(bool isActive) const
{
   if (!isActive && getTextValue("Inactive Symlink Foreground") == "Inactive Foreground")
      return getForegroundColor_Int(false);
   QColor color = getColor("Symlink Foreground");
   if (!isActive) SETCOLOR(color, getColor("Inactive Symlink Foreground"))
   if (!color.isValid())
      return getForegroundColor_Int(isActive);
   return color;
}

QColor KrColorCache::getSymlinkForegroundColor(bool isActive) const
{
   QString colorName = isActive?"Symlink Foreground":"Inactive Symlink Foreground";
   QColor * color = colorCache.find(colorName);
   if (!color)
   {
      color = new QColor();
      *color = getSymlinkForegroundColor_Int(isActive);
      ((KrColorCache *)this)->setColor(colorName, color, isActive);
   }
   return * color;
}

QColor KrColorCache::getInvalidSymlinkForegroundColor_Int(bool isActive) const
{
   if (!isActive && getTextValue("Inactive Invalid Symlink Foreground") == "Inactive Foreground")
      return getForegroundColor_Int(false);
   QColor color = getColor("Invalid Symlink Foreground");
   if (!isActive) SETCOLOR(color, getColor("Inactive Invalid Symlink Foreground"))
   if (!color.isValid())
      return getForegroundColor_Int(isActive);
   return color;
}

QColor KrColorCache::getInvalidSymlinkForegroundColor(bool isActive) const
{
   QString colorName = isActive?"Invalid Symlink Foreground":"Inactive Invalid Symlink Foreground";
   QColor * color = colorCache.find(colorName);
   if (!color)
   {
      color = new QColor();
      *color = getInvalidSymlinkForegroundColor_Int(isActive);
      ((KrColorCache *)this)->setColor(colorName, color, isActive);
   }
   return * color;
}

QColor KrColorCache::getMarkedForegroundColor_Int(bool isActive) const
{
   QString colorName = isActive?"Marked Foreground":"Inactive Marked Foreground";
   if (getTextValue(colorName) == "transparent")
       return QColor();
   if (isActive && getTextValue(colorName) == "")
      return KGlobalSettings::highlightedTextColor();
   if (!isActive && getTextValue(colorName) == "")
      return getMarkedForegroundColor_Int(true);
   return getColor(colorName);
}

QColor KrColorCache::getMarkedForegroundColor(bool isActive) const
{
   QString colorName = isActive?"Marked Foreground":"Inactive Marked Foreground";
   QColor * color = colorCache.find(colorName);
   if (!color)
   {
      color = new QColor();
      *color = getMarkedForegroundColor_Int(isActive);
      ((KrColorCache *)this)->setColor(colorName, color, isActive);
   }
   return * color;
}

QColor KrColorCache::getMarkedBackgroundColor_Int(bool isActive) const
{
   if (isActive && getTextValue("Marked Background") == "")
      return KGlobalSettings::highlightColor();
   if (isActive && getTextValue("Marked Background") == "Background")
      return getBackgroundColor_Int(true);
   if (!isActive && getTextValue("Inactive Marked Background") == "")
      return getMarkedBackgroundColor_Int(true);
   if (!isActive && getTextValue("Inactive Marked Background") == "Inactive Background")
      return getBackgroundColor_Int(false);
   return isActive?KrColorCache::getColor("Marked Background"):KrColorCache::getColor("Inactive Marked Background");
}

QColor KrColorCache::getMarkedBackgroundColor(bool isActive) const
{
   QString colorName = isActive?"Marked Background":"Inactive Marked Background";
   QColor * color = colorCache.find(colorName);
   if (!color)
   {
      color = new QColor();
      *color = getMarkedBackgroundColor_Int(isActive);
      ((KrColorCache *)this)->setColor(colorName, color, isActive, true);
   }
   return * color;
}

QColor KrColorCache::getCurrentForegroundColor_Int(bool isActive) const
{
   QColor color = getColor("Current Foreground");
   if (!isActive) SETCOLOR(color, getColor("Inactive Current Foreground"))
   return color;
}

QColor KrColorCache::getCurrentForegroundColor(bool isActive) const
{
   QString colorName = isActive?"Current Foreground":"Inactive Current Foreground";
   QColor * color = colorCache.find(colorName);
   if (!color)
   {
      color = new QColor();
      *color = getCurrentForegroundColor_Int(isActive);
      ((KrColorCache *)this)->setColor(colorName, color, isActive);
   }
   return * color;
}

QColor KrColorCache::getCurrentBackgroundColor_Int(bool isActive) const
{
   if (isActive && getTextValue("Current Background") == "")
      return QColor();
   if (isActive && getTextValue("Current Background") == "Background")
      return getBackgroundColor_Int(true);
   if (!isActive && getTextValue("Inactive Current Background") == "")
      return getCurrentBackgroundColor_Int(true);
   if (!isActive && getTextValue("Inactive Current Background") == "Inactive Background")
      return getBackgroundColor_Int(false);
   return isActive?getColor("Current Background"):getColor("Inactive Current Background");
}

QColor KrColorCache::getCurrentBackgroundColor(bool isActive) const
{
   QString colorName = isActive?"Current Background":"Inactive Current Background";
   QColor * color = colorCache.find(colorName);
   if (!color)
   {
      color = new QColor();
      *color = getCurrentBackgroundColor_Int(isActive);
      ((KrColorCache *)this)->setColor(colorName, color, isActive, true);
   }
   return * color;
}

QColor KrColorCache::getCurrentMarkedForegroundColor_Int(bool isActive) const
{
   QString colorName = isActive?"Marked Current Foreground":"Inactive Marked Current Foreground";
   if (isActive && getTextValue(colorName) == "")
       return QColor();
   if (isActive && getTextValue(colorName) == "Marked Foreground")
       return getMarkedForegroundColor_Int(true);
   if (!isActive && getTextValue(colorName) == "")
       return getCurrentMarkedForegroundColor_Int(true);
   if (!isActive && getTextValue(colorName) == "Inactive Marked Foreground")
       return getMarkedForegroundColor_Int(false);
   return getColor(colorName);
}

QColor KrColorCache::getCurrentMarkedForegroundColor(bool isActive) const
{
   QString colorName = isActive?"Marked Current Foreground":"Inactive Marked Current Foreground";
   QColor * color = colorCache.find(colorName);
   if (!color)
   {
      color = new QColor();
      *color = getCurrentMarkedForegroundColor_Int(isActive);
      ((KrColorCache *)this)->setColor(colorName, color, isActive);
   }
   return * color;
}

QColor KrColorCache::getBackgroundColor_Int(bool isActive) const
{
   QColor color = KGlobalSettings::baseColor();
   SETCOLOR(color, getColor("Background"))
   if (!isActive) SETCOLOR(color, getColor("Inactive Background"))
   return color;
}

QColor KrColorCache::getBackgroundColor(bool isActive) const
{
   QString colorName = isActive?"Background":"Inactive Background";
   QColor * color = colorCache.find(colorName);
   if (!color)
   {
      color = new QColor();
      *color = getBackgroundColor_Int(isActive);
      ((KrColorCache *)this)->setColor(colorName, color, isActive, true);
   }
   return * color;
}

QColor KrColorCache::getAlternateBackgroundColor_Int(bool isActive) const
{
   if (isActive && getTextValue("Alternate Background") == "Background")
      return getBackgroundColor_Int(true);
   if (!isActive && getTextValue("Inactive Alternate Background") == "")
      return getAlternateBackgroundColor_Int(true);
   if (!isActive && getTextValue("Inactive Alternate Background") == "Inactive Background")
      return getBackgroundColor_Int(false);
   QColor color = isActive?getColor("Alternate Background"):getColor("Inactive Alternate Background");
   if (!color.isValid())
      color = KGlobalSettings::alternateBackgroundColor();
   if (!color.isValid())
      color = KGlobalSettings::baseColor();
   return color;
}

QColor KrColorCache::getAlternateBackgroundColor(bool isActive) const
{
   QString colorName = isActive?"Alternate Background":"Inactive Alternate Background";
   QColor * color = colorCache.find(colorName);
   if (!color)
   {
      color = new QColor();
      *color = getAlternateBackgroundColor_Int(isActive);
      ((KrColorCache *)this)->setColor(colorName, color, isActive, true);
   }
   return * color;
}

QColor KrColorCache::getAlternateMarkedBackgroundColor_Int(bool isActive) const
{
   if (isActive && getTextValue("Alternate Marked Background") == "Alternate Background")
      return getAlternateBackgroundColor_Int(true);
   if (isActive && getTextValue("Alternate Marked Background") == "")
      return getMarkedBackgroundColor_Int(true);
   if (!isActive && getTextValue("Inactive Alternate Marked Background") == "")
      return getAlternateMarkedBackgroundColor_Int(true);
   if (!isActive && getTextValue("Inactive Alternate Marked Background") == "Inactive Alternate Background")
      return getAlternateBackgroundColor_Int(false);
   if (!isActive && getTextValue("Inactive Alternate Marked Background") == "Inactive Marked Background")
      return getMarkedBackgroundColor_Int(false);
   return isActive?KrColorCache::getColor("Alternate Marked Background"):KrColorCache::getColor("Inactive Alternate Marked Background");
}

QColor KrColorCache::getAlternateMarkedBackgroundColor(bool isActive) const
{
   QString colorName = isActive?"Alternate Marked Background":"Inactive Alternate Marked Background";
   QColor * color = colorCache.find(colorName);
   if (!color)
   {
      color = new QColor();
      *color = getAlternateMarkedBackgroundColor_Int(isActive);
      ((KrColorCache *)this)->setColor(colorName, color, isActive, true);
   }
   return * color;
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

#include "krcolorcache.moc"
