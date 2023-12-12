/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGCOLORS_H
#define KGCOLORS_H

// QtCore
#include <QList>
// QtWidgets
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>

#include "../GUI/krtreewidget.h"
#include "konfiguratorpage.h"

class KgColors : public KonfiguratorPage
{
    Q_OBJECT

public:
    explicit KgColors(bool first, QWidget *parent = nullptr);

    bool apply() override;

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

    int addColorSelector(const QString &cfgName,
                         QString name,
                         QColor defaultValue,
                         const QString &dfltName = QString(),
                         ADDITIONAL_COLOR *addColor = nullptr,
                         int addColNum = 0);
    KonfiguratorColorChooser *getColorSelector(const QString &name);
    QLabel *getSelectorLabel(const QString &name);
    void serialize(class QDataStream &);
    void deserialize(class QDataStream &);
    void serializeItem(class QDataStream &, const char *name);
    void setColorWithDimming(PreviewItem *item, QColor foreground, QColor background, bool dimmed);

private:
    QWidget *colorsGrp;
    QGridLayout *colorsGrid;
    int offset;
    int endOfActiveColors;
    int endOfPanelColors;
    int activeTabIdx, inactiveTabIdx;
#ifdef SYNCHRONIZER_ENABLED
    int synchronizerTabIdx;
#endif
    int otherTabIdx;

    QGroupBox *previewGrp;
    QGridLayout *previewGrid;
    QTabWidget *colorTabWidget;

    QStackedWidget *inactiveColorStack;
    QWidget *normalInactiveWidget;
    QWidget *dimmedInactiveWidget;
    KonfiguratorSpinBox *dimFactor;

    KonfiguratorCheckBoxGroup *generals;

    QList<QLabel *> labelList;
    QList<KonfiguratorColorChooser *> itemList;
    QList<QString> itemNames;

    KrTreeWidget *preview;
    QPushButton *importBtn, *exportBtn;

    class PreviewItem : public QTreeWidgetItem
    {
    private:
        QColor defaultBackground;
        QColor defaultForeground;
        QString label;

    public:
        PreviewItem(QTreeWidget *parent, const QString &name)
        {
            setText(0, name);
            defaultBackground = QColor(255, 255, 255);
            defaultForeground = QColor(0, 0, 0);
            label = name;
            parent->insertTopLevelItem(0, this);
        }

        void setColor(const QColor &foregnd, const QColor &backgnd)
        {
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

        QString text()
        {
            return label;
        }
    };
};
#endif /* __KGCOLORS_H__ */
