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
//Added by qt3to4:
#include <Q3ValueList>


// Macro: set target = col, if col is valid
#define SETCOLOR(target, col) { if (col.isValid()) target = col; }

/*
Static class, which lists all allowed keywords for a quick access. Just call a method to initialize it. 
*/
class KrColorSettingNames
{
	static QMap<QString, bool> s_colorNames;
	static QMap<QString, bool> s_numNames;
	static QMap<QString, bool> s_boolNames;
	static void initialize();
public:
	static QStringList getColorNames();
	static bool isColorNameValid(const QString & settingName);
	static QStringList getNumNames();
	static bool isNumNameValid(const QString & settingName);
	static QStringList getBoolNames();
	static bool isBoolNameValid(const QString & settingName);
} krColorSettingNames;

QMap<QString, bool> KrColorSettingNames::s_colorNames;
QMap<QString, bool> KrColorSettingNames::s_numNames;
QMap<QString, bool> KrColorSettingNames::s_boolNames;

void KrColorSettingNames::initialize()
{
	if (!s_colorNames.empty())
		return;
	s_colorNames["Foreground"] = true;
	s_colorNames["Inactive Foreground"] = true;
	s_colorNames["Directory Foreground"] = true;
	s_colorNames["Inactive Directory Foreground"] = true;
	s_colorNames["Executable Foreground"] = true;
	s_colorNames["Inactive Executable Foreground"] = true;
	s_colorNames["Symlink Foreground"] = true;
	s_colorNames["Inactive Symlink Foreground"] = true;
	s_colorNames["Invalid Symlink Foreground"] = true;
	s_colorNames["Inactive Invalid Symlink Foreground"] = true;
	s_colorNames["Marked Foreground"] = true;
	s_colorNames["Inactive Marked Foreground"] = true;
	s_colorNames["Marked Background"] = true;
	s_colorNames["Inactive Marked Background"] = true;
	s_colorNames["Current Foreground"] = true;
	s_colorNames["Inactive Current Foreground"] = true;
	s_colorNames["Current Background"] = true;
	s_colorNames["Inactive Current Background"] = true;
	s_colorNames["Marked Current Foreground"] = true;
	s_colorNames["Inactive Marked Current Foreground"] = true;
	s_colorNames["Alternate Background"] = true;
	s_colorNames["Inactive Alternate Background"] = true;
	s_colorNames["Background"] = true;
	s_colorNames["Inactive Background"] = true;
	s_colorNames["Alternate Marked Background"] = true;
	s_colorNames["Inactive Alternate Marked Background"] = true;
	s_colorNames["Dim Target Color"] = true;

	s_numNames["Dim Factor"] = true;

	s_boolNames["KDE Default"] = true;
	s_boolNames["Enable Alternate Background"] = true;
	s_boolNames["Show Current Item Always"] = true;
	s_boolNames["Dim Inactive Colors"] = true;
}

QStringList KrColorSettingNames::getColorNames()
{
	initialize();
	return s_colorNames.keys();
}

bool KrColorSettingNames::isColorNameValid(const QString & settingName)
{
	initialize();
	return s_colorNames.contains(settingName);
}

QStringList KrColorSettingNames::getNumNames()
{
	initialize();
	return s_numNames.keys();
}

bool KrColorSettingNames::isNumNameValid(const QString & settingName)
{
	initialize();
	return s_numNames.contains(settingName);
}

QStringList KrColorSettingNames::getBoolNames()
{
	initialize();
	return s_boolNames.keys();
}

bool KrColorSettingNames::isBoolNameValid(const QString & settingName)
{
	initialize();
	return s_boolNames.contains(settingName);
}



/*
KrColorSettings implementation. Contains all properties in QMaps. loadFromConfig initializes them from krConfig.
*/
class KrColorSettingsImpl
{
	friend class KrColorSettings;
	QMap<QString, QString> m_colorTextValues;
	QMap<QString, QColor> m_colorValues;
	QMap<QString, int> m_numValues;
	QMap<QString, bool> m_boolValues;
	void loadFromConfig();
};

void KrColorSettingsImpl::loadFromConfig()
{
	krConfig->setGroup("Colors");
	QStringList names = KrColorSettingNames::getColorNames();
	for ( QStringList::Iterator it = names.begin(); it != names.end(); ++it )
	{
		m_colorTextValues[*it] = krConfig->readEntry(*it, QString());
		m_colorValues[*it] = krConfig->readEntry(*it, QColor() );
	}
	names = KrColorSettingNames::getNumNames();
	for ( QStringList::Iterator it = names.begin(); it != names.end(); ++it )
	{
		if (krConfig->readEntry(*it, QString()) != QString())
			m_numValues[*it] = krConfig->readNumEntry(*it);
	}
	names = KrColorSettingNames::getBoolNames();
	for ( QStringList::Iterator it = names.begin(); it != names.end(); ++it )
	{
		if (krConfig->readEntry(*it, QString()) != QString())
			m_boolValues[*it] = krConfig->readBoolEntry(*it);
	}
}



KrColorSettings::KrColorSettings()
{
	m_impl = new KrColorSettingsImpl();
	m_impl->loadFromConfig();
}

KrColorSettings::KrColorSettings(const KrColorSettings & src)
{
	m_impl = new KrColorSettingsImpl();
	operator =(src);
}

KrColorSettings::~KrColorSettings()
{
	delete m_impl;
}

const KrColorSettings & KrColorSettings::operator= (const KrColorSettings & src)
{
	if (this == & src)
		return * this;
	QStringList names = KrColorSettingNames::getColorNames();
	for ( QStringList::Iterator it = names.begin(); it != names.end(); ++it )
	{
		m_impl->m_colorTextValues[*it] = src.m_impl->m_colorTextValues[*it];
		m_impl->m_colorValues[*it] = src.m_impl->m_colorValues[*it];
	}
	for ( QMap<QString, int>::Iterator it = src.m_impl->m_numValues.begin(); it != src.m_impl->m_numValues.end(); ++it )
	{
		m_impl->m_numValues[it.key()] = it.data();
	}
	for ( QMap<QString, bool>::Iterator it = src.m_impl->m_boolValues.begin(); it != src.m_impl->m_boolValues.end(); ++it )
	{
		m_impl->m_boolValues[it.key()] = it.data();
	}
	return * this;
}

Q3ValueList<QString> KrColorSettings::getColorNames()
{
	return KrColorSettingNames::getColorNames();
}

bool KrColorSettings::isColorNameValid(const QString & settingName)
{
	return KrColorSettingNames::isColorNameValid(settingName);
}

bool KrColorSettings::setColorValue(const QString & settingName, const QColor & color)
{
	if (!isColorNameValid(settingName))
	{
		krOut << "Invalid color setting name: " << settingName << endl;
		return false;
	}
	m_impl->m_colorValues[settingName] = color;
	return true;
}

QColor KrColorSettings::getColorValue(const QString & settingName) const
{
	if (!isColorNameValid(settingName))
	{
		krOut << "Invalid color setting name: " << settingName << endl;
		return QColor();
	}
	return m_impl->m_colorValues[settingName];
}

bool KrColorSettings::setColorTextValue(const QString & settingName, const QString & colorText)
{
	if (!isColorNameValid(settingName))
	{
		krOut << "Invalid color setting name: " << settingName << endl;
		return false;
	}
	m_impl->m_colorTextValues[settingName] = colorText;
	return true;
}

QString KrColorSettings::getColorTextValue(const QString & settingName) const
{
	if (!isColorNameValid(settingName))
	{
		krOut << "Invalid color setting name: " << settingName << endl;
		return QString();
	}
	return m_impl->m_colorTextValues[settingName];
}

Q3ValueList<QString> KrColorSettings::getNumNames()
{
	return KrColorSettingNames::getNumNames();
}

bool KrColorSettings::isNumNameValid(const QString & settingName)
{
	return KrColorSettingNames::isNumNameValid(settingName);
}

bool KrColorSettings::setNumValue(const QString & settingName, int value)
{
	if (!isNumNameValid(settingName))
	{
		krOut << "Invalid number setting name: " << settingName << endl;
		return false;
	}
	m_impl->m_numValues[settingName] = value;
	return true;
}

int KrColorSettings::getNumValue(const QString & settingName, int defaultValue) const
{
	if (!isNumNameValid(settingName))
	{
		krOut << "Invalid number setting name: " << settingName << endl;
		return 0;
	}
	if (!m_impl->m_numValues.contains(settingName))
		return defaultValue;
	return m_impl->m_numValues[settingName];
}

Q3ValueList<QString> KrColorSettings::getBoolNames()
{
	return KrColorSettingNames::getBoolNames();
}

bool KrColorSettings::isBoolNameValid(const QString & settingName)
{
	return KrColorSettingNames::isBoolNameValid(settingName);
}

bool KrColorSettings::setBoolValue(const QString & settingName, bool value)
{
	if (!isBoolNameValid(settingName))
	{
		krOut << "Invalid bool setting name: " << settingName << endl;
		return false;
	}
	m_impl->m_boolValues[settingName] = value;
	return true;
}

int KrColorSettings::getBoolValue(const QString & settingName, bool defaultValue) const
{
	if (!isBoolNameValid(settingName))
	{
		krOut << "Invalid bool setting name: " << settingName << endl;
		return false;
	}
	if (!m_impl->m_boolValues.contains(settingName))
		return defaultValue;
	return m_impl->m_boolValues[settingName];
}



KrColorItemType::KrColorItemType()
{
	m_fileType = File;
	m_alternateBackgroundColor = false;
	m_activePanel = false;
	m_currentItem = false;
	m_selectedItem = false;
}

KrColorItemType::KrColorItemType(FileType type, bool alternateBackgroundColor, bool activePanel, bool currentItem, bool selectedItem)
{
	m_fileType = type;
	m_alternateBackgroundColor = alternateBackgroundColor;
	m_activePanel = activePanel;
	m_currentItem = currentItem;
	m_selectedItem = selectedItem;
}

KrColorItemType::KrColorItemType(const KrColorItemType & src)
{
	operator= (src);
}

const KrColorItemType & KrColorItemType::operator= (const KrColorItemType & src)
{
	if (this == & src)
		return * this;
	m_fileType = src.m_fileType;
	m_alternateBackgroundColor = src.m_alternateBackgroundColor;
	m_activePanel = src.m_activePanel;
	m_currentItem = src.m_currentItem;
	m_selectedItem = src.m_selectedItem;
	return * this;
}




/*
KrColorCache implementation. Contains the KrColorSettings used for teh calculation and the cache for the results.
getColors is the only method to call. All other are taken from the previous versions.
*/
class KrColorCacheImpl
{
	friend class KrColorCache;
	QMap<QString, QColorGroup> m_cachedColors;
	KrColorSettings m_colorSettings;
	QColorGroup getColors(const KrColorItemType & type) const;
	static const QColor & setColorIfContrastIsSufficient(const QColor & background, const QColor & color1, const QColor & color2);
	QColor getForegroundColor(bool isActive) const;
	QColor getSpecialForegroundColor(const QString & type, bool isActive) const;
	QColor getBackgroundColor(bool isActive) const;
	QColor getAlternateBackgroundColor(bool isActive) const;
	QColor getMarkedForegroundColor(bool isActive) const;
	QColor getMarkedBackgroundColor(bool isActive) const;
	QColor getAlternateMarkedBackgroundColor(bool isActive) const;
	QColor getCurrentForegroundColor(bool isActive) const;
	QColor getCurrentBackgroundColor(bool isActive) const;
	QColor getCurrentMarkedForegroundColor(bool isActive) const;
	QColor dimColor(QColor color, bool isBackgroundColor) const;
};

QColorGroup KrColorCacheImpl::getColors(const KrColorItemType & type) const
{
	QColorGroup result;
	if (m_colorSettings.getBoolValue("KDE Default", _KDEDefaultColors))
	{
		// KDE default? Getcolors from KGlobalSettings.
		bool enableAlternateBackground = m_colorSettings.getBoolValue("Enable Alternate Background", _AlternateBackground);
		QColor background = enableAlternateBackground && type.m_alternateBackgroundColor ? 
			KGlobalSettings::alternateBackgroundColor()
			: KGlobalSettings::baseColor();
		result.setColor(QColorGroup::Base, background);
		result.setColor(QColorGroup::Background, background);
		result.setColor(QColorGroup::Text, KGlobalSettings::textColor());
		result.setColor(QColorGroup::HighlightedText, KGlobalSettings::highlightedTextColor());
		result.setColor(QColorGroup::Highlight, KGlobalSettings::highlightColor());
		return result;
	}
	bool markCurrentAlways = m_colorSettings.getBoolValue("Show Current Item Always", _ShowCurrentItemAlways);
	bool dimBackground = m_colorSettings.getBoolValue("Dim Inactive Colors", false);

	// cache m_activePanel flag. If color dimming is turned on, it is set to true, as the inactive colors
	// are calculated from the active ones at the end.
	bool isActive = type.m_activePanel;
	if (dimBackground)
		isActive = true;

	// First calculate fore- and background.
	QColor background = type.m_alternateBackgroundColor ?
		getAlternateBackgroundColor(isActive) 
		: getBackgroundColor(isActive);
	QColor foreground;
	switch(type.m_fileType)
	{
		case KrColorItemType::Directory :
			foreground = getSpecialForegroundColor("Directory", isActive);
			break;
		case KrColorItemType::Executable :
			foreground = getSpecialForegroundColor("Executable", isActive);
			break;
		case KrColorItemType::InvalidSymlink :
			foreground = getSpecialForegroundColor("Invalid Symlink", isActive);
			break;
		case KrColorItemType::Symlink :
			foreground = getSpecialForegroundColor("Symlink", isActive);
			break;
		default:
			foreground = getForegroundColor(isActive);
	}

	// set the background color
	result.setColor(QColorGroup::Base, background);
	result.setColor(QColorGroup::Background, background);
	
	// set the foreground color
	result.setColor(QColorGroup::Text, foreground);

	// now the color of a marked item
	QColor markedBackground = type.m_alternateBackgroundColor ?
		getAlternateMarkedBackgroundColor(isActive)
		: getMarkedBackgroundColor(isActive);
	QColor markedForeground = getMarkedForegroundColor(isActive);
	if (!markedForeground.isValid()) // transparent
		// choose fore- or background, depending on its contrast compared to markedBackground
		markedForeground = setColorIfContrastIsSufficient(markedBackground, foreground, background);

	// set it in the color group (different group color than normal foreground!)
	result.setColor(QColorGroup::HighlightedText, markedForeground);
	result.setColor(QColorGroup::Highlight, markedBackground);

	// In case the current item is a selected one, set the fore- and background colors for the contrast calculation below
	if (type.m_selectedItem)
	{
		background = markedBackground;
		foreground = markedForeground;
	}
	
	// finally the current item
	if (type.m_currentItem && (markCurrentAlways || isActive))
	{
		// if this is the current item AND the panels has the focus OR the current should be marked always
		QColor currentBackground = getCurrentBackgroundColor(isActive);

		if (!currentBackground.isValid()) // transparent
			currentBackground = background;
		
		// set the background
		result.setColor(QColorGroup::Highlight, currentBackground);
		result.setColor(QColorGroup::Base, currentBackground);
		result.setColor(QColorGroup::Background, currentBackground);
		
		QColor color;
		if (type.m_selectedItem)
			color = getCurrentMarkedForegroundColor(isActive);
		if (!color.isValid()) // not used
		{
			color = getCurrentForegroundColor(isActive);
			if (!color.isValid()) // transparent
				// choose fore- or background, depending on its contrast compared to markedBackground
				color = setColorIfContrastIsSufficient(currentBackground, foreground, background);
		}
		
		// set the foreground
		result.setColor(QColorGroup::Text, color);
		result.setColor(QColorGroup::HighlightedText, color);
	}

	if (dimBackground && !type.m_activePanel)
	{
		// if color dimming is choosen, dim the colors for the inactive panel 
		result.setColor(QColorGroup::Base, dimColor(result.base(), true));
		result.setColor(QColorGroup::Background, dimColor(result.base(), true));
		result.setColor(QColorGroup::Text, dimColor(result.text(), false));
		result.setColor(QColorGroup::HighlightedText, dimColor(result.highlightedText(), false));
		result.setColor(QColorGroup::Highlight, dimColor(result.highlight(), true));
	}
	return result;
}

const QColor & KrColorCacheImpl::setColorIfContrastIsSufficient(const QColor & background, const QColor & color1, const QColor & color2)
{
   #define sqr(x) ((x)*(x))
   int contrast = sqr(color1.red() - background.red()) + sqr(color1.green() - background.green()) + sqr(color1.blue() - background.blue());

   // if the contrast between background and color1 is too small, take color2 instead.
   if (contrast < 1000)
      return color2;
   return color1;
}

QColor KrColorCacheImpl::getForegroundColor(bool isActive) const
{
	QColor color = KGlobalSettings::textColor();
	SETCOLOR(color, m_colorSettings.getColorValue("Foreground"));
	if (!isActive) SETCOLOR(color, m_colorSettings.getColorValue("Inactive Foreground"));
	return color;
}

QColor KrColorCacheImpl::getSpecialForegroundColor(const QString & type, bool isActive) const
{
	QString colorName = "Inactive " + type + " Foreground";
	if (!isActive && m_colorSettings.getColorTextValue(colorName) == "Inactive Foreground")
		return getForegroundColor(false);
	QColor color = m_colorSettings.getColorValue(type + " Foreground");
	if (!isActive) SETCOLOR(color, m_colorSettings.getColorValue(colorName));
	if (!color.isValid())
		return getForegroundColor(isActive);
	return color;
}

QColor KrColorCacheImpl::getBackgroundColor(bool isActive) const
{
	QColor color = KGlobalSettings::baseColor();
	SETCOLOR(color, m_colorSettings.getColorValue("Background"));
	if (!isActive) SETCOLOR(color, m_colorSettings.getColorValue("Inactive Background"));
	return color;
}

QColor KrColorCacheImpl::getAlternateBackgroundColor(bool isActive) const
{
	if (isActive && m_colorSettings.getColorTextValue("Alternate Background") == "Background")
		return getBackgroundColor(true);
	if (!isActive && m_colorSettings.getColorTextValue("Inactive Alternate Background") == "")
		return getAlternateBackgroundColor(true);
	if (!isActive && m_colorSettings.getColorTextValue("Inactive Alternate Background") == "Inactive Background")
		return getBackgroundColor(false);
	QColor color = isActive ? 
		m_colorSettings.getColorValue("Alternate Background") 
		: m_colorSettings.getColorValue("Inactive Alternate Background");
	if (!color.isValid())
		color = KGlobalSettings::alternateBackgroundColor();
	if (!color.isValid())
		color = KGlobalSettings::baseColor();
	return color;
}

QColor KrColorCacheImpl::getMarkedForegroundColor(bool isActive) const
{
	QString colorName = isActive?"Marked Foreground":"Inactive Marked Foreground";
	if (m_colorSettings.getColorTextValue(colorName) == "transparent")
		return QColor();
	if (isActive && m_colorSettings.getColorTextValue(colorName) == "")
		return KGlobalSettings::highlightedTextColor();
	if (!isActive && m_colorSettings.getColorTextValue(colorName) == "")
		return getMarkedForegroundColor(true);
	return m_colorSettings.getColorValue(colorName);
}

QColor KrColorCacheImpl::getMarkedBackgroundColor(bool isActive) const
{
	if (isActive && m_colorSettings.getColorTextValue("Marked Background") == "")
		return KGlobalSettings::highlightColor();
	if (isActive && m_colorSettings.getColorTextValue("Marked Background") == "Background")
		return getBackgroundColor(true);
	if (!isActive && m_colorSettings.getColorTextValue("Inactive Marked Background") == "")
		return getMarkedBackgroundColor(true);
	if (!isActive && m_colorSettings.getColorTextValue("Inactive Marked Background") == "Inactive Background")
		return getBackgroundColor(false);
	return isActive ?
		m_colorSettings.getColorValue("Marked Background")
		: m_colorSettings.getColorValue("Inactive Marked Background");
}

QColor KrColorCacheImpl::getAlternateMarkedBackgroundColor(bool isActive) const
{
	if (isActive && m_colorSettings.getColorTextValue("Alternate Marked Background") == "Alternate Background")
		return getAlternateBackgroundColor(true);
	if (isActive && m_colorSettings.getColorTextValue("Alternate Marked Background") == "")
		return getMarkedBackgroundColor(true);
	if (!isActive && m_colorSettings.getColorTextValue("Inactive Alternate Marked Background") == "")
		return getAlternateMarkedBackgroundColor(true);
	if (!isActive && m_colorSettings.getColorTextValue("Inactive Alternate Marked Background") == "Inactive Alternate Background")
		return getAlternateBackgroundColor(false);
	if (!isActive && m_colorSettings.getColorTextValue("Inactive Alternate Marked Background") == "Inactive Marked Background")
		return getMarkedBackgroundColor(false);
	return isActive ? 
		m_colorSettings.getColorValue("Alternate Marked Background")
		: m_colorSettings.getColorValue("Inactive Alternate Marked Background");
}

QColor KrColorCacheImpl::getCurrentForegroundColor(bool isActive) const
{
	QColor color = m_colorSettings.getColorValue("Current Foreground");
	if (!isActive) SETCOLOR(color, m_colorSettings.getColorValue("Inactive Current Foreground"));
	return color;
}

QColor KrColorCacheImpl::getCurrentBackgroundColor(bool isActive) const
{
	if (isActive && m_colorSettings.getColorTextValue("Current Background") == "")
		return QColor();
	if (isActive && m_colorSettings.getColorTextValue("Current Background") == "Background")
		return getBackgroundColor(true);
	if (!isActive && m_colorSettings.getColorTextValue("Inactive Current Background") == "")
		return getCurrentBackgroundColor(true);
	if (!isActive && m_colorSettings.getColorTextValue("Inactive Current Background") == "Inactive Background")
		return getBackgroundColor(false);
	return isActive ? 
		m_colorSettings.getColorValue("Current Background")
		: m_colorSettings.getColorValue("Inactive Current Background");
}

QColor KrColorCacheImpl::getCurrentMarkedForegroundColor(bool isActive) const
{
	QString colorName = isActive?"Marked Current Foreground":"Inactive Marked Current Foreground";
	if (isActive && m_colorSettings.getColorTextValue(colorName) == "")
		return QColor();
	if (isActive && m_colorSettings.getColorTextValue(colorName) == "Marked Foreground")
		return getMarkedForegroundColor(true);
	if (!isActive && m_colorSettings.getColorTextValue(colorName) == "")
		return getCurrentMarkedForegroundColor(true);
	if (!isActive && m_colorSettings.getColorTextValue(colorName) == "Inactive Marked Foreground")
		return getMarkedForegroundColor(false);
	return m_colorSettings.getColorValue(colorName);
}

QColor KrColorCacheImpl::dimColor(QColor color, bool /* isBackgroundColor */) const
{
	krConfig->setGroup("Colors");
	int dimFactor = m_colorSettings.getNumValue("Dim Factor", 100);
	QColor targetColor = m_colorSettings.getColorValue("Dim Target Color");
	if (!targetColor.isValid())
		targetColor = QColor(255, 255, 255);
	bool dimBackground = m_colorSettings.getBoolValue("Dim Inactive Colors", false);
	bool dim = dimFactor >= 0 && dimFactor < 100 && dimBackground;
	if (dim)
		color = KrColorCache::dimColor(color, dimFactor, targetColor);
	return color;
}






KrColorCache * KrColorCache::m_instance = 0;

KrColorCache::KrColorCache()
{
	m_impl = new KrColorCacheImpl;
}

KrColorCache::~KrColorCache()
{
	delete m_impl;
}

KrColorCache & KrColorCache::getColorCache()
{
	if (!m_instance)
	{
		m_instance = new KrColorCache;
		m_instance->refreshColors();
	}
	return * m_instance;
}

void KrColorCache::getColors(QColorGroup  & result, const KrColorItemType & type) const
{
	// for the cache lookup: calculate a unique key from the type
	char hashKey[128];
	switch(type.m_fileType)
	{
		case KrColorItemType::Directory :
		 	strcpy(hashKey, "Directory");
			break;
		case KrColorItemType::Executable :
		 	strcpy(hashKey, "Executable");
			break;
		case KrColorItemType::InvalidSymlink :
		 	strcpy(hashKey, "InvalidSymlink");
			break;
		case KrColorItemType::Symlink :
		 	strcpy(hashKey, "Symlink");
			break;
		default:
		 	strcpy(hashKey, "File");
	}
	if (type.m_activePanel)
		strcat(hashKey, "-Active");
	if (type.m_alternateBackgroundColor)
		strcat(hashKey, "-Alternate");
	if (type.m_currentItem)
		strcat(hashKey, "-Current");
	if (type.m_selectedItem)
		strcat(hashKey, "-Selected");

	// lookup in cache
	if (!m_impl->m_cachedColors.contains(hashKey))
		// not found: calculate color group and store it in cache
		m_impl->m_cachedColors[hashKey] = m_impl->getColors(type);

	// get color group from cache
	const QColorGroup & col = m_impl->m_cachedColors[hashKey];

	// copy colors in question to result color group
	result.setColor(QColorGroup::Base, col.base());
	result.setColor(QColorGroup::Background, col.base());
	result.setColor(QColorGroup::Text, col.text());
	result.setColor(QColorGroup::HighlightedText, col.highlightedText());
	result.setColor(QColorGroup::Highlight, col.highlight());
}

QColor KrColorCache::dimColor(const QColor & color, int dim, const QColor & targetColor)
{
   return QColor((targetColor.red() * (100 - dim) + color.red() * dim) / 100, 
		(targetColor.green() * (100 - dim) + color.green() * dim) / 100, 
		(targetColor.blue() * (100 - dim) + color.blue() * dim) / 100);
}

void KrColorCache::refreshColors()
{
	m_impl->m_cachedColors.clear();
	m_impl->m_colorSettings = KrColorSettings();
	colorsRefreshed();
}

void KrColorCache::setColors(const KrColorSettings & colorSettings)
{
	m_impl->m_cachedColors.clear();
	m_impl->m_colorSettings = colorSettings;
	colorsRefreshed();
}

#include "krcolorcache.moc"
