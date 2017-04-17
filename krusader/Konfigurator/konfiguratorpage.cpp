/* **************************************************************************
                      konfiguratorpage.cpp  -  description
                             -------------------
    copyright            : (C) 2003 by Csaba Karai
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

#include "konfiguratorpage.h"

// QtWidgets
#include <QLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>

#include <KConfigCore/KConfig>

#include "../krglobal.h"

KonfiguratorPage::KonfiguratorPage(bool firstTime, QWidget* parent) :
        QScrollArea(parent), firstCall(firstTime)
{
    setFrameStyle(QFrame::NoFrame);
}

bool KonfiguratorPage::apply()
{
    bool restartNeeded = false;

    for (QList<KonfiguratorExtension *>::iterator item = itemList.begin(); item != itemList.end(); item ++)
        restartNeeded = (*item)->apply() || restartNeeded;

    krConfig->sync();
    return restartNeeded;
}

void KonfiguratorPage::setDefaults()
{
    int activePage = activeSubPage();

    for (QList<KonfiguratorExtension *>::iterator item = itemList.begin(); item != itemList.end(); item ++) {
        if ((*item)->subPage() == activePage)
            (*item)->setDefaults();
    }
}

void KonfiguratorPage::loadInitialValues()
{
    for (QList<KonfiguratorExtension *>::iterator item = itemList.begin(); item != itemList.end(); item ++)
        (*item)->loadInitialValue();
}

bool KonfiguratorPage::isChanged()
{
    bool isChanged = false;

    for (QList<KonfiguratorExtension *>::iterator item = itemList.begin(); item != itemList.end(); item ++)
        isChanged = isChanged || (*item)->isChanged();

    return isChanged;
}

KonfiguratorCheckBox* KonfiguratorPage::createCheckBox(QString configGroup, QString name,
        bool defaultValue, QString text, QWidget *parent, bool restart, QString toolTip, int page)
{
    KonfiguratorCheckBox *checkBox = new KonfiguratorCheckBox(configGroup, name, defaultValue, text,
            parent, restart, page);
    if (!toolTip.isEmpty())
        checkBox->setWhatsThis(toolTip);

    registerObject(checkBox->extension());
    return checkBox;
}

KonfiguratorSpinBox* KonfiguratorPage::createSpinBox(QString configGroup, QString configName,
        int defaultValue, int min, int max, QWidget *parent, bool restart, int page)
{
    KonfiguratorSpinBox *spinBox = new KonfiguratorSpinBox(configGroup, configName, defaultValue,
                                                           min, max, parent, restart, page);

    registerObject(spinBox->extension());
    return spinBox;
}

KonfiguratorEditBox* KonfiguratorPage::createEditBox(QString configGroup, QString name,
        QString defaultValue, QWidget *parent, bool restart, int page)
{
    KonfiguratorEditBox *editBox = new KonfiguratorEditBox(configGroup, name, defaultValue, parent,
            restart, page);

    registerObject(editBox->extension());
    return editBox;
}

KonfiguratorListBox* KonfiguratorPage::createListBox(QString configGroup, QString name,
        QStringList defaultValue, QWidget *parent, bool restart, int page)
{
    KonfiguratorListBox *listBox = new KonfiguratorListBox(configGroup, name, defaultValue, parent,
            restart, page);

    registerObject(listBox->extension());
    return listBox;
}

KonfiguratorURLRequester* KonfiguratorPage::createURLRequester(QString configGroup, QString name,
        QString defaultValue, QWidget *parent, bool restart, int page, bool expansion)
{
    KonfiguratorURLRequester *urlRequester = new KonfiguratorURLRequester(configGroup, name, defaultValue,
            parent, restart, page, expansion);

    registerObject(urlRequester->extension());
    return urlRequester;
}

QGroupBox* KonfiguratorPage::createFrame(QString text, QWidget *parent)
{
    QGroupBox *groupBox = new QGroupBox(parent);
    if (!text.isNull())
        groupBox->setTitle(text);
    return groupBox;
}

QGridLayout* KonfiguratorPage::createGridLayout(QWidget *parent)
{
    QGridLayout *gridLayout = new QGridLayout(parent);
    gridLayout->setAlignment(Qt::AlignTop);
    gridLayout->setSpacing(6);
    gridLayout->setContentsMargins(11, 11, 11, 11);
    return gridLayout;
}

QLabel* KonfiguratorPage::addLabel(QGridLayout *layout, int x, int y, QString label,
                                   QWidget *parent)
{
    QLabel *lbl = new QLabel(label, parent);
    layout->addWidget(lbl, x, y);
    return lbl;
}

QWidget* KonfiguratorPage::createSpacer(QWidget *parent)
{
    QWidget *widget = new QWidget(parent);
    QHBoxLayout *hboxlayout = new QHBoxLayout(widget);
    QSpacerItem* spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hboxlayout->addItem(spacer);
    return widget;
}

KonfiguratorCheckBoxGroup* KonfiguratorPage::createCheckBoxGroup(int sizex, int sizey,
        KONFIGURATOR_CHECKBOX_PARAM *params, int paramNum, QWidget *parent,
        int page)
{
    KonfiguratorCheckBoxGroup *groupWidget = new KonfiguratorCheckBoxGroup(parent);
    QGridLayout *layout = new QGridLayout(groupWidget);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);

    int x = 0, y = 0;

    for (int i = 0; i != paramNum; i++) {
        KonfiguratorCheckBox *checkBox = createCheckBox(params[i].configClass,
                                         params[i].configName, params[i].defaultValue, params[i].text, groupWidget,
                                         params[i].restart, params[i].toolTip, page);

        groupWidget->add(checkBox);
        layout->addWidget(checkBox, y, x);

        if (sizex) {
            if (++x == sizex)
                x = 0, y++;
        } else {
            if (++y == sizey)
                y = 0, x++;
        }
    }

    return groupWidget;
}

KonfiguratorRadioButtons* KonfiguratorPage::createRadioButtonGroup(QString configGroup,
        QString name, QString defaultValue, int sizex, int sizey, KONFIGURATOR_NAME_VALUE_TIP *params,
        int paramNum, QWidget *parent, bool restart, int page)
{
    KonfiguratorRadioButtons *radioWidget = new KonfiguratorRadioButtons(configGroup, name, defaultValue, parent, restart, page);

    QGridLayout *layout = new QGridLayout(radioWidget);
    layout->setAlignment(Qt::AlignTop);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);

    int x = 0, y = 0;

    for (int i = 0; i != paramNum; i++) {
        QRadioButton *radBtn = new QRadioButton(params[i].text, radioWidget);

        if (!params[i].tooltip.isEmpty())
            radBtn->setWhatsThis(params[i].tooltip);

        layout->addWidget(radBtn, y, x);

        radioWidget->addRadioButton(radBtn, params[i].text, params[i].value);

        if (sizex) {
            if (++x == sizex)
                x = 0, y++;
        } else {
            if (++y == sizey)
                y = 0, x++;
        }
    }

    radioWidget->loadInitialValue();
    registerObject(radioWidget->extension());
    return radioWidget;
}

KonfiguratorFontChooser *KonfiguratorPage::createFontChooser(QString configGroup, QString name,
        QFont defaultValue, QWidget *parent, bool restart, int page)
{
    KonfiguratorFontChooser *fontChooser = new KonfiguratorFontChooser(configGroup, name, defaultValue, parent,
            restart, page);

    registerObject(fontChooser->extension());
    return fontChooser;
}

KonfiguratorComboBox *KonfiguratorPage::createComboBox(QString configGroup, QString name, QString defaultValue,
        KONFIGURATOR_NAME_VALUE_PAIR *params, int paramNum, QWidget *parent, bool restart, bool editable, int page)
{
    KonfiguratorComboBox *comboBox = new KonfiguratorComboBox(configGroup, name, defaultValue, params,
            paramNum, parent, restart, editable, page);

    registerObject(comboBox->extension());
    return comboBox;
}

QFrame* KonfiguratorPage::createLine(QWidget *parent, bool vertical)
{
    QFrame *line = new QFrame(parent);
    line->setFrameStyle((vertical ? QFrame::VLine : QFrame::HLine) | QFrame::Sunken);
    return line;
}

void KonfiguratorPage::registerObject(KonfiguratorExtension *item)
{
    itemList.push_back(item);
    connect(item, SIGNAL(sigChanged(bool)), this, SIGNAL(sigChanged()));
}

void KonfiguratorPage::removeObject(KonfiguratorExtension *item)
{
    int ndx = itemList.indexOf(item);
    if (ndx != -1) {
        QList<KonfiguratorExtension *>::iterator it = itemList.begin() + ndx;
        delete *it;
        itemList.erase(it);
    }
}

KonfiguratorColorChooser *KonfiguratorPage::createColorChooser(QString configGroup, QString name, QColor defaultValue,
        QWidget *parent, bool restart,
        ADDITIONAL_COLOR *addColPtr, int addColNum, int page)
{
    KonfiguratorColorChooser *colorChooser = new KonfiguratorColorChooser(configGroup, name, defaultValue,  parent,
            restart, addColPtr, addColNum, page);

    registerObject(colorChooser->extension());
    return colorChooser;
}

