/***************************************************************************
                      advancedfilter.cpp  -  description
                             -------------------
    copyright            : (C) 2003 + by Shie Erlich & Rafi Yanai & Csaba Karai
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

#include "advancedfilter.h"

#include <time.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QPixmap>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QButtonGroup>

#include <KLocale>
#include <KDebug>
#include <KMessageBox>
#include <KIconLoader>

#include "../krglobal.h"
#include "../Dialogs/krdialogs.h"

#define USERSFILE  QString("/etc/passwd")
#define GROUPSFILE QString("/etc/group")

AdvancedFilter::AdvancedFilter(FilterTabs *tabs, QWidget *parent) : QWidget(parent), fltTabs(tabs)
{
    QGridLayout *filterLayout = new QGridLayout(this);
    filterLayout->setSpacing(6);
    filterLayout->setContentsMargins(11, 11, 11, 11);

    // Options for size

    QGroupBox *sizeGroup = new QGroupBox(this);
    QSizePolicy sizeGroupPolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    sizeGroupPolicy.setHeightForWidth(sizeGroup->sizePolicy().hasHeightForWidth());
    sizeGroup->setSizePolicy(sizeGroupPolicy);
    sizeGroup->setTitle(i18n("Size"));
    QGridLayout *sizeLayout = new QGridLayout(sizeGroup);
    sizeLayout->setAlignment(Qt::AlignTop);
    sizeLayout->setSpacing(6);
    sizeLayout->setContentsMargins(11, 11, 11, 11);

    minSizeEnabled = new QCheckBox(sizeGroup);
    minSizeEnabled->setText(i18n("At Least"));
    sizeLayout->addWidget(minSizeEnabled, 0, 0);

    minSizeAmount = new KLineEdit(sizeGroup);
    minSizeAmount->setEnabled(false);
    QSizePolicy minSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    minSizePolicy.setHeightForWidth(minSizeAmount->sizePolicy().hasHeightForWidth());
    minSizeAmount->setSizePolicy(minSizePolicy);
    sizeLayout->addWidget(minSizeAmount, 0, 1);

    minSizeType = new KComboBox(false, sizeGroup);
    minSizeType->addItem(i18n("Byte"));
    minSizeType->addItem(i18n("KiB"));
    minSizeType->addItem(i18n("MiB"));
    minSizeType->addItem(i18n("GiB"));
    minSizeType->setEnabled(false);
    sizeLayout->addWidget(minSizeType, 0, 2);

    maxSizeEnabled = new QCheckBox(sizeGroup);
    maxSizeEnabled->setText(i18n("At Most"));
    sizeLayout->addWidget(maxSizeEnabled, 0, 3);

    maxSizeAmount = new KLineEdit(sizeGroup);
    maxSizeAmount->setEnabled(false);
    QSizePolicy maxSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    maxSizePolicy.setHeightForWidth(maxSizeAmount->sizePolicy().hasHeightForWidth());
    maxSizeAmount->setSizePolicy(maxSizePolicy);
    sizeLayout->addWidget(maxSizeAmount, 0, 4);

    maxSizeType = new KComboBox(false, sizeGroup);
    maxSizeType->addItem(i18n("Byte"));
    maxSizeType->addItem(i18n("KiB"));
    maxSizeType->addItem(i18n("MiB"));
    maxSizeType->addItem(i18n("GiB"));
    maxSizeType->setEnabled(false);
    sizeLayout->addWidget(maxSizeType, 0, 5);

    // set a tighter box around the type box

    int height = QFontMetrics(minSizeType->font()).height() + 2;
    minSizeType->setMaximumHeight(height);
    maxSizeType->setMaximumHeight(height);

    filterLayout->addWidget(sizeGroup, 0, 0);

    // Options for date

    QPixmap iconDate = krLoader->loadIcon("date", KIconLoader::Toolbar, 16);

    QGroupBox *dateGroup = new QGroupBox(this);
    QButtonGroup *btnGroup = new QButtonGroup(dateGroup);
    dateGroup->setTitle(i18n("Date"));
    btnGroup->setExclusive(true);
    QGridLayout *dateLayout = new QGridLayout(dateGroup);
    dateLayout->setAlignment(Qt::AlignTop);
    dateLayout->setSpacing(6);
    dateLayout->setContentsMargins(11, 11, 11, 11);

    modifiedBetweenEnabled = new QRadioButton(dateGroup);
    modifiedBetweenEnabled->setText(i18n("&Modified between"));
    btnGroup->addButton(modifiedBetweenEnabled);

    dateLayout->addWidget(modifiedBetweenEnabled, 0, 0, 1, 2);

    modifiedBetweenData1 = new KLineEdit(dateGroup);
    modifiedBetweenData1->setEnabled(false);
    modifiedBetweenData1->setText("");
    dateLayout->addWidget(modifiedBetweenData1, 0, 2, 1, 2);

    modifiedBetweenBtn1 = new QToolButton(dateGroup);
    modifiedBetweenBtn1->setEnabled(false);
    modifiedBetweenBtn1->setText("");
    modifiedBetweenBtn1->setIcon(QIcon(iconDate));
    dateLayout->addWidget(modifiedBetweenBtn1, 0, 4);

    QLabel *andLabel = new QLabel(dateGroup);
    andLabel->setText(i18n("an&d"));
    dateLayout->addWidget(andLabel, 0, 5);

    modifiedBetweenData2 = new KLineEdit(dateGroup);
    modifiedBetweenData2->setEnabled(false);
    modifiedBetweenData2->setText("");
    andLabel->setBuddy(modifiedBetweenData2);
    dateLayout->addWidget(modifiedBetweenData2, 0, 6);

    modifiedBetweenBtn2 = new QToolButton(dateGroup);
    modifiedBetweenBtn2->setEnabled(false);
    modifiedBetweenBtn2->setText("");
    modifiedBetweenBtn2->setIcon(QIcon(iconDate));
    dateLayout->addWidget(modifiedBetweenBtn2, 0, 7);

    notModifiedAfterEnabled = new QRadioButton(dateGroup);
    notModifiedAfterEnabled->setText(i18n("&Not modified after"));
    btnGroup->addButton(notModifiedAfterEnabled);
    dateLayout->addWidget(notModifiedAfterEnabled, 1, 0, 1, 2);

    notModifiedAfterData = new KLineEdit(dateGroup);
    notModifiedAfterData->setEnabled(false);
    notModifiedAfterData->setText("");
    dateLayout->addWidget(notModifiedAfterData, 1, 2, 1, 2);

    notModifiedAfterBtn = new QToolButton(dateGroup);
    notModifiedAfterBtn->setEnabled(false);
    notModifiedAfterBtn->setText("");
    notModifiedAfterBtn->setIcon(QIcon(iconDate));
    dateLayout->addWidget(notModifiedAfterBtn, 1, 4);

    modifiedInTheLastEnabled = new QRadioButton(dateGroup);
    modifiedInTheLastEnabled->setText(i18n("Mod&ified in the last"));
    btnGroup->addButton(modifiedInTheLastEnabled);
    dateLayout->addWidget(modifiedInTheLastEnabled, 2, 0);

    modifiedInTheLastData = new KLineEdit(dateGroup);
    modifiedInTheLastData->setEnabled(false);
    modifiedInTheLastData->setText("");
    dateLayout->addWidget(modifiedInTheLastData, 2, 2);

    modifiedInTheLastType = new KComboBox(dateGroup);
    modifiedInTheLastType->addItem(i18n("days"));
    modifiedInTheLastType->addItem(i18n("weeks"));
    modifiedInTheLastType->addItem(i18n("months"));
    modifiedInTheLastType->addItem(i18n("years"));
    modifiedInTheLastType->setEnabled(false);
    dateLayout->addWidget(modifiedInTheLastType, 2, 3, 1, 2);

    notModifiedInTheLastData = new KLineEdit(dateGroup);
    notModifiedInTheLastData->setEnabled(false);
    notModifiedInTheLastData->setText("");
    dateLayout->addWidget(notModifiedInTheLastData, 3, 2);

    QLabel *notModifiedInTheLastLbl = new QLabel(dateGroup);
    notModifiedInTheLastLbl->setText(i18n("No&t modified in the last"));
    notModifiedInTheLastLbl->setBuddy(notModifiedInTheLastData);
    dateLayout->addWidget(notModifiedInTheLastLbl, 3, 0);

    notModifiedInTheLastType = new KComboBox(dateGroup);
    notModifiedInTheLastType->addItem(i18n("days"));
    notModifiedInTheLastType->addItem(i18n("weeks"));
    notModifiedInTheLastType->addItem(i18n("months"));
    notModifiedInTheLastType->addItem(i18n("years"));
    notModifiedInTheLastType->setEnabled(false);
    dateLayout->addWidget(notModifiedInTheLastType, 3, 3, 1, 2);

    filterLayout->addWidget(dateGroup, 1, 0);

    // Options for ownership

    QGroupBox *ownershipGroup = new QGroupBox(this);
    ownershipGroup->setTitle(i18n("Ownership"));
    QGridLayout *ownershipLayout = new QGridLayout(ownershipGroup);
    ownershipLayout->setAlignment(Qt::AlignTop);
    ownershipLayout->setSpacing(6);
    ownershipLayout->setContentsMargins(11, 11, 11, 11);

    QHBoxLayout *hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setContentsMargins(0, 0, 0, 0);

    belongsToUserEnabled = new QCheckBox(ownershipGroup);
    belongsToUserEnabled->setText(i18n("Belongs to &user"));
    hboxLayout->addWidget(belongsToUserEnabled);

    belongsToUserData = new KComboBox(ownershipGroup);
    belongsToUserData->setEnabled(false);
    belongsToUserData->setEditable(false);
    hboxLayout->addWidget(belongsToUserData);

    belongsToGroupEnabled = new QCheckBox(ownershipGroup);
    belongsToGroupEnabled->setText(i18n("Belongs to gr&oup"));
    hboxLayout->addWidget(belongsToGroupEnabled);

    belongsToGroupData = new KComboBox(ownershipGroup);
    belongsToGroupData->setEnabled(false);
    belongsToGroupData->setEditable(false);
    hboxLayout->addWidget(belongsToGroupData);

    ownershipLayout->addLayout(hboxLayout, 0, 0, 1, 4);

    permissionsEnabled = new QCheckBox(ownershipGroup);
    permissionsEnabled->setText(i18n("P&ermissions"));
    ownershipLayout->addWidget(permissionsEnabled, 1, 0);

    QGroupBox *ownerGroup = new QGroupBox(ownershipGroup);
    QHBoxLayout *ownerHBox = new QHBoxLayout(ownerGroup);
    ownerGroup->setTitle(i18n("O&wner"));
    int width = 2 * height + height / 2;

    ownerR = new KComboBox(ownerGroup);
    ownerR->addItem(i18n("?"));
    ownerR->addItem(i18n("r"));
    ownerR->addItem(i18n("-"));
    ownerR->setEnabled(false);
    ownerR->setGeometry(QRect(10, 20, width, height + 6));
    ownerHBox->addWidget(ownerR);

    ownerW = new KComboBox(ownerGroup);
    ownerW->addItem(i18n("?"));
    ownerW->addItem(i18n("w"));
    ownerW->addItem(i18n("-"));
    ownerW->setEnabled(false);
    ownerW->setGeometry(QRect(10 + width, 20, width, height + 6));
    ownerHBox->addWidget(ownerW);

    ownerX = new KComboBox(ownerGroup);
    ownerX->addItem(i18n("?"));
    ownerX->addItem(i18n("x"));
    ownerX->addItem(i18n("-"));
    ownerX->setEnabled(false);
    ownerX->setGeometry(QRect(10 + 2*width, 20, width, height + 6));
    ownerHBox->addWidget(ownerX);

    ownershipLayout->addWidget(ownerGroup, 1, 1);

    QGroupBox *groupGroup = new QGroupBox(ownershipGroup);
    QHBoxLayout *groupHBox = new QHBoxLayout(groupGroup);
    groupGroup->setTitle(i18n("Grou&p"));

    groupR = new KComboBox(groupGroup);
    groupR->addItem(i18n("?"));
    groupR->addItem(i18n("r"));
    groupR->addItem(i18n("-"));
    groupR->setEnabled(false);
    groupR->setGeometry(QRect(10, 20, width, height + 6));
    groupHBox->addWidget(groupR);

    groupW = new KComboBox(groupGroup);
    groupW->addItem(i18n("?"));
    groupW->addItem(i18n("w"));
    groupW->addItem(i18n("-"));
    groupW->setEnabled(false);
    groupW->setGeometry(QRect(10 + width, 20, width, height + 6));
    groupHBox->addWidget(groupW);

    groupX = new KComboBox(groupGroup);
    groupX->addItem(i18n("?"));
    groupX->addItem(i18n("x"));
    groupX->addItem(i18n("-"));
    groupX->setEnabled(false);
    groupX->setGeometry(QRect(10 + 2*width, 20, width, height + 6));
    groupHBox->addWidget(groupX);

    ownershipLayout->addWidget(groupGroup, 1, 2);

    QGroupBox *allGroup = new QGroupBox(ownershipGroup);
    QHBoxLayout *allHBox = new QHBoxLayout(allGroup);
    allGroup->setTitle(i18n("A&ll"));

    allR = new KComboBox(allGroup);
    allR->addItem(i18n("?"));
    allR->addItem(i18n("r"));
    allR->addItem(i18n("-"));
    allR->setEnabled(false);
    allR->setGeometry(QRect(10, 20, width, height + 6));
    allHBox->addWidget(allR);

    allW = new KComboBox(allGroup);
    allW->addItem(i18n("?"));
    allW->addItem(i18n("w"));
    allW->addItem(i18n("-"));
    allW->setEnabled(false);
    allW->setGeometry(QRect(10 + width, 20, width, height + 6));
    allHBox->addWidget(allW);

    allX = new KComboBox(allGroup);
    allX->addItem(i18n("?"));
    allX->addItem(i18n("x"));
    allX->addItem(i18n("-"));
    allX->setEnabled(false);
    allX->setGeometry(QRect(10 + 2*width, 20, width, height + 6));
    allHBox->addWidget(allX);

    ownershipLayout->addWidget(allGroup, 1, 3);

    QLabel *infoLabel = new QLabel(ownershipGroup);
    QFont infoLabel_font(infoLabel->font());
    infoLabel_font.setFamily("adobe-helvetica");
    infoLabel_font.setItalic(true);
    infoLabel->setFont(infoLabel_font);
    infoLabel->setText(i18n("Note: a '?' is a wildcard"));

    ownershipLayout->addWidget(infoLabel, 2, 0, 1, 4, Qt::AlignRight);

    filterLayout->addWidget(ownershipGroup, 2, 0);

    // Connection table

    connect(minSizeEnabled, SIGNAL(toggled(bool)), minSizeAmount, SLOT(setEnabled(bool)));
    connect(minSizeEnabled, SIGNAL(toggled(bool)), minSizeType, SLOT(setEnabled(bool)));
    connect(maxSizeEnabled, SIGNAL(toggled(bool)), maxSizeAmount, SLOT(setEnabled(bool)));
    connect(maxSizeEnabled, SIGNAL(toggled(bool)), maxSizeType, SLOT(setEnabled(bool)));
    connect(modifiedBetweenEnabled, SIGNAL(toggled(bool)), modifiedBetweenData1, SLOT(setEnabled(bool)));
    connect(modifiedBetweenEnabled, SIGNAL(toggled(bool)), modifiedBetweenBtn1, SLOT(setEnabled(bool)));
    connect(modifiedBetweenEnabled, SIGNAL(toggled(bool)), modifiedBetweenData2, SLOT(setEnabled(bool)));
    connect(modifiedBetweenEnabled, SIGNAL(toggled(bool)), modifiedBetweenBtn2, SLOT(setEnabled(bool)));
    connect(notModifiedAfterEnabled, SIGNAL(toggled(bool)), notModifiedAfterData, SLOT(setEnabled(bool)));
    connect(notModifiedAfterEnabled, SIGNAL(toggled(bool)), notModifiedAfterBtn, SLOT(setEnabled(bool)));
    connect(modifiedInTheLastEnabled, SIGNAL(toggled(bool)), modifiedInTheLastData, SLOT(setEnabled(bool)));
    connect(modifiedInTheLastEnabled, SIGNAL(toggled(bool)), modifiedInTheLastType, SLOT(setEnabled(bool)));
    connect(modifiedInTheLastEnabled, SIGNAL(toggled(bool)), notModifiedInTheLastData, SLOT(setEnabled(bool)));
    connect(modifiedInTheLastEnabled, SIGNAL(toggled(bool)), notModifiedInTheLastType, SLOT(setEnabled(bool)));
    connect(belongsToUserEnabled, SIGNAL(toggled(bool)), belongsToUserData, SLOT(setEnabled(bool)));
    connect(belongsToGroupEnabled, SIGNAL(toggled(bool)), belongsToGroupData, SLOT(setEnabled(bool)));
    connect(permissionsEnabled, SIGNAL(toggled(bool)), ownerR, SLOT(setEnabled(bool)));
    connect(permissionsEnabled, SIGNAL(toggled(bool)), ownerW, SLOT(setEnabled(bool)));
    connect(permissionsEnabled, SIGNAL(toggled(bool)), ownerX, SLOT(setEnabled(bool)));
    connect(permissionsEnabled, SIGNAL(toggled(bool)), groupR, SLOT(setEnabled(bool)));
    connect(permissionsEnabled, SIGNAL(toggled(bool)), groupW, SLOT(setEnabled(bool)));
    connect(permissionsEnabled, SIGNAL(toggled(bool)), groupX, SLOT(setEnabled(bool)));
    connect(permissionsEnabled, SIGNAL(toggled(bool)), allR, SLOT(setEnabled(bool)));
    connect(permissionsEnabled, SIGNAL(toggled(bool)), allW, SLOT(setEnabled(bool)));
    connect(permissionsEnabled, SIGNAL(toggled(bool)), allX, SLOT(setEnabled(bool)));
    connect(modifiedBetweenBtn1, SIGNAL(clicked()), this, SLOT(modifiedBetweenSetDate1()));
    connect(modifiedBetweenBtn2, SIGNAL(clicked()), this, SLOT(modifiedBetweenSetDate2()));
    connect(notModifiedAfterBtn, SIGNAL(clicked()), this, SLOT(notModifiedAfterSetDate()));

    // fill the users and groups list

    fillList(belongsToUserData, USERSFILE);
    fillList(belongsToGroupData, GROUPSFILE);

    // tab order
    setTabOrder(minSizeEnabled, minSizeAmount);
    setTabOrder(minSizeAmount, maxSizeEnabled);
    setTabOrder(maxSizeEnabled, maxSizeAmount);
    setTabOrder(maxSizeAmount, modifiedBetweenEnabled);
    setTabOrder(modifiedBetweenEnabled, modifiedBetweenData1);
    setTabOrder(modifiedBetweenData1, modifiedBetweenData2);
    setTabOrder(modifiedBetweenData2, notModifiedAfterEnabled);
    setTabOrder(notModifiedAfterEnabled, notModifiedAfterData);
    setTabOrder(notModifiedAfterData, modifiedInTheLastEnabled);
    setTabOrder(modifiedInTheLastEnabled, modifiedInTheLastData);
    setTabOrder(modifiedInTheLastData, notModifiedInTheLastData);
    setTabOrder(notModifiedInTheLastData, belongsToUserEnabled);
    setTabOrder(belongsToUserEnabled, belongsToUserData);
    setTabOrder(belongsToUserData, belongsToGroupEnabled);
    setTabOrder(belongsToGroupEnabled, belongsToGroupData);
    setTabOrder(belongsToGroupData, permissionsEnabled);
    setTabOrder(permissionsEnabled, ownerR);
    setTabOrder(ownerR, ownerW);
    setTabOrder(ownerW, ownerX);
    setTabOrder(ownerX, groupR);
    setTabOrder(groupR, groupW);
    setTabOrder(groupW, groupX);
    setTabOrder(groupX, allR);
    setTabOrder(allR, allW);
    setTabOrder(allW, allX);
    setTabOrder(allX, minSizeType);
    setTabOrder(minSizeType, maxSizeType);
    setTabOrder(maxSizeType, modifiedInTheLastType);
    setTabOrder(modifiedInTheLastType, notModifiedInTheLastType);
}

void AdvancedFilter::modifiedBetweenSetDate1()
{
    changeDate(modifiedBetweenData1);
}

void AdvancedFilter::modifiedBetweenSetDate2()
{
    changeDate(modifiedBetweenData2);
}

void AdvancedFilter::notModifiedAfterSetDate()
{
    changeDate(notModifiedAfterData);
}

void AdvancedFilter::changeDate(KLineEdit *p)
{
    // check if the current date is valid
    QDate d = KGlobal::locale()->readDate(p->text());
    if (!d.isValid()) d = QDate::currentDate();

    KRGetDate *gd = new KRGetDate(d, this);
    d = gd->getDate();
    // if a user pressed ESC or closed the dialog, we'll return an invalid date
    if (d.isValid())
        p->setText(KGlobal::locale()->formatDate(d, KLocale::ShortDate));
    delete gd;
}

void AdvancedFilter::fillList(KComboBox *list, QString filename)
{
    QFile data(filename);
    if (!data.open(QIODevice::ReadOnly)) {
        krOut << "Search: Unable to read " << filename << " !!!" << endl;
        return;
    }
    // and read it into the temporary array
    QTextStream t(&data);
    while (!t.atEnd()) {
        QString s = t.readLine();
        QString name = s.left(s.indexOf(':'));
        if (!name.startsWith('#'))
            list->addItem(name);
    }
}

void AdvancedFilter::invalidDateMessage(KLineEdit *p)
{
    // FIXME p->text() is empty sometimes (to reproduce, set date to "13.09.005")
    KMessageBox::detailedError(this, i18n("Invalid date entered."),
                               i18n("The date %1 is not valid according to your locale. Please re-enter a valid date (use the date button for easy access).", p->text()));
    p->setFocus();
}

bool AdvancedFilter::getSettings(FilterSettings &s)
{
    s.minSizeEnabled =  minSizeEnabled->isChecked();
    s.minSize.amount = minSizeAmount->text().toULong();
    s.minSize.unit = static_cast<FilterSettings::SizeUnit>(minSizeType->currentIndex());

    s.maxSizeEnabled = maxSizeEnabled->isChecked();
    s.maxSize.amount = maxSizeAmount->text().toULong();
    s.maxSize.unit = static_cast<FilterSettings::SizeUnit>(maxSizeType->currentIndex());

    if (s.minSize.size() && s.maxSize.size() && (s.maxSize.size() < s.minSize.size())) {
        KMessageBox::detailedError(this, i18n("Specified sizes are inconsistent!"),
                            i18n("Please re-enter the values, so that the left side size "
                                 "will be smaller than (or equal to) the right side size."));
        minSizeAmount->setFocus();
        return false;
    }

    s.modifiedBetweenEnabled = modifiedBetweenEnabled->isChecked();
    s.modifiedBetween1 = KGlobal::locale()->readDate(modifiedBetweenData1->text());
    s.modifiedBetween2 = KGlobal::locale()->readDate(modifiedBetweenData2->text());

    if (s.modifiedBetweenEnabled) {
        // check if date is valid
        if (!s.modifiedBetween1.isValid()) {
            invalidDateMessage(modifiedBetweenData1);
            return false;
        } else if (!s.modifiedBetween2.isValid()) {
            invalidDateMessage(modifiedBetweenData2);
            return false;
        } else if (s.modifiedBetween1 > s.modifiedBetween2) {
            KMessageBox::detailedError(this, i18n("Dates are inconsistent!"),
                                i18n("The date on the left is later than the date on the right. "
                                     "Please re-enter the dates, so that the left side date "
                                     "will be earlier than the right side date."));
            modifiedBetweenData1->setFocus();
            return false;
        }
    }

    s.notModifiedAfterEnabled = notModifiedAfterEnabled->isChecked();
    s.notModifiedAfter = KGlobal::locale()->readDate(notModifiedAfterData->text());

    if(s.notModifiedAfterEnabled && !s.notModifiedAfter.isValid()) {
        invalidDateMessage(notModifiedAfterData);
        return false;
    }

    s.modifiedInTheLastEnabled = modifiedInTheLastEnabled->isChecked();
    s.modifiedInTheLast.amount = modifiedInTheLastData->text().toInt();
    s.modifiedInTheLast.unit =
        static_cast<FilterSettings::TimeUnit>(modifiedInTheLastType->currentIndex());
    s.notModifiedInTheLast.amount = notModifiedInTheLastData->text().toInt();
    s.notModifiedInTheLast.unit =
        static_cast<FilterSettings::TimeUnit>(notModifiedInTheLastType->currentIndex());

    if (s.modifiedInTheLast.amount && s.notModifiedInTheLast.amount) {
        if (s.modifiedInTheLast.days() < s.notModifiedInTheLast.days()) {
            KMessageBox::detailedError(this, i18n("Dates are inconsistent!"),
                                i18n("The date on top is later than the date on the bottom. "
                                     "Please re-enter the dates, so that the top date "
                                     "will be earlier than the bottom date."));
            modifiedInTheLastData->setFocus();
            return false;
        }
    }

    s.ownerEnabled = belongsToUserEnabled->isChecked();
    s.owner = belongsToUserData->currentText();

    s.groupEnabled = belongsToGroupEnabled->isChecked();
    s.group = belongsToGroupData->currentText();

    s.permissionsEnabled = permissionsEnabled->isChecked();
    s.permissions = ownerR->currentText() + ownerW->currentText() + ownerX->currentText() +
                    groupR->currentText() + groupW->currentText() + groupX->currentText() +
                    allR->currentText()   + allW->currentText()   + allX->currentText();

    return true;
}

void AdvancedFilter::applySettings(const FilterSettings &s)
{
    minSizeEnabled->setChecked(s.minSizeEnabled);
    minSizeAmount->setText(QString::number(s.minSize.amount));
    minSizeType->setCurrentIndex(s.minSize.unit);

    maxSizeEnabled->setChecked(s.maxSizeEnabled);
    maxSizeAmount->setText(QString::number(s.maxSize.amount));
    maxSizeType->setCurrentIndex(s.maxSize.unit);

    modifiedBetweenEnabled->setChecked(s.modifiedBetweenEnabled);
    modifiedBetweenData1->setText(
        KGlobal::locale()->formatDate(s.modifiedBetween1, KLocale::ShortDate));
    modifiedBetweenData2->setText(
        KGlobal::locale()->formatDate(s.modifiedBetween2, KLocale::ShortDate));

    notModifiedAfterEnabled->setChecked(s.notModifiedAfterEnabled);
    notModifiedAfterData->setText(
        KGlobal::locale()->formatDate(s.notModifiedAfter, KLocale::ShortDate));

    modifiedInTheLastEnabled->setChecked(s.modifiedInTheLastEnabled);
    modifiedInTheLastData->setText(QString::number(s.modifiedInTheLast.amount));
    modifiedInTheLastType->setCurrentIndex(s.modifiedInTheLast.unit);
    notModifiedInTheLastData->setText(QString::number(s.notModifiedInTheLast.amount));
    notModifiedInTheLastType->setCurrentIndex(s.notModifiedInTheLast.unit);

    belongsToUserEnabled->setChecked(s.ownerEnabled);
    setComboBoxValue(belongsToUserData, s.owner);

    belongsToGroupEnabled->setChecked(s.groupEnabled);
    setComboBoxValue(belongsToGroupData, s.group);

    permissionsEnabled->setChecked(s.permissionsEnabled);
    QString perm = s.permissions;
    if (perm.length() != 9)
        perm = "?????????";
    setComboBoxValue(ownerR, QString(perm[0]));
    setComboBoxValue(ownerW, QString(perm[1]));
    setComboBoxValue(ownerX, QString(perm[2]));
    setComboBoxValue(groupR, QString(perm[3]));
    setComboBoxValue(groupW, QString(perm[4]));
    setComboBoxValue(groupX, QString(perm[5]));
    setComboBoxValue(allR, QString(perm[6]));
    setComboBoxValue(allW, QString(perm[7]));
    setComboBoxValue(allX, QString(perm[8]));
}

#include "advancedfilter.moc"
