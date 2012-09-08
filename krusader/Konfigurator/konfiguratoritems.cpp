/***************************************************************************
                     konfiguratoritems.cpp  -  description
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

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "konfiguratoritems.h"
#include "../krglobal.h"
#include <klocale.h>
#include <klineedit.h>
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QColorDialog>
#include <QPixmap>
#include <QLabel>
#include <kiconloader.h>

KonfiguratorExtension::KonfiguratorExtension(QObject *obj, QString cfgClass, QString cfgName, bool rst, int pg) :
        QObject(), objectPtr(obj), applyConnected(false), setDefaultsConnected(false),
        changed(false), restartNeeded(rst), subpage(pg), configClass(cfgClass), configName(cfgName)
{
}

void KonfiguratorExtension::connectNotify(const char *signal)
{
    QString signalString = QString(signal).remove(' ');
    QString applyString = QString(SIGNAL(applyManually(QObject *, QString, QString))).remove(' ');
    QString defaultsString = QString(SIGNAL(setDefaultsManually(QObject *))).remove(' ');

    if (signalString == applyString)
        applyConnected = true;
    else if (signalString == defaultsString)
        setDefaultsConnected = true;

    QObject::connectNotify(signal);
}

bool KonfiguratorExtension::apply()
{
    if (!changed)
        return false;

    if (applyConnected)
        emit applyManually(objectPtr, configClass, configName);
    else
        emit applyAuto(objectPtr, configClass, configName);

    setChanged(false);
    return restartNeeded;
}

void KonfiguratorExtension::setDefaults()
{
    if (setDefaultsConnected)
        emit setDefaultsManually(objectPtr);
    else
        emit setDefaultsAuto(objectPtr);
}

void KonfiguratorExtension::loadInitialValue()
{
    emit setInitialValue(objectPtr);
}

bool KonfiguratorExtension::isChanged()
{
    return changed;
}

// KonfiguratorCheckBox class
///////////////////////////////

KonfiguratorCheckBox::KonfiguratorCheckBox(QString cls, QString name, bool dflt, QString text,
        QWidget *parent, bool rst, int pg) : QCheckBox(text, parent),
        defaultValue(dflt)
{
    ext = new KonfiguratorExtension(this, cls, name, rst, pg);
    connect(ext, SIGNAL(applyAuto(QObject *, QString, QString)), this, SLOT(slotApply(QObject *, QString, QString)));
    connect(ext, SIGNAL(setDefaultsAuto(QObject *)), this, SLOT(slotSetDefaults(QObject *)));
    connect(ext, SIGNAL(setInitialValue(QObject *)), this, SLOT(loadInitialValue()));

    connect(this, SIGNAL(stateChanged(int)), ext, SLOT(setChanged()));
    loadInitialValue();
}

KonfiguratorCheckBox::~KonfiguratorCheckBox()
{
    delete ext;
}

void KonfiguratorCheckBox::loadInitialValue()
{
    KConfigGroup group(krConfig, ext->getCfgClass());
    setChecked(group.readEntry(ext->getCfgName(), defaultValue));
    ext->setChanged(false);
}

void KonfiguratorCheckBox::checkStateSet()
{
    QCheckBox::checkStateSet();
    updateDeps();
}

void KonfiguratorCheckBox::nextCheckState()
{
    QCheckBox::nextCheckState();
    updateDeps();
}

void KonfiguratorCheckBox::addDep(KonfiguratorCheckBox *dep)
{
    deps << dep;
    dep->setEnabled(isChecked());
}

void KonfiguratorCheckBox::updateDeps()
{
    foreach(KonfiguratorCheckBox *dep, deps)
        dep->setEnabled(isChecked());
}

void KonfiguratorCheckBox::slotApply(QObject *, QString cls, QString name)
{
    KConfigGroup(krConfig, cls).writeEntry(name, isChecked());
}

void KonfiguratorCheckBox::slotSetDefaults(QObject *)
{
    if (isChecked() != defaultValue)
        setChecked(defaultValue);
}


// KonfiguratorSpinBox class
///////////////////////////////

KonfiguratorSpinBox::KonfiguratorSpinBox(QString cls, QString name, int dflt, int min, int max,
        QWidget *parent, bool rst, int pg) : QSpinBox(parent),
        defaultValue(dflt)
{
    ext = new KonfiguratorExtension(this, cls, name, rst, pg);
    connect(ext, SIGNAL(applyAuto(QObject *, QString, QString)), this, SLOT(slotApply(QObject *, QString, QString)));
    connect(ext, SIGNAL(setDefaultsAuto(QObject *)), this, SLOT(slotSetDefaults(QObject *)));
    connect(ext, SIGNAL(setInitialValue(QObject *)), this, SLOT(loadInitialValue()));

    connect(this, SIGNAL(valueChanged(int)), ext, SLOT(setChanged()));

    setMinimum(min);
    setMaximum(max);

    loadInitialValue();
}

KonfiguratorSpinBox::~KonfiguratorSpinBox()
{
    delete ext;
}

void KonfiguratorSpinBox::loadInitialValue()
{
    KConfigGroup group(krConfig, ext->getCfgClass());
    setValue(group.readEntry(ext->getCfgName(), defaultValue));
    ext->setChanged(false);
}

void KonfiguratorSpinBox::slotApply(QObject *, QString cls, QString name)
{
    KConfigGroup(krConfig, cls).writeEntry(name, value());
}

void KonfiguratorSpinBox::slotSetDefaults(QObject *)
{
    if (value() != defaultValue)
        setValue(defaultValue);
}

// KonfiguratorCheckBoxGroup class
///////////////////////////////

void KonfiguratorCheckBoxGroup::add(KonfiguratorCheckBox *checkBox)
{
    checkBoxList.append(checkBox);
}

KonfiguratorCheckBox * KonfiguratorCheckBoxGroup::find(int index)
{
    if (index < 0 || index >= checkBoxList.count())
        return 0;
    return checkBoxList.at(index);
}

KonfiguratorCheckBox * KonfiguratorCheckBoxGroup::find(QString name)
{
    QListIterator<KonfiguratorCheckBox *> it(checkBoxList);
    while (it.hasNext()) {
        KonfiguratorCheckBox * checkBox = it.next();

        if (checkBox->extension()->getCfgName() == name)
            return checkBox;
    }

    return 0;
}


// KonfiguratorRadioButtons class
///////////////////////////////

KonfiguratorRadioButtons::KonfiguratorRadioButtons(QString cls, QString name,
        QString dflt, QWidget *parent, bool rst, int pg) :
        QWidget(parent), defaultValue(dflt)
{
    ext = new KonfiguratorExtension(this, cls, name, rst, pg);
    connect(ext, SIGNAL(applyAuto(QObject *, QString, QString)), this, SLOT(slotApply(QObject *, QString, QString)));
    connect(ext, SIGNAL(setDefaultsAuto(QObject *)), this, SLOT(slotSetDefaults(QObject *)));
    connect(ext, SIGNAL(setInitialValue(QObject *)), this, SLOT(loadInitialValue()));
}

KonfiguratorRadioButtons::~KonfiguratorRadioButtons()
{
    delete ext;
}

void KonfiguratorRadioButtons::addRadioButton(QRadioButton *radioWidget, QString name, QString value)
{
    radioButtons.append(radioWidget);
    radioNames.push_back(name);
    radioValues.push_back(value);

    connect(radioWidget, SIGNAL(toggled(bool)), ext, SLOT(setChanged()));
}

QRadioButton * KonfiguratorRadioButtons::find(int index)
{
    if (index < 0 || index >= radioButtons.count())
        return 0;

    return radioButtons.at(index);
}

QRadioButton * KonfiguratorRadioButtons::find(QString name)
{
    int index = radioNames.indexOf(name);
    if (index == -1)
        return 0;

    return radioButtons.at(index);
}

void KonfiguratorRadioButtons::selectButton(QString value)
{
    int cnt = 0;

    QListIterator<QRadioButton *> it(radioButtons);
    while (it.hasNext()) {
        QRadioButton * btn = it.next();

        if (value == radioValues[ cnt ]) {
            btn->setChecked(true);
            return;
        }

        cnt++;
    }

    if (!radioButtons.isEmpty())
        radioButtons.first()->setChecked(true);
}

void KonfiguratorRadioButtons::loadInitialValue()
{
    KConfigGroup group(krConfig, ext->getCfgClass());
    QString initValue = group.readEntry(ext->getCfgName(), defaultValue);

    selectButton(initValue);
    ext->setChanged(false);
}

QString KonfiguratorRadioButtons::selectedValue()
{
    int cnt = 0;

    QListIterator<QRadioButton *> it(radioButtons);
    while (it.hasNext()) {
        QRadioButton * btn = it.next();

        if (btn->isChecked()) {
            return radioValues[ cnt ];
        }

        cnt++;
    }
    return QString();
}

void KonfiguratorRadioButtons::slotApply(QObject *, QString cls, QString name)
{
    QString value = selectedValue();

    if (!value.isEmpty())
        KConfigGroup(krConfig, cls).writeEntry(name, value);
}

void KonfiguratorRadioButtons::slotSetDefaults(QObject *)
{
    selectButton(defaultValue);
}

// KonfiguratorEditBox class
///////////////////////////////

KonfiguratorEditBox::KonfiguratorEditBox(QString cls, QString name, QString dflt,
        QWidget *parent, bool rst, int pg) : QLineEdit(parent),
        defaultValue(dflt)
{
    ext = new KonfiguratorExtension(this, cls, name, rst, pg);
    connect(ext, SIGNAL(applyAuto(QObject *, QString, QString)), this, SLOT(slotApply(QObject *, QString, QString)));
    connect(ext, SIGNAL(setDefaultsAuto(QObject *)), this, SLOT(slotSetDefaults(QObject *)));
    connect(ext, SIGNAL(setInitialValue(QObject *)), this, SLOT(loadInitialValue()));

    connect(this, SIGNAL(textChanged(const QString &)), ext, SLOT(setChanged()));

    loadInitialValue();
}

KonfiguratorEditBox::~KonfiguratorEditBox()
{
    delete ext;
}

void KonfiguratorEditBox::loadInitialValue()
{
    KConfigGroup group(krConfig, ext->getCfgClass());
    setText(group.readEntry(ext->getCfgName(), defaultValue));
    ext->setChanged(false);
}

void KonfiguratorEditBox::slotApply(QObject *, QString cls, QString name)
{
    KConfigGroup(krConfig, cls).writeEntry(name, text());
}

void KonfiguratorEditBox::slotSetDefaults(QObject *)
{
    if (text() != defaultValue)
        setText(defaultValue);
}


// KonfiguratorURLRequester class
///////////////////////////////

KonfiguratorURLRequester::KonfiguratorURLRequester(QString cls, QString name, QString dflt,
        QWidget *parent, bool rst, int pg, bool expansion) : KUrlRequester(parent),
        defaultValue(dflt),
        expansion(expansion)
{
    ext = new KonfiguratorExtension(this, cls, name, rst, pg);
    connect(ext, SIGNAL(applyAuto(QObject *, QString, QString)), this, SLOT(slotApply(QObject *, QString, QString)));
    connect(ext, SIGNAL(setDefaultsAuto(QObject *)), this, SLOT(slotSetDefaults(QObject *)));
    connect(ext, SIGNAL(setInitialValue(QObject *)), this, SLOT(loadInitialValue()));

    connect(this, SIGNAL(textChanged(const QString &)), ext, SLOT(setChanged()));

    button()->setIcon(KIcon(SmallIcon("document-open")));
    loadInitialValue();
}

KonfiguratorURLRequester::~KonfiguratorURLRequester()
{
    delete ext;
}

void KonfiguratorURLRequester::loadInitialValue()
{
    KConfigGroup group(krConfig, ext->getCfgClass());
    lineEdit()->setText(group.readEntry(ext->getCfgName(), defaultValue));
    ext->setChanged(false);
}

void KonfiguratorURLRequester::slotApply(QObject *, QString cls, QString name)
{
    KConfigGroup(krConfig, cls).writeEntry(name, expansion ? url().pathOrUrl() : text());
}

void KonfiguratorURLRequester::slotSetDefaults(QObject *)
{
    if (url() != defaultValue)
        lineEdit()->setText(defaultValue);
}

// KonfiguratorFontChooser class
///////////////////////////////

KonfiguratorFontChooser::KonfiguratorFontChooser(QString cls, QString name, QFont dflt,
        QWidget *parent, bool rst, int pg) : QWidget(parent),
        defaultValue(dflt)
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    ext = new KonfiguratorExtension(this, cls, name, rst, pg);
    connect(ext, SIGNAL(applyAuto(QObject *, QString, QString)), this, SLOT(slotApply(QObject *, QString, QString)));
    connect(ext, SIGNAL(setDefaultsAuto(QObject *)), this, SLOT(slotSetDefaults(QObject *)));
    connect(ext, SIGNAL(setInitialValue(QObject *)), this, SLOT(loadInitialValue()));

    pLabel = new QLabel(this);
    pLabel->setMinimumWidth(150);
    layout->addWidget(pLabel);

    pToolButton = new QToolButton(this);

    connect(pToolButton, SIGNAL(clicked()), this, SLOT(slotBrowseFont()));

    pToolButton->setIcon(SmallIcon("document-open"));
    layout->addWidget(pToolButton);

    loadInitialValue();
}

KonfiguratorFontChooser::~KonfiguratorFontChooser()
{
    delete ext;
}

void KonfiguratorFontChooser::loadInitialValue()
{
    KConfigGroup group(krConfig, ext->getCfgClass());
    font = group.readEntry(ext->getCfgName(), defaultValue);
    ext->setChanged(false);
    setFont();
}

void KonfiguratorFontChooser::setFont()
{
    pLabel->setFont(font);
    pLabel->setText(font.family() + QString(", %1").arg(font.pointSize()));
}

void KonfiguratorFontChooser::slotApply(QObject *, QString cls, QString name)
{
    KConfigGroup(krConfig, cls).writeEntry(name, font);
}

void KonfiguratorFontChooser::slotSetDefaults(QObject *)
{
    font = defaultValue;
    ext->setChanged();
    setFont();
}

void KonfiguratorFontChooser::slotBrowseFont()
{
    int ok = KFontDialog::getFont(font);
    if (ok != 1) return;  // cancelled by the user
    ext->setChanged();
    setFont();
}

// KonfiguratorComboBox class
///////////////////////////////

KonfiguratorComboBox::KonfiguratorComboBox(QString cls, QString name, QString dflt,
        KONFIGURATOR_NAME_VALUE_PAIR *listIn, int listInLen, QWidget *parent,
        bool rst, bool editable, int pg) : QComboBox(parent),
        defaultValue(dflt), listLen(listInLen)
{
    list = new KONFIGURATOR_NAME_VALUE_PAIR[ listInLen ];

    for (int i = 0; i != listLen; i++) {
        list[i] = listIn[i];
        addItem(list[i].text);
    }

    ext = new KonfiguratorExtension(this, cls, name, rst, pg);
    connect(ext, SIGNAL(applyAuto(QObject *, QString, QString)), this, SLOT(slotApply(QObject *, QString, QString)));
    connect(ext, SIGNAL(setDefaultsAuto(QObject *)), this, SLOT(slotSetDefaults(QObject *)));
    connect(ext, SIGNAL(setInitialValue(QObject *)), this, SLOT(loadInitialValue()));

//  connect( this, SIGNAL( highlighted(int) ), ext, SLOT( setChanged() ) ); /* Removed because of startup combo failure */
    connect(this, SIGNAL(activated(int)), ext, SLOT(setChanged()));
    connect(this, SIGNAL(textChanged(const QString &)), ext, SLOT(setChanged()));

    setEditable(editable);
    loadInitialValue();
}

KonfiguratorComboBox::~KonfiguratorComboBox()
{
    delete []list;
    delete ext;
}

void KonfiguratorComboBox::loadInitialValue()
{
    KConfigGroup group(krConfig, ext->getCfgClass());
    QString select = group.readEntry(ext->getCfgName(), defaultValue);
    selectEntry(select);
    ext->setChanged(false);
}

void KonfiguratorComboBox::slotApply(QObject *, QString cls, QString name)
{
    QString text = isEditable() ? lineEdit()->text() : currentText();
    QString value = text;

    for (int i = 0; i != listLen; i++)
        if (list[i].text == text) {
            value = list[i].value;
            break;
        }

    KConfigGroup(krConfig, cls).writeEntry(name, value);
}

void KonfiguratorComboBox::selectEntry(QString entry)
{
    for (int i = 0; i != listLen; i++)
        if (list[i].value == entry) {
            setCurrentIndex(i);
            return;
        }

    if (isEditable())
        lineEdit()->setText(entry);
    else
        setCurrentIndex(0);
}

void KonfiguratorComboBox::slotSetDefaults(QObject *)
{
    selectEntry(defaultValue);
}


// KonfiguratorColorChooser class
///////////////////////////////

KonfiguratorColorChooser::KonfiguratorColorChooser(QString cls, QString name, QColor dflt,
        QWidget *parent, bool rst, ADDITIONAL_COLOR *addColPtr,
        int addColNum, int pg) : QComboBox(parent),
        defaultValue(dflt), disableColorChooser(true)
{
    ext = new KonfiguratorExtension(this, cls, name, rst, pg);

    connect(ext, SIGNAL(applyAuto(QObject *, QString, QString)), this, SLOT(slotApply(QObject *, QString, QString)));
    connect(ext, SIGNAL(setDefaultsAuto(QObject *)), this, SLOT(slotSetDefaults(QObject *)));
    connect(ext, SIGNAL(setInitialValue(QObject *)), this, SLOT(loadInitialValue()));

    addColor(i18n("Custom color"),  QColor(255, 255, 255));
    addColor(i18nc("Default color", "Default"), defaultValue);

    for (int i = 0; i != addColNum; i++) {
        additionalColors.push_back(addColPtr[i]);
        addColor(addColPtr[i].name, addColPtr[i].color);
    }

    addColor(i18n("Red"),           Qt::red);
    addColor(i18n("Green"),         Qt::green);
    addColor(i18n("Blue"),          Qt::blue);
    addColor(i18n("Cyan"),          Qt::cyan);
    addColor(i18n("Magenta"),       Qt::magenta);
    addColor(i18n("Yellow"),        Qt::yellow);
    addColor(i18n("Dark Red"),      Qt::darkRed);
    addColor(i18n("Dark Green"),    Qt::darkGreen);
    addColor(i18n("Dark Blue"),     Qt::darkBlue);
    addColor(i18n("Dark Cyan"),     Qt::darkCyan);
    addColor(i18n("Dark Magenta"),  Qt::darkMagenta);
    addColor(i18n("Dark Yellow"),   Qt::darkYellow);
    addColor(i18n("White"),         Qt::white);
    addColor(i18n("Light Gray"),    Qt::lightGray);
    addColor(i18n("Gray"),          Qt::gray);
    addColor(i18n("Dark Gray"),     Qt::darkGray);
    addColor(i18n("Black"),         Qt::black);

    connect(this, SIGNAL(activated(int)),   this, SLOT(slotCurrentChanged(int)));

    loadInitialValue();
}

KonfiguratorColorChooser::~KonfiguratorColorChooser()
{
    delete ext;
}

QPixmap KonfiguratorColorChooser::createPixmap(QColor color)
{
    QPainter painter;
    QPen pen;
    int size = QFontMetrics(font()).height() * 3 / 4;
    QRect rect(0, 0, size, size);
    QPixmap pixmap(rect.width(), rect.height());

    pen.setColor(Qt::black);

    painter.begin(&pixmap);
    QBrush brush(color);
    painter.fillRect(rect, brush);
    painter.setPen(pen);
    painter.drawRect(rect);
    painter.end();

    pixmap.detach();
    return pixmap;
}

void KonfiguratorColorChooser::addColor(QString text, QColor color)
{
    addItem(createPixmap(color), text);
    palette.push_back(color);
}

void KonfiguratorColorChooser::loadInitialValue()
{
    KConfigGroup group(krConfig, ext->getCfgClass());
    QString selected = group.readEntry(ext->getCfgName(), QString(""));
    setValue(selected);
    ext->setChanged(false);
}

void KonfiguratorColorChooser::setDefaultColor(QColor dflt)
{
    defaultValue = dflt;
    palette[1] = defaultValue;
    setItemIcon(1, createPixmap(defaultValue));

    if (currentIndex() == 1)
        emit colorChanged();
}

void KonfiguratorColorChooser::changeAdditionalColor(int num, QColor color)
{
    if (num < additionalColors.size()) {
        palette[2+num] = color;
        additionalColors[num].color = color;
        setItemIcon(2 + num, createPixmap(color));

        if (currentIndex() == 2 + num)
            emit colorChanged();
    }
}

void KonfiguratorColorChooser::setDefaultText(QString text)
{
    setItemIcon(1, createPixmap(defaultValue));
    setItemText(1, text);
}

void KonfiguratorColorChooser::slotApply(QObject *, QString cls, QString name)
{
    KConfigGroup(krConfig, cls).writeEntry(name, getValue());
}

void KonfiguratorColorChooser::setValue(QString value)
{
    disableColorChooser = true;

    if (value.isEmpty()) {
        setCurrentIndex(1);
        customValue = defaultValue;
    } else {
        bool found = false;

        for (int j = 0; j != additionalColors.size(); j++)
            if (additionalColors[j].value == value) {
                setCurrentIndex(2 + j);
                found = true;
                break;
            }

        if (! found) {
            KConfigGroup colGroup(krConfig, ext->getCfgClass());
            colGroup.writeEntry("TmpColor", value);
            QColor color = colGroup.readEntry("TmpColor", defaultValue);
            customValue = color;
            colGroup.deleteEntry("TmpColor");

            setCurrentIndex(0);
            for (int i = 2 + additionalColors.size(); i != palette.size(); i++)
                if (palette[i] == color) {
                    setCurrentIndex(i);
                    break;
                }
        }
    }

    palette[0] = customValue;
    setItemIcon(0, createPixmap(customValue));

    ext->setChanged();
    emit colorChanged();
    disableColorChooser = false;
}

QString KonfiguratorColorChooser::getValue()
{
    QColor color = palette[ currentIndex()];
    if (currentIndex() == 1)     /* it's the default value? */
        return "";
    else if (currentIndex() >= 2 && currentIndex() < 2 + additionalColors.size())
        return additionalColors[ currentIndex() - 2 ].value;
    else
        return QString("%1,%2,%3").arg(color.red()).arg(color.green()).arg(color.blue());
}

bool KonfiguratorColorChooser::isValueRGB()
{
    return !(currentIndex() >= 1 && currentIndex() < 2 + additionalColors.size());
}

void KonfiguratorColorChooser::slotSetDefaults(QObject *)
{
    ext->setChanged();
    setCurrentIndex(1);
    emit colorChanged();
}

void KonfiguratorColorChooser::slotCurrentChanged(int number)
{
    ext->setChanged();
    if (number == 0 && !disableColorChooser) {
        QColor color = QColorDialog::getColor(customValue, this);
        if (color.isValid()) {
            disableColorChooser = true;
            customValue = color;
            palette[0] = customValue;
            setItemIcon(0, createPixmap(customValue));
            disableColorChooser = false;
        }
    }

    emit colorChanged();
}

QColor KonfiguratorColorChooser::getColor()
{
    return palette[ currentIndex()];
}

// KonfiguratorListBox class
///////////////////////////////

KonfiguratorListBox::KonfiguratorListBox(QString cls, QString name, QStringList dflt,
        QWidget *parent, bool rst, int pg) : KrListWidget(parent),
        defaultValue(dflt)
{
    ext = new KonfiguratorExtension(this, cls, name, rst, pg);
    connect(ext, SIGNAL(applyAuto(QObject *, QString, QString)), this, SLOT(slotApply(QObject *, QString, QString)));
    connect(ext, SIGNAL(setDefaultsAuto(QObject *)), this, SLOT(slotSetDefaults(QObject *)));
    connect(ext, SIGNAL(setInitialValue(QObject *)), this, SLOT(loadInitialValue()));

    loadInitialValue();
}

KonfiguratorListBox::~KonfiguratorListBox()
{
    delete ext;
}

void KonfiguratorListBox::loadInitialValue()
{
    KConfigGroup group(krConfig, ext->getCfgClass());
    setList(group.readEntry(ext->getCfgName(), defaultValue));
    ext->setChanged(false);
}

void KonfiguratorListBox::slotApply(QObject *, QString cls, QString name)
{
    KConfigGroup(krConfig, cls).writeEntry(name, list());
}

void KonfiguratorListBox::slotSetDefaults(QObject *)
{
    if (list() != defaultValue) {
        ext->setChanged();
        setList(defaultValue);
    }
}

void KonfiguratorListBox::setList(QStringList list)
{
    clear();
    addItems(list);
}

QStringList KonfiguratorListBox::list()
{
    QStringList lst;

    for (int i = 0; i != count(); i++)
        lst += item(i)->text();

    return lst;
}

void KonfiguratorListBox::addItem(const QString & item)
{
    if (!list().contains(item)) {
        KrListWidget::addItem(item);
        ext->setChanged();
    }
}

void KonfiguratorListBox::removeItem(const QString & item)
{
    QList<QListWidgetItem *> list = findItems(item, Qt::MatchExactly);
    for (int i = 0; i != list.count(); i++)
        delete list[ i ];

    if (list.count())
        ext->setChanged();
}

#include "konfiguratoritems.moc"
