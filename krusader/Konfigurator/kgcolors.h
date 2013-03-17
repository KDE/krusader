/***************************************************************************
                          kgcolors.h  -  description
                             -------------------
    copyright            : (C) 2004 + by Csaba Karai
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

#ifndef KGCOLORS_H
#define KGCOLORS_H

#include <QtCore/QList>
#include <qstackedwidget.h>
#include <QGridLayout>
#include <QLabel>

#include "konfiguratorpage.h"
#include "../GUI/krtreewidget.h"

class KgColors : public KonfiguratorPage
{
    Q_OBJECT

public:
    KgColors(bool first, QWidget* parent = 0);

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

protected slots:
    void slotImportColors();
    void slotExportColors();

private:
    class PreviewItem;

    int                        addColorSelector(QString cfgName, QString name, QColor dflt, QString dfltName = QString(),
            ADDITIONAL_COLOR *addColor = 0, int addColNum = 0);
    KonfiguratorColorChooser  *getColorSelector(QString name);
    QLabel                    *getSelectorLabel(QString name);
    void                       serialize(class QDataStream &);
    void                       deserialize(class QDataStream &);
    void                       serializeItem(class QDataStream &, const char * name);
    void                       setColorWithDimming(PreviewItem * item, QColor foreground, QColor background, bool dimmed);

private:
    QWidget                            *colorsGrp;
    QGridLayout                        *colorsGrid;
    int                                 offset;
    int                                 endOfActiveColors;
    int                                 endOfPanelColors;
    int                                 activeTabIdx, inactiveTabIdx;
#ifdef ENABLE_SYNCHRONIZER
    int                                 synchronizerTabIdx;
#endif
    int                                 otherTabIdx;

    QGroupBox                          *previewGrp;
    QGridLayout                        *previewGrid;
    QTabWidget                         *colorTabWidget;

    QStackedWidget                     *inactiveColorStack;
    QWidget                            *normalInactiveWidget;
    QWidget                            *dimmedInactiveWidget;
    KonfiguratorSpinBox                *dimFactor;

    KonfiguratorCheckBoxGroup          *generals;

    QList<QLabel *>                     labelList;
    QList<KonfiguratorColorChooser *>   itemList;
    QList<QString>                      itemNames;

    KrTreeWidget                       *preview;
    KPushButton *importBtn, *exportBtn;

    class PreviewItem : public QTreeWidgetItem
    {
    private:
        QColor  defaultBackground;
        QColor  defaultForeground;
        QString label;

    public:
        PreviewItem(QTreeWidget * parent, QString name) : QTreeWidgetItem() {
            setText(0, name);
            defaultBackground = QColor(255, 255, 255);
            defaultForeground = QColor(0, 0, 0);
            label = name;
            parent->insertTopLevelItem(0, this);
        }

        void setColor(QColor foregnd, QColor backgnd) {
            defaultForeground = foregnd;
            defaultBackground = backgnd;


            QBrush textColor(foregnd);
            QBrush baseColor(backgnd);

            for (int i = 0; i != columnCount(); i++) {
                if (backgnd.isValid())
                    setBackground(i, baseColor);
                if (foregnd.isValid())
                    setForeground(i, textColor);
            }
        }

        QString text() {
            return label;
        }
    };
};
#endif /* __KGCOLORS_H__ */
