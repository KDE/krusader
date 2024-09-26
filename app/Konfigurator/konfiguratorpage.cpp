/*
    SPDX-FileCopyrightText: 2003 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "konfiguratorpage.h"

// QtWidgets
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QScrollArea>

#include <KConfig>
#include <utility>

#include "../krglobal.h"

KonfiguratorPage::KonfiguratorPage(bool firstTime, QWidget *parent)
    : QScrollArea(parent)
    , firstCall(firstTime)
{
    setFrameStyle(QFrame::NoFrame);
}

bool KonfiguratorPage::apply()
{
    bool restartNeeded = false;

    for (auto &item : itemList)
        restartNeeded = item->apply() || restartNeeded;

    krConfig->sync();
    return restartNeeded;
}

void KonfiguratorPage::setDefaults()
{
    int activePage = activeSubPage();

    for (auto &item : itemList) {
        if (item->subPage() == activePage)
            item->setDefaults();
    }
}

void KonfiguratorPage::loadInitialValues()
{
    for (auto &item : itemList)
        item->loadInitialValue();
}

bool KonfiguratorPage::isChanged()
{
    bool isChanged = false;

    for (auto &item : itemList)
        isChanged = isChanged || item->isChanged();

    return isChanged;
}

KonfiguratorCheckBox *KonfiguratorPage::createCheckBox(QString configGroup,
                                                       QString name,
                                                       bool defaultValue,
                                                       QString text,
                                                       QWidget *parent,
                                                       bool restart,
                                                       const QString &toolTip,
                                                       int page)
{
    KonfiguratorCheckBox *checkBox = new KonfiguratorCheckBox(std::move(configGroup), std::move(name), defaultValue, std::move(text), parent, restart, page);
    if (!toolTip.isEmpty()) {
        checkBox->setWhatsThis(toolTip);
        checkBox->setToolTip(toolTip);
    }

    registerObject(checkBox->extension());
    return checkBox;
}

KonfiguratorSpinBox *KonfiguratorPage::createSpinBox(QString configGroup,
                                                     QString configName,
                                                     int defaultValue,
                                                     int min,
                                                     int max,
                                                     QLabel *label,
                                                     QWidget *parent,
                                                     bool restart,
                                                     const QString &toolTip,
                                                     int page)
{
    KonfiguratorSpinBox *spinBox = new KonfiguratorSpinBox(std::move(configGroup), std::move(configName), defaultValue, min, max, parent, restart, page);
    if (!toolTip.isEmpty()) {
        label->setWhatsThis(toolTip);
        label->setToolTip(toolTip);
        spinBox->setWhatsThis(toolTip);
        spinBox->setToolTip(toolTip);
    }

    registerObject(spinBox->extension());
    return spinBox;
}

KonfiguratorEditBox *KonfiguratorPage::createEditBox(QString configGroup,
                                                     QString name,
                                                     QString defaultValue,
                                                     QLabel *label,
                                                     QWidget *parent,
                                                     bool restart,
                                                     const QString &toolTip,
                                                     int page)
{
    KonfiguratorEditBox *editBox = new KonfiguratorEditBox(std::move(configGroup), std::move(name), std::move(defaultValue), parent, restart, page);
    if (!toolTip.isEmpty()) {
        label->setWhatsThis(toolTip);
        label->setToolTip(toolTip);
        editBox->setWhatsThis(toolTip);
        editBox->setToolTip(toolTip);
    }

    registerObject(editBox->extension());
    return editBox;
}

KonfiguratorListBox *
KonfiguratorPage::createListBox(QString configGroup, QString name, QStringList defaultValue, QWidget *parent, bool restart, const QString &toolTip, int page)
{
    KonfiguratorListBox *listBox = new KonfiguratorListBox(std::move(configGroup), std::move(name), std::move(defaultValue), parent, restart, page);
    if (!toolTip.isEmpty()) {
        listBox->setWhatsThis(toolTip);
        listBox->setToolTip(toolTip);
    }

    registerObject(listBox->extension());
    return listBox;
}

KonfiguratorURLRequester *KonfiguratorPage::createURLRequester(QString configGroup,
                                                               QString name,
                                                               QString defaultValue,
                                                               QLabel *label,
                                                               QWidget *parent,
                                                               bool restart,
                                                               const QString &toolTip,
                                                               int page,
                                                               bool expansion)
{
    KonfiguratorURLRequester *urlRequester =
        new KonfiguratorURLRequester(std::move(configGroup), std::move(name), std::move(defaultValue), parent, restart, page, expansion);
    if (!toolTip.isEmpty()) {
        label->setWhatsThis(toolTip);
        label->setToolTip(toolTip);
        urlRequester->setWhatsThis(toolTip);
        urlRequester->setToolTip(toolTip);
    }

    registerObject(urlRequester->extension());
    return urlRequester;
}

QGroupBox *KonfiguratorPage::createFrame(const QString &text, QWidget *parent)
{
    auto *groupBox = new QGroupBox(parent);
    if (!text.isNull())
        groupBox->setTitle(text);
    return groupBox;
}

QGridLayout *KonfiguratorPage::createGridLayout(QWidget *parent)
{
    auto *gridLayout = new QGridLayout(parent);
    gridLayout->setAlignment(Qt::AlignTop);
    gridLayout->setSpacing(6);
    gridLayout->setContentsMargins(11, 11, 11, 11);
    return gridLayout;
}

QLabel *KonfiguratorPage::addLabel(QGridLayout *layout, int x, int y, const QString &label, QWidget *parent)
{
    QLabel *lbl = new QLabel(label, parent);
    layout->addWidget(lbl, x, y);
    return lbl;
}

QWidget *KonfiguratorPage::createSpacer(QWidget *parent)
{
    QWidget *widget = new QWidget(parent);
    auto *hboxlayout = new QHBoxLayout(widget);
    auto *spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hboxlayout->addItem(spacer);
    return widget;
}

KonfiguratorCheckBoxGroup *
KonfiguratorPage::createCheckBoxGroup(int sizex, int sizey, KONFIGURATOR_CHECKBOX_PARAM *params, int paramNum, QWidget *parent, int page)
{
    auto *groupWidget = new KonfiguratorCheckBoxGroup(parent);
    auto *layout = new QGridLayout(groupWidget);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);

    int x = 0, y = 0;

    for (int i = 0; i != paramNum; i++) {
        KonfiguratorCheckBox *checkBox = createCheckBox(params[i].configClass,
                                                        params[i].configName,
                                                        params[i].defaultValue,
                                                        params[i].text,
                                                        groupWidget,
                                                        params[i].restart,
                                                        params[i].toolTip,
                                                        page);

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

KonfiguratorRadioButtons *KonfiguratorPage::createRadioButtonGroup(QString configGroup,
                                                                   QString name,
                                                                   QString defaultValue,
                                                                   int sizex,
                                                                   int sizey,
                                                                   KONFIGURATOR_NAME_VALUE_TIP *params,
                                                                   int paramNum,
                                                                   QWidget *parent,
                                                                   bool restart,
                                                                   int page)
{
    KonfiguratorRadioButtons *radioWidget =
        new KonfiguratorRadioButtons(std::move(configGroup), std::move(name), std::move(defaultValue), parent, restart, page);

    auto *layout = new QGridLayout(radioWidget);
    layout->setAlignment(Qt::AlignTop);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);

    int x = 0, y = 0;

    for (int i = 0; i != paramNum; i++) {
        auto *radBtn = new QRadioButton(params[i].text, radioWidget);

        if (!params[i].tooltip.isEmpty()) {
            radBtn->setWhatsThis(params[i].tooltip);
            radBtn->setToolTip(params[i].tooltip);
        }

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

KonfiguratorFontChooser *KonfiguratorPage::createFontChooser(QString configGroup,
                                                             QString name,
                                                             const QFont &defaultValue,
                                                             QLabel *label,
                                                             QWidget *parent,
                                                             bool restart,
                                                             const QString &toolTip,
                                                             int page)
{
    KonfiguratorFontChooser *fontChooser = new KonfiguratorFontChooser(std::move(configGroup), std::move(name), defaultValue, parent, restart, page);
    if (!toolTip.isEmpty()) {
        label->setWhatsThis(toolTip);
        label->setToolTip(toolTip);
        fontChooser->setWhatsThis(toolTip);
        fontChooser->setToolTip(toolTip);
    }

    registerObject(fontChooser->extension());
    return fontChooser;
}

KonfiguratorComboBox *KonfiguratorPage::createComboBox(QString configGroup,
                                                       QString name,
                                                       QString defaultValue,
                                                       KONFIGURATOR_NAME_VALUE_PAIR *params,
                                                       int paramNum,
                                                       QLabel *label,
                                                       QWidget *parent,
                                                       bool restart,
                                                       bool editable,
                                                       const QString &toolTip,
                                                       int page)
{
    KonfiguratorComboBox *comboBox =
        new KonfiguratorComboBox(std::move(configGroup), std::move(name), std::move(defaultValue), params, paramNum, parent, restart, editable, page);
    if (!toolTip.isEmpty()) {
        label->setWhatsThis(toolTip);
        label->setToolTip(toolTip);
        comboBox->setWhatsThis(toolTip);
        comboBox->setToolTip(toolTip);
    }

    registerObject(comboBox->extension());
    return comboBox;
}

QFrame *KonfiguratorPage::createLine(QWidget *parent, bool vertical)
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
    qsizetype ndx = itemList.indexOf(item);
    if (ndx != -1) {
        QList<KonfiguratorExtension *>::iterator it = itemList.begin() + ndx;
        delete *it;
        itemList.erase(it);
    }
}

KonfiguratorColorChooser *KonfiguratorPage::createColorChooser(QString configGroup,
                                                               QString name,
                                                               QColor defaultValue,
                                                               QWidget *parent,
                                                               bool restart,
                                                               ADDITIONAL_COLOR *addColPtr,
                                                               int addColNum,
                                                               int page)
{
    KonfiguratorColorChooser *colorChooser =
        new KonfiguratorColorChooser(std::move(configGroup), std::move(name), std::move(defaultValue), parent, restart, addColPtr, addColNum, page);

    registerObject(colorChooser->extension());
    return colorChooser;
}
