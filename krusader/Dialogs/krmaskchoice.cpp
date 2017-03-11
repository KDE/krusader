/***************************************************************************
                                 krmaskchoice.cpp
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
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
#include "krmaskchoice.h"

// QtCore
#include <QVariant>
// QtWidgets
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialogButtonBox>

#include <KCompletion/KComboBox>
#include <KI18n/KLocalizedString>

#include "../GUI/krlistwidget.h"

/**
 * Constructs a KRMaskChoice which is a child of 'parent', with the
 * name 'name' and widget flags set to 'f'
 *
 * The dialog will by default be modeless, unless you set 'modal' to
 * TRUE to construct a modal dialog.
 */
KRMaskChoice::KRMaskChoice(QWidget* parent)
        : QDialog(parent)
{
    setModal(true);
    resize(401, 314);
    setWindowTitle(i18n("Choose Files"));

    QVBoxLayout* MainLayout = new QVBoxLayout(this);

    QHBoxLayout* HeaderLayout = new QHBoxLayout();
    MainLayout->addLayout(HeaderLayout);

    PixmapLabel1 = new QLabel(this);
    PixmapLabel1->setScaledContents(true);
    PixmapLabel1->setMaximumSize(QSize(31, 31));
    HeaderLayout->addWidget(PixmapLabel1);

    label = new QLabel(this);
    label->setText(i18n("Select the following files:"));
    HeaderLayout->addWidget(label);

    selection = new KComboBox(this);
    selection->setEditable(true);
    selection->setInsertPolicy(QComboBox::InsertAtTop);
    selection->setAutoCompletion(true);
    MainLayout->addWidget(selection);

    QGroupBox* GroupBox1 = new QGroupBox(this);
    GroupBox1->setTitle(i18n("Predefined Selections"));
    MainLayout->addWidget(GroupBox1);

    QHBoxLayout* gbLayout = new QHBoxLayout(GroupBox1);

    preSelections = new KrListWidget(GroupBox1);
    preSelections->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    preSelections->setWhatsThis(i18n("A predefined selection is a file-mask which you often use.\nSome examples are: \"*.c, *.h\", \"*.c, *.o\", etc.\nYou can add these masks to the list by typing them and pressing the Add button.\nDelete removes a predefined selection and Clear removes all of them.\nNotice that the line in which you edit the mask has its own history, you can scroll it, if needed."));
    gbLayout->addWidget(preSelections);

    QVBoxLayout* vbox = new QVBoxLayout();
    gbLayout->addLayout(vbox);

    PushButton7 = new QPushButton(GroupBox1);
    PushButton7->setText(i18n("Add"));
    PushButton7->setToolTip(i18n("Adds the selection in the line-edit to the list"));
    vbox->addWidget(PushButton7);

    PushButton7_2 = new QPushButton(GroupBox1);
    PushButton7_2->setText(i18n("Delete"));
    PushButton7_2->setToolTip(i18n("Delete the marked selection from the list"));
    vbox->addWidget(PushButton7_2);

    PushButton7_3 = new QPushButton(GroupBox1);
    PushButton7_3->setText(i18n("Clear"));
    PushButton7_3->setToolTip(i18n("Clears the entire list of selections"));
    vbox->addWidget(PushButton7_3);
    vbox->addItem(new QSpacerItem(5, 5, QSizePolicy::Fixed, QSizePolicy::Expanding));

    QDialogButtonBox* ButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    MainLayout->addWidget(ButtonBox);

    // signals and slots connections
    connect(ButtonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(ButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(PushButton7, SIGNAL(clicked()), this, SLOT(addSelection()));
    connect(PushButton7_2, SIGNAL(clicked()), this, SLOT(deleteSelection()));
    connect(PushButton7_3, SIGNAL(clicked()), this, SLOT(clearSelections()));
    connect(selection, SIGNAL(activated(QString)), selection, SLOT(setEditText(QString)));
    connect(selection->lineEdit(), SIGNAL(returnPressed()), this, SLOT(accept()));
    connect(preSelections, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(currentItemChanged(QListWidgetItem*)));
    connect(preSelections, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(acceptFromList(QListWidgetItem*)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
KRMaskChoice::~KRMaskChoice()
{
    // no need to delete child widgets, Qt does it all for us
}

void KRMaskChoice::addSelection()
{
    qWarning("KRMaskChoice::addSelection(): Not implemented yet!");
}

void KRMaskChoice::clearSelections()
{
    qWarning("KRMaskChoice::clearSelections(): Not implemented yet!");
}

void KRMaskChoice::deleteSelection()
{
    qWarning("KRMaskChoice::deleteSelection(): Not implemented yet!");
}

void KRMaskChoice::acceptFromList(QListWidgetItem *)
{
    qWarning("KRMaskChoice::acceptFromList(QListWidgetItem *): Not implemented yet!");
}

void KRMaskChoice::currentItemChanged(QListWidgetItem *)
{
    qWarning("KRMaskChoice::currentItemChanged(QListWidgetItem *): Not implemented yet!");
}

