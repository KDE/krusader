/***************************************************************************
                        splittergui.cpp  -  description
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

#include "splittergui.h"

#include "../VFS/vfs.h"

#include <klocale.h>
#include <kmessagebox.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kdebug.h>

#include <QDoubleSpinBox>
#include <QValidator>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QLayout>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QKeyEvent>


struct SplitterGUI::PredefinedDevice
{
    PredefinedDevice(QString name, KIO::filesize_t capacity) :
        name(name), capacity(capacity) {}
    PredefinedDevice(const PredefinedDevice &other) :
        name(other.name), capacity(other.capacity) {}
    PredefinedDevice &operator=(const PredefinedDevice &other);

    QString name;
    KIO::filesize_t capacity;
};


const QList<SplitterGUI::PredefinedDevice> &SplitterGUI::predefinedDevices()
{
    static QList<PredefinedDevice> list;
    if(!list.count()) {
        list << PredefinedDevice(i18n("1.44 MB (3.5\")"), 1457664);
        list << PredefinedDevice(i18n("1.2 MB (5.25\")"), 1213952);
        list << PredefinedDevice(i18n("720 kB (3.5\")"), 730112);
        list << PredefinedDevice(i18n("360 kB (5.25\")"), 362496);
        list << PredefinedDevice(i18n("100 MB (ZIP)"), 100431872);
        list << PredefinedDevice(i18n("250 MB (ZIP)"), 250331136);
        list << PredefinedDevice(i18n("650 MB (CD-R)"), 650*0x100000);
        list << PredefinedDevice(i18n("700 MB (CD-R)"), 700*0x100000);
    }
    return list;
};

SplitterGUI::SplitterGUI(QWidget* parent,  KUrl fileURL, KUrl defaultDir) :
        QDialog(parent),
        userDefinedSize(0x100000), lastSelectedDevice(-1), resultCode(QDialog::Rejected),
        division(1)
{
    setModal(true);

    QGridLayout *grid = new QGridLayout(this);
    grid->setSpacing(6);
    grid->setContentsMargins(11, 11, 11, 11);

    QLabel *splitterLabel = new QLabel(this);
    splitterLabel->setText(i18n("Split the file %1 to directory:", fileURL.pathOrUrl()));
    splitterLabel->setMinimumWidth(400);
    grid->addWidget(splitterLabel, 0 , 0);

    urlReq = new KUrlRequester(this);
    urlReq->setUrl(defaultDir.pathOrUrl());
    urlReq->setMode(KFile::Directory);
    grid->addWidget(urlReq, 1 , 0);

    QWidget *splitSizeLine = new QWidget(this);
    QHBoxLayout * splitSizeLineLayout = new QHBoxLayout;
    splitSizeLine->setLayout(splitSizeLineLayout);

    deviceCombo = new QComboBox(splitSizeLine);
    for (int i = 0; i != predefinedDevices().count(); i++)
        deviceCombo->addItem(predefinedDevices()[i].name);
    deviceCombo->addItem(i18n("User Defined"));
    splitSizeLineLayout->addWidget(deviceCombo);

    QLabel *spacer = new QLabel(splitSizeLine);
    spacer->setText(" ");
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    splitSizeLineLayout->addWidget(spacer);

    QLabel *bytesPerFile = new QLabel(splitSizeLine);
    bytesPerFile->setText(i18n("Max file size:"));
    splitSizeLineLayout->addWidget(bytesPerFile);

    spinBox = new QDoubleSpinBox(splitSizeLine);
    spinBox->setMaximum(9999999999.0);
    spinBox->setMinimumWidth(85);
    spinBox->setEnabled(false);
    splitSizeLineLayout->addWidget(spinBox);

    sizeCombo = new QComboBox(splitSizeLine);
    sizeCombo->addItem(i18n("Byte"));
    sizeCombo->addItem(i18n("kByte"));
    sizeCombo->addItem(i18n("MByte"));
    sizeCombo->addItem(i18n("GByte"));
    splitSizeLineLayout->addWidget(sizeCombo);

    grid->addWidget(splitSizeLine, 2 , 0);

    overwriteCb = new QCheckBox(i18n("Overwrite files without confirmation"), this);
    grid->addWidget(overwriteCb, 3, 0);

    QFrame *separator = new QFrame(this);
    separator->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    separator->setFixedHeight(separator->sizeHint().height());

    grid->addWidget(separator, 4 , 0);

    QHBoxLayout *splitButtons = new QHBoxLayout;
    splitButtons->setSpacing(6);
    splitButtons->setContentsMargins(0, 0, 0, 0);

    QSpacerItem* spacer2 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    splitButtons->addItem(spacer2);

    QPushButton *splitBtn = new QPushButton(this);
    splitBtn->setText(i18n("&Split"));
    splitBtn->setIcon(KIcon("dialog-ok"));
    splitButtons->addWidget(splitBtn);

    QPushButton *cancelBtn = new QPushButton(this);
    cancelBtn->setText(i18n("&Cancel"));
    cancelBtn->setIcon(KIcon("dialog-cancel"));
    splitButtons->addWidget(cancelBtn);

    grid->addLayout(splitButtons, 5 , 0);

    setWindowTitle(i18n("Krusader::Splitter"));


    KConfigGroup cfg(KGlobal::config(), "Splitter");
    overwriteCb->setChecked(cfg.readEntry("OverWriteFiles", false));

    connect(sizeCombo, SIGNAL(activated(int)), this, SLOT(sizeComboActivated(int)));
    connect(deviceCombo, SIGNAL(activated(int)), this, SLOT(predefinedComboActivated(int)));
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(reject()));
    connect(splitBtn , SIGNAL(clicked()), this, SLOT(splitPressed()));

    predefinedComboActivated(0);
    resultCode = exec();
}

SplitterGUI::~SplitterGUI()
{
    KConfigGroup cfg(KGlobal::config(), "Splitter");
    cfg.writeEntry("OverWriteFiles", overwriteCb->isChecked());
}

KIO::filesize_t SplitterGUI::getSplitSize()
{
    if(deviceCombo->currentIndex() < predefinedDevices().count()) // predefined size selected
        return predefinedDevices()[deviceCombo->currentIndex()].capacity;
    // user defined size selected
    return spinBox->value() * division;
}

bool SplitterGUI::overWriteFiles()
{
    return overwriteCb->isChecked();
}

void SplitterGUI::sizeComboActivated(int item)
{
    KIO::filesize_t prevDivision = division;
    switch (item) {
    case 0:
        division = 1;            /* byte */
        break;
    case 1:
        division = 0x400;        /* kbyte */
        break;
    case 2:
        division = 0x100000;     /* Mbyte */
        break;
    case 3:
        division = 0x40000000;   /* Gbyte */
        break;
    }
    double value;
    if(deviceCombo->currentIndex() < predefinedDevices().count()) // predefined size selected
        value = (double)predefinedDevices()[deviceCombo->currentIndex()].capacity / division;
    else { // use defined size selected
        value = (double)(spinBox->value() * prevDivision) / division;
        if(value < 1)
            value = 1;
    }
    spinBox->setValue(value);
}

void SplitterGUI::predefinedComboActivated(int item)
{
    if(item == lastSelectedDevice)
        return;

    KIO::filesize_t capacity = userDefinedSize;

    if (lastSelectedDevice == predefinedDevices().count()) // user defined was selected previously
        userDefinedSize = spinBox->value() * division; // remember user defined size

    if(item < predefinedDevices().count()) { // predefined size selected
        capacity = predefinedDevices()[item].capacity;
        spinBox->setEnabled(false);
    } else // user defined size selected
        spinBox->setEnabled(true);

    kDebug() << capacity;

    if (capacity >= 0x40000000) {          /* Gbyte */
        sizeCombo->setCurrentIndex(3);
        division = 0x40000000;
    } else if (capacity >= 0x100000) {      /* Mbyte */
        sizeCombo->setCurrentIndex(2);
        division = 0x100000;
    } else if (capacity >= 0x400) {         /* kbyte */
        sizeCombo->setCurrentIndex(1);
        division = 0x400;
    } else {
        sizeCombo->setCurrentIndex(0);       /* byte */
        division = 1;
    }

    spinBox->setValue((double)capacity / division);

    lastSelectedDevice = item;
}

void SplitterGUI::splitPressed()
{
    if (!urlReq->url().isValid()) {
        KMessageBox::error(this, i18n("The directory path URL is malformed!"));
        return;
    }
    if(getSplitSize() > 0)
        emit accept();
}

void SplitterGUI::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Enter :
    case Qt::Key_Return :
        emit splitPressed();
        return;
    default:
        QDialog::keyPressEvent(e);
    }
}

#include "splittergui.moc"
