/*
    SPDX-FileCopyrightText: 2003 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2003 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "advancedfilter.h"

// QtCore
#include <QDebug>
#include <QFile>
#include <QTextStream>
// QtGui
#include <QPixmap>
// QtWidgets
#include <QButtonGroup>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QSpinBox>

#include <KLocalizedString>
#include <KMessageBox>

#include "../Dialogs/krdialogs.h"
#include "../icon.h"
#include "../krglobal.h"

#define USERSFILE QString("/etc/passwd")
#define GROUPSFILE QString("/etc/group")

AdvancedFilter::AdvancedFilter(FilterTabs *tabs, QWidget *parent)
    : QWidget(parent)
    , fltTabs(tabs)
{
    auto *filterLayout = new QGridLayout(this);
    filterLayout->setSpacing(6);
    filterLayout->setContentsMargins(11, 11, 11, 11);

    // Options for size

    auto *sizeGroup = new QGroupBox(this);
    QSizePolicy sizeGroupPolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    sizeGroupPolicy.setHeightForWidth(sizeGroup->sizePolicy().hasHeightForWidth());
    sizeGroup->setSizePolicy(sizeGroupPolicy);
    sizeGroup->setTitle(i18n("Size"));
    auto *sizeLayout = new QGridLayout(sizeGroup);
    sizeLayout->setAlignment(Qt::AlignTop);
    sizeLayout->setSpacing(6);
    sizeLayout->setContentsMargins(11, 11, 11, 11);

    minSizeEnabled = new QCheckBox(sizeGroup);
    minSizeEnabled->setText(i18n("At Least"));
    minSizeEnabled->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    sizeLayout->addWidget(minSizeEnabled, 0, 0);

    minSizeAmount = new QSpinBox(sizeGroup);
    minSizeAmount->setRange(0, 999999999);
    minSizeAmount->setEnabled(false);
    sizeLayout->addWidget(minSizeAmount, 0, 1);

    minSizeType = new KComboBox(false, sizeGroup);
    // i18n: @item:inlistbox next to a text field containing the amount
    minSizeType->addItem(i18n("Byte"));
    // i18n: @item:inlistbox next to a text field containing the amount
    minSizeType->addItem(i18n("KiB"));
    // i18n: @item:inlistbox next to a text field containing the amount
    minSizeType->addItem(i18n("MiB"));
    // i18n: @item:inlistbox next to a text field containing the amount
    minSizeType->addItem(i18n("GiB"));
    minSizeType->setEnabled(false);
    sizeLayout->addWidget(minSizeType, 0, 2);

    maxSizeEnabled = new QCheckBox(sizeGroup);
    maxSizeEnabled->setText(i18n("At Most"));
    maxSizeEnabled->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
    sizeLayout->addWidget(maxSizeEnabled, 0, 3);

    maxSizeAmount = new QSpinBox(sizeGroup);
    maxSizeAmount->setRange(0, 999999999);
    maxSizeAmount->setEnabled(false);
    sizeLayout->addWidget(maxSizeAmount, 0, 4);

    maxSizeType = new KComboBox(false, sizeGroup);
    // i18n: @item:inlistbox next to a text field containing the amount
    maxSizeType->addItem(i18n("Byte"));
    // i18n: @item:inlistbox next to a text field containing the amount
    maxSizeType->addItem(i18n("KiB"));
    // i18n: @item:inlistbox next to a text field containing the amount
    maxSizeType->addItem(i18n("MiB"));
    // i18n: @item:inlistbox next to a text field containing the amount
    maxSizeType->addItem(i18n("GiB"));
    maxSizeType->setEnabled(false);
    sizeLayout->addWidget(maxSizeType, 0, 5);

    filterLayout->addWidget(sizeGroup, 0, 0);

    // Options for date

    Icon iconDate("view-calendar");

    auto *dateGroup = new QGroupBox(this);
    auto *btnGroup = new QButtonGroup(dateGroup);
    dateGroup->setTitle(i18n("Date"));
    btnGroup->setExclusive(true);
    auto *dateLayout = new QGridLayout(dateGroup);
    dateLayout->setAlignment(Qt::AlignTop);
    dateLayout->setSpacing(6);
    dateLayout->setContentsMargins(11, 11, 11, 11);

    anyDateEnabled = new QRadioButton(dateGroup);
    anyDateEnabled->setText(i18n("Any date"));
    btnGroup->addButton(anyDateEnabled);
    anyDateEnabled->setChecked(true);

    modifiedBetweenEnabled = new QRadioButton(dateGroup);
    modifiedBetweenEnabled->setText(i18n("&Modified between"));
    btnGroup->addButton(modifiedBetweenEnabled);

    modifiedBetweenData1 = new KLineEdit(dateGroup);
    modifiedBetweenData1->setEnabled(false);
    modifiedBetweenData1->setText("");

    modifiedBetweenBtn1 = new QToolButton(dateGroup);
    modifiedBetweenBtn1->setEnabled(false);
    modifiedBetweenBtn1->setText("");
    modifiedBetweenBtn1->setIcon(iconDate);

    QLabel *andLabel = new QLabel(dateGroup);
    andLabel->setText(i18n("an&d"));

    modifiedBetweenData2 = new KLineEdit(dateGroup);
    modifiedBetweenData2->setEnabled(false);
    modifiedBetweenData2->setText("");
    andLabel->setBuddy(modifiedBetweenData2);

    modifiedBetweenBtn2 = new QToolButton(dateGroup);
    modifiedBetweenBtn2->setEnabled(false);
    modifiedBetweenBtn2->setText("");
    modifiedBetweenBtn2->setIcon(iconDate);

    notModifiedAfterEnabled = new QRadioButton(dateGroup);
    notModifiedAfterEnabled->setText(i18n("&Not modified after"));
    btnGroup->addButton(notModifiedAfterEnabled);

    notModifiedAfterData = new KLineEdit(dateGroup);
    notModifiedAfterData->setEnabled(false);
    notModifiedAfterData->setText("");

    notModifiedAfterBtn = new QToolButton(dateGroup);
    notModifiedAfterBtn->setEnabled(false);
    notModifiedAfterBtn->setText("");
    notModifiedAfterBtn->setIcon(iconDate);

    modifiedInTheLastEnabled = new QRadioButton(dateGroup);
    modifiedInTheLastEnabled->setText(i18n("Mod&ified in the last"));
    btnGroup->addButton(modifiedInTheLastEnabled);

    modifiedInTheLastData = new QSpinBox(dateGroup);
    modifiedInTheLastData->setRange(0, 99999);
    modifiedInTheLastData->setEnabled(false);

    modifiedInTheLastType = new KComboBox(dateGroup);
    modifiedInTheLastType->addItem(i18n("days"));
    modifiedInTheLastType->addItem(i18n("weeks"));
    modifiedInTheLastType->addItem(i18n("months"));
    modifiedInTheLastType->addItem(i18n("years"));
    modifiedInTheLastType->setEnabled(false);

    notModifiedInTheLastData = new QSpinBox(dateGroup);
    notModifiedInTheLastData->setRange(0, 99999);
    notModifiedInTheLastData->setEnabled(false);

    QLabel *notModifiedInTheLastLbl = new QLabel(dateGroup);
    notModifiedInTheLastLbl->setText(i18n("No&t modified in the last"));
    notModifiedInTheLastLbl->setBuddy(notModifiedInTheLastData);

    notModifiedInTheLastType = new KComboBox(dateGroup);
    notModifiedInTheLastType->addItem(i18n("days"));
    notModifiedInTheLastType->addItem(i18n("weeks"));
    notModifiedInTheLastType->addItem(i18n("months"));
    notModifiedInTheLastType->addItem(i18n("years"));
    notModifiedInTheLastType->setEnabled(false);

    // Date options layout

    dateLayout->addWidget(anyDateEnabled, 0, 0);

    dateLayout->addWidget(modifiedBetweenEnabled, 1, 0);
    dateLayout->addWidget(modifiedBetweenData1, 1, 1);
    dateLayout->addWidget(modifiedBetweenBtn1, 1, 2);
    dateLayout->addWidget(andLabel, 1, 3);
    dateLayout->addWidget(modifiedBetweenData2, 1, 4);
    dateLayout->addWidget(modifiedBetweenBtn2, 1, 5);

    dateLayout->addWidget(notModifiedAfterEnabled, 2, 0);
    dateLayout->addWidget(notModifiedAfterData, 2, 1);
    dateLayout->addWidget(notModifiedAfterBtn, 2, 2);

    dateLayout->addWidget(modifiedInTheLastEnabled, 3, 0);
    auto *modifiedInTheLastLayout = new QHBoxLayout();
    modifiedInTheLastLayout->addWidget(modifiedInTheLastData);
    modifiedInTheLastLayout->addWidget(modifiedInTheLastType);
    dateLayout->addLayout(modifiedInTheLastLayout, 3, 1);

    dateLayout->addWidget(notModifiedInTheLastLbl, 4, 0);
    modifiedInTheLastLayout = new QHBoxLayout();
    modifiedInTheLastLayout->addWidget(notModifiedInTheLastData);
    modifiedInTheLastLayout->addWidget(notModifiedInTheLastType);
    dateLayout->addLayout(modifiedInTheLastLayout, 4, 1);

    filterLayout->addWidget(dateGroup, 1, 0);

    // Options for ownership

    auto *ownershipGroup = new QGroupBox(this);
    ownershipGroup->setTitle(i18n("Ownership"));
    auto *ownershipLayout = new QGridLayout(ownershipGroup);
    ownershipLayout->setAlignment(Qt::AlignTop);
    ownershipLayout->setSpacing(6);
    ownershipLayout->setContentsMargins(11, 11, 11, 11);

    auto *hboxLayout = new QHBoxLayout();
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

    auto *ownerGroup = new QGroupBox(ownershipGroup);
    auto *ownerHBox = new QHBoxLayout(ownerGroup);
    ownerGroup->setTitle(i18n("O&wner"));

    ownerR = new KComboBox(ownerGroup);
    ownerR->addItem(i18n("?"));
    ownerR->addItem(i18n("r"));
    ownerR->addItem(i18n("-"));
    ownerR->setEnabled(false);
    ownerHBox->addWidget(ownerR);

    ownerW = new KComboBox(ownerGroup);
    ownerW->addItem(i18n("?"));
    ownerW->addItem(i18n("w"));
    ownerW->addItem(i18n("-"));
    ownerW->setEnabled(false);
    ownerHBox->addWidget(ownerW);

    ownerX = new KComboBox(ownerGroup);
    ownerX->addItem(i18n("?"));
    ownerX->addItem(i18n("x"));
    ownerX->addItem(i18n("-"));
    ownerX->setEnabled(false);
    ownerHBox->addWidget(ownerX);

    ownershipLayout->addWidget(ownerGroup, 1, 1);

    auto *groupGroup = new QGroupBox(ownershipGroup);
    auto *groupHBox = new QHBoxLayout(groupGroup);
    groupGroup->setTitle(i18n("Grou&p"));

    groupR = new KComboBox(groupGroup);
    groupR->addItem(i18n("?"));
    groupR->addItem(i18n("r"));
    groupR->addItem(i18n("-"));
    groupR->setEnabled(false);
    groupHBox->addWidget(groupR);

    groupW = new KComboBox(groupGroup);
    groupW->addItem(i18n("?"));
    groupW->addItem(i18n("w"));
    groupW->addItem(i18n("-"));
    groupW->setEnabled(false);
    groupHBox->addWidget(groupW);

    groupX = new KComboBox(groupGroup);
    groupX->addItem(i18n("?"));
    groupX->addItem(i18n("x"));
    groupX->addItem(i18n("-"));
    groupX->setEnabled(false);
    groupHBox->addWidget(groupX);

    ownershipLayout->addWidget(groupGroup, 1, 2);

    auto *allGroup = new QGroupBox(ownershipGroup);
    auto *allHBox = new QHBoxLayout(allGroup);
    allGroup->setTitle(i18n("A&ll"));

    allR = new KComboBox(allGroup);
    allR->addItem(i18n("?"));
    allR->addItem(i18n("r"));
    allR->addItem(i18n("-"));
    allR->setEnabled(false);
    allHBox->addWidget(allR);

    allW = new KComboBox(allGroup);
    allW->addItem(i18n("?"));
    allW->addItem(i18n("w"));
    allW->addItem(i18n("-"));
    allW->setEnabled(false);
    allHBox->addWidget(allW);

    allX = new KComboBox(allGroup);
    allX->addItem(i18n("?"));
    allX->addItem(i18n("x"));
    allX->addItem(i18n("-"));
    allX->setEnabled(false);
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

    connect(minSizeEnabled, &QCheckBox::toggled, minSizeAmount, &QSpinBox::setEnabled);
    connect(minSizeEnabled, &QCheckBox::toggled, minSizeType, &KComboBox::setEnabled);
    connect(maxSizeEnabled, &QCheckBox::toggled, maxSizeAmount, &QSpinBox::setEnabled);
    connect(maxSizeEnabled, &QCheckBox::toggled, maxSizeType, &KComboBox::setEnabled);
    connect(modifiedBetweenEnabled, &QRadioButton::toggled, modifiedBetweenData1, &KLineEdit::setEnabled);
    connect(modifiedBetweenEnabled, &QRadioButton::toggled, modifiedBetweenBtn1, &QToolButton::setEnabled);
    connect(modifiedBetweenEnabled, &QRadioButton::toggled, modifiedBetweenData2, &KLineEdit::setEnabled);
    connect(modifiedBetweenEnabled, &QRadioButton::toggled, modifiedBetweenBtn2, &QToolButton::setEnabled);
    connect(notModifiedAfterEnabled, &QRadioButton::toggled, notModifiedAfterData, &KLineEdit::setEnabled);
    connect(notModifiedAfterEnabled, &QRadioButton::toggled, notModifiedAfterBtn, &QToolButton::setEnabled);
    connect(modifiedInTheLastEnabled, &QRadioButton::toggled, modifiedInTheLastData, &QSpinBox::setEnabled);
    connect(modifiedInTheLastEnabled, &QRadioButton::toggled, modifiedInTheLastType, &KComboBox::setEnabled);
    connect(modifiedInTheLastEnabled, &QRadioButton::toggled, notModifiedInTheLastData, &QSpinBox::setEnabled);
    connect(modifiedInTheLastEnabled, &QRadioButton::toggled, notModifiedInTheLastType, &KComboBox::setEnabled);
    connect(belongsToUserEnabled, &QCheckBox::toggled, belongsToUserData, &KComboBox::setEnabled);
    connect(belongsToGroupEnabled, &QCheckBox::toggled, belongsToGroupData, &KComboBox::setEnabled);
    connect(permissionsEnabled, &QCheckBox::toggled, ownerR, &KComboBox::setEnabled);
    connect(permissionsEnabled, &QCheckBox::toggled, ownerW, &KComboBox::setEnabled);
    connect(permissionsEnabled, &QCheckBox::toggled, ownerX, &KComboBox::setEnabled);
    connect(permissionsEnabled, &QCheckBox::toggled, groupR, &KComboBox::setEnabled);
    connect(permissionsEnabled, &QCheckBox::toggled, groupW, &KComboBox::setEnabled);
    connect(permissionsEnabled, &QCheckBox::toggled, groupX, &KComboBox::setEnabled);
    connect(permissionsEnabled, &QCheckBox::toggled, allR, &KComboBox::setEnabled);
    connect(permissionsEnabled, &QCheckBox::toggled, allW, &KComboBox::setEnabled);
    connect(permissionsEnabled, &QCheckBox::toggled, allX, &KComboBox::setEnabled);
    connect(modifiedBetweenBtn1, &QToolButton::clicked, this, &AdvancedFilter::modifiedBetweenSetDate1);
    connect(modifiedBetweenBtn2, &QToolButton::clicked, this, &AdvancedFilter::modifiedBetweenSetDate2);
    connect(notModifiedAfterBtn, &QToolButton::clicked, this, &AdvancedFilter::notModifiedAfterSetDate);

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
    QDate d = stringToDate(p->text());
    if (!d.isValid())
        d = QDate::currentDate();

    auto *gd = new KrGetDate(d, this);
    d = gd->getDate();
    // if a user pressed ESC or closed the dialog, we'll return an invalid date
    if (d.isValid())
        p->setText(dateToString(d));
    delete gd;
}

void AdvancedFilter::fillList(KComboBox *list, const QString &filename)
{
    QFile data(filename);
    if (!data.open(QIODevice::ReadOnly)) {
        qWarning() << "Search: Unable to read " << filename << " !!!";
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
    KMessageBox::detailedError(
        this,
        i18n("Invalid date entered."),
        i18n("The date %1 is not valid according to your locale. Please re-enter a valid date (use the date button for easy access).", p->text()));
    p->setFocus();
}

bool AdvancedFilter::getSettings(FilterSettings &s)
{
    s.minSizeEnabled = minSizeEnabled->isChecked();
    s.minSize.amount = minSizeAmount->value();
    s.minSize.unit = static_cast<FilterSettings::SizeUnit>(minSizeType->currentIndex());

    s.maxSizeEnabled = maxSizeEnabled->isChecked();
    s.maxSize.amount = maxSizeAmount->value();
    s.maxSize.unit = static_cast<FilterSettings::SizeUnit>(maxSizeType->currentIndex());

    if (s.minSizeEnabled && s.maxSizeEnabled && (s.maxSize.size() < s.minSize.size())) {
        KMessageBox::detailedError(this,
                                   i18n("Specified sizes are inconsistent."),
                                   i18n("Please re-enter the values, so that the left side size "
                                        "will be smaller than (or equal to) the right side size."));
        minSizeAmount->setFocus();
        return false;
    }

    s.modifiedBetweenEnabled = modifiedBetweenEnabled->isChecked();
    s.modifiedBetween1 = stringToDate(modifiedBetweenData1->text());
    s.modifiedBetween2 = stringToDate(modifiedBetweenData2->text());

    if (s.modifiedBetweenEnabled) {
        // check if date is valid
        if (!s.modifiedBetween1.isValid()) {
            invalidDateMessage(modifiedBetweenData1);
            return false;
        } else if (!s.modifiedBetween2.isValid()) {
            invalidDateMessage(modifiedBetweenData2);
            return false;
        } else if (s.modifiedBetween1 > s.modifiedBetween2) {
            KMessageBox::detailedError(this,
                                       i18n("Dates are inconsistent."),
                                       i18n("The date on the left is later than the date on the right. "
                                            "Please re-enter the dates, so that the left side date "
                                            "will be earlier than the right side date."));
            modifiedBetweenData1->setFocus();
            return false;
        }
    }

    s.notModifiedAfterEnabled = notModifiedAfterEnabled->isChecked();
    s.notModifiedAfter = stringToDate(notModifiedAfterData->text());

    if (s.notModifiedAfterEnabled && !s.notModifiedAfter.isValid()) {
        invalidDateMessage(notModifiedAfterData);
        return false;
    }

    s.modifiedInTheLastEnabled = modifiedInTheLastEnabled->isChecked();
    s.modifiedInTheLast.amount = modifiedInTheLastData->value();
    s.modifiedInTheLast.unit = static_cast<FilterSettings::TimeUnit>(modifiedInTheLastType->currentIndex());
    s.notModifiedInTheLast.amount = notModifiedInTheLastData->value();
    s.notModifiedInTheLast.unit = static_cast<FilterSettings::TimeUnit>(notModifiedInTheLastType->currentIndex());

    if (s.modifiedInTheLastEnabled && s.modifiedInTheLast.amount && s.notModifiedInTheLast.amount) {
        if (s.modifiedInTheLast.days() < s.notModifiedInTheLast.days()) {
            KMessageBox::detailedError(this,
                                       i18n("Dates are inconsistent."),
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

    // clang-format off
    s.permissions = ownerR->currentText() + ownerW->currentText() + ownerX->currentText() +
                    groupR->currentText() + groupW->currentText() + groupX->currentText() +
                    allR->currentText()   + allW->currentText()   + allX->currentText();
    // clang-format on

    return true;
}

void AdvancedFilter::applySettings(const FilterSettings &s)
{
    minSizeEnabled->setChecked(s.minSizeEnabled);
    minSizeAmount->setValue(static_cast<int>(s.minSize.amount));
    minSizeType->setCurrentIndex(s.minSize.unit);

    maxSizeEnabled->setChecked(s.maxSizeEnabled);
    maxSizeAmount->setValue(static_cast<int>(s.maxSize.amount));
    maxSizeType->setCurrentIndex(s.maxSize.unit);

    if (s.modifiedBetweenEnabled)
        modifiedBetweenEnabled->setChecked(true);
    else if (s.notModifiedAfterEnabled)
        notModifiedAfterEnabled->setChecked(true);
    else if (s.modifiedInTheLastEnabled)
        modifiedInTheLastEnabled->setChecked(true);
    else
        anyDateEnabled->setChecked(true);

    modifiedBetweenData1->setText(dateToString(s.modifiedBetween1));
    modifiedBetweenData2->setText(dateToString(s.modifiedBetween2));

    notModifiedAfterData->setText(dateToString(s.notModifiedAfter));

    modifiedInTheLastData->setValue(s.modifiedInTheLast.amount);
    modifiedInTheLastType->setCurrentIndex(s.modifiedInTheLast.unit);
    notModifiedInTheLastData->setValue(s.notModifiedInTheLast.amount);
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
