/*****************************************************************************
 * Copyright (C) 2000-2002 Shie Erlich <erlich@users.sourceforge.net>        *
 * Copyright (C) 2000-2002 Rafi Yanai <yanai@users.sourceforge.net>          *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef KRCOLORCACHE_H
#define KRCOLORCACHE_H

#include <QtCore/QObject>
#include <QList>
#include <qcolor.h>
#include <QtGui/QPalette>

/**
 * Design goals: Color calculation is done on one place only.
 * Configuration through krConfig OR through local settings.
 * Calculation must be fast through caching.
 *
 * This implementation exposes 3 classes:
 * KrColorSettings: holds the color settings from krConfig,
 *                  which can be changed locally
 * KrColorItemType: specifies the colors to be calculated
 * KrColorCache: perfomes the color calculation and caches the result.
 *               Uses KrColorSettings for the calculation
 *
 * Copies all used color settings from krConfig into a local cache on creation.
 * It contains 3 types of properties:
 * color, numeric (int) and boolean.
 * Color properties can have string or color values.
 * Property values can be changed.
 * These changes does not go into krConfig!
 *
 * is*Valid checks, if a protery name is valid
 * get*Names returns a list of all allowed property names
 * set*Value overwrites a property with a new value
 * get*Value retunrs the current value
 *
 * For colors the value can be returned as text or as color.
 * If a text representation is not a valid color, setColorValue(QColor())
 * should be called.
 */
class KrColorSettings
{
	class KrColorSettingsImpl * m_impl;
public:
	KrColorSettings();
	KrColorSettings(const KrColorSettings &);
	~KrColorSettings();
	const KrColorSettings & operator= (const KrColorSettings &);

	static bool isColorNameValid(const QString & settingName);
	static QList<QString> getColorNames();
	bool setColorValue(const QString & settingName, const QColor & color);
	QColor getColorValue(const QString & settingName) const;
	bool setColorTextValue(const QString & settingName, const QString & colorText);
	QString getColorTextValue(const QString & settingName) const;

	static bool isNumNameValid(const QString & settingName);
	static QList<QString> getNumNames();
	bool setNumValue(const QString & settingName, int value);
	int getNumValue(const QString & settingName, int defaultValue = 0) const;

	static bool isBoolNameValid(const QString & settingName);
	static QList<QString> getBoolNames();
	bool setBoolValue(const QString & settingName, bool value);
	int getBoolValue(const QString & settingName, bool defaultValue = false) const;
};

/**
 * A colletcion of properties which describe the color group to be calculated
 */
class KrColorItemType
{
public:
	enum FileType {File, InvalidSymlink, Symlink, Directory, Executable};
	FileType m_fileType;
	bool m_alternateBackgroundColor, m_activePanel, m_currentItem, m_selectedItem;
	KrColorItemType();
	KrColorItemType(FileType type, bool alternateBackgroundColor, bool activePanel, bool currentItem, bool selectedItem);
	KrColorItemType(const KrColorItemType &);
	const KrColorItemType & operator= (const KrColorItemType &);
};

/**
 * The color calculation. It bases on an internal KrColorSettings instance.
 * Via setColors it can be changed.
 * getColors does the color calculation.
 * It sets the colors Base, Background, Text, HighlightedText and Highlight.
 * All calculated values are cached.
 * The cache is deleted on refreshColors and setColors, which also trigger
 * colorsRefreshed.
 * getColorCache returns a static color cached for painting the panels.
 * On the color cache setColors should NEVER be called!
 */
class KrColorGroup {
public:
	inline KrColorGroup() : _textColor(), _backgroundColor(), _highlightedTextColor(),
		 _highlightedBackgroundColor() {}
	
	inline const QColor & text() const            { return _textColor; }
	inline const QColor & background() const      { return _backgroundColor; }
	inline const QColor & highlight() const       { return _highlightedBackgroundColor; }
	inline const QColor & highlightedText() const { return _highlightedTextColor; }
	
	inline void setText( QColor c )               { _textColor = c; }
	inline void setBackground( QColor c)          { _backgroundColor = c; }
	inline void setHighlight( QColor c )          { _highlightedBackgroundColor = c; }
	inline void setHighlightedText( QColor c )    { _highlightedTextColor = c; }

protected:
	QColor _textColor;
	QColor _backgroundColor;
	QColor _highlightedTextColor;
	QColor _highlightedBackgroundColor;
};

class KrColorCache : public QObject
{
	Q_OBJECT
	static KrColorCache * m_instance;
	class KrColorCacheImpl * m_impl;
	KrColorCache(const KrColorCache &);
	const KrColorCache & operator= (const KrColorCache &);
public:
	KrColorCache();
	~KrColorCache();
	static KrColorCache & getColorCache();
	void getColors(KrColorGroup & result, const KrColorItemType & type) const;
	static QColor dimColor(const QColor & color, int dim, const QColor & targetColor);
public slots:
	void refreshColors();
	void setColors(const KrColorSettings &);
signals:
	void colorsRefreshed();
};

#endif
