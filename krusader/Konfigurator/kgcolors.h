/***************************************************************************
                          kgcolors.h  -  description
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

#ifndef __KGCOLORS_H__
#define __KGCOLORS_H__

#include "konfiguratorpage.h"
#include <qptrlist.h>
#include <qvaluelist.h>
#include <qlistview.h>

class KgColors : public KonfiguratorPage
{
  Q_OBJECT

public:
  KgColors( bool first, QWidget* parent=0,  const char* name=0 );

  bool apply();

public slots:
  void slotDisable();
  void slotForegroundChanged();
  void slotBackgroundChanged();
  void slotAltBackgroundChanged();
  void slotActiveChanged();
  void slotMarkedBackgroundChanged();
  void slotInactiveForegroundChanged();
  void slotInactiveBackgroundChanged();
  void slotInactiveAltBackgroundChanged();
  void slotInactiveMarkedBackgroundChanged();
  void generatePreview();

private:
  int                        addColorSelector( QString cfgName, QString name, QColor dflt, QString dfltName = QString::null,
                                               ADDITIONAL_COLOR *addColor = 0, int addColNum = 0);
  KonfiguratorColorChooser  *getColorSelector( QString name );
  QLabel                    *getSelectorLabel( QString name );
  const QColor & setColorIfContrastIsSufficient(const QColor & background, const QColor & color1, const QColor & color2);
  
private:
  QWidget                            *colorsGrp;
  QGridLayout                        *colorsGrid;
  int                                 offset;
  
  QGroupBox                          *previewGrp;
  QGridLayout                        *previewGrid;
  QTabWidget                         *colorTabWidget;

  KonfiguratorCheckBoxGroup          *generals;
  
  QPtrList<QLabel>                    labelList;
  QPtrList<KonfiguratorColorChooser>  itemList;
  QValueList<QString>                 itemNames;

  QListView                          *preview;

  class PreviewItem : public QListViewItem
  {
  private:
    QColor  defaultBackground;
    QColor  defaultForeground;
    QString label;
    
  public:
    PreviewItem( QListView * parent, QString name ) : QListViewItem( parent, name )
    {
      defaultBackground = QColor( 255, 255, 255 );
      defaultForeground = QColor( 0, 0, 0 );
      label = name;
    }

    void setColor( QColor foregnd, QColor backgnd )
    {
      defaultForeground = foregnd;
      defaultBackground = backgnd;
      listView()->repaintItem( this );
    }

    QString text()
    {
      return label;
    }

    void paintCell ( QPainter * p, const QColorGroup & cg, int column, int width, int align )
    {
      QColorGroup _cg( cg );
      _cg.setColor( QColorGroup::Base, defaultBackground );
      _cg.setColor( QColorGroup::Text, defaultForeground );
      QListViewItem::paintCell(p, _cg, column, width, align);
    }
  } *pwDir, *pwFile, *pwApp, *pwSymLink, *pwInvLink, *pwCurrent, *pwMark1, *pwMark2;
};
#endif /* __KGCOLORS_H__ */
