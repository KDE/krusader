/*****************************************************************************
 * Copyright (C) 2004 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2004-2019 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#ifndef KGCOLORS_H
#define KGCOLORS_H

// QtCore
#include <QList>
// QtWidgets
#include <QStackedWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

#include "konfiguratorpage.h"
#include "../GUI/krtreewidget.h"

class KgColors : public KonfiguratorPage
{
    Q_OBJECT

public:
    explicit KgColors(bool first, QWidget* parent = nullptr);

    bool apply() Q_DECL_OVERRIDE;

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

    int                        addColorSelector(const QString& cfgName, QString name, QColor defaultValue, const QString& dfltName = QString(),
            ADDITIONAL_COLOR *addColor = nullptr, int addColNum = 0);
    KonfiguratorColorChooser  *getColorSelector(const QString& name);
    QLabel                    *getSelectorLabel(const QString& name);
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
#ifdef SYNCHRONIZER_ENABLED
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
    QPushButton *importBtn, *exportBtn;

    class PreviewItem : public QTreeWidgetItem
    {
    private:
        QColor  defaultBackground;
        QColor  defaultForeground;
        QString label;

    public:
        PreviewItem(QTreeWidget * parent, const QString& name)
        {
            setText(0, name);
            defaultBackground = QColor(255, 255, 255);
            defaultForeground = QColor(0, 0, 0);
            label = name;
            parent->insertTopLevelItem(0, this);
        }

        void setColor(const QColor& foregnd, const QColor& backgnd) {
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
