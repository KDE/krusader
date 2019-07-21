/*****************************************************************************
 * Copyright (C) 2000 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2000 Rafi Yanai <krusader@users.sourceforge.net>            *
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

    auto* MainLayout = new QVBoxLayout(this);

    auto* HeaderLayout = new QHBoxLayout();
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

    auto* GroupBox1 = new QGroupBox(this);
    GroupBox1->setTitle(i18n("Predefined Selections"));
    MainLayout->addWidget(GroupBox1);

    auto* gbLayout = new QHBoxLayout(GroupBox1);

    preSelections = new KrListWidget(GroupBox1);
    preSelections->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    preSelections->setWhatsThis(i18n("A predefined selection is a file-mask which you often use.\nSome examples are: \"*.c, *.h\", \"*.c, *.o\", etc.\nYou can add these masks to the list by typing them and pressing the Add button.\nDelete removes a predefined selection and Clear removes all of them.\nNotice that the line in which you edit the mask has its own history, you can scroll it, if needed."));
    gbLayout->addWidget(preSelections);

    auto* vbox = new QVBoxLayout();
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
    connect(ButtonBox, &QDialogButtonBox::rejected, this, &KRMaskChoice::reject);
    connect(ButtonBox, &QDialogButtonBox::accepted, this, &KRMaskChoice::accept);
    connect(PushButton7, &QPushButton::clicked, this, &KRMaskChoice::addSelection);
    connect(PushButton7_2, &QPushButton::clicked, this, &KRMaskChoice::deleteSelection);
    connect(PushButton7_3, &QPushButton::clicked, this, &KRMaskChoice::clearSelections);
    connect(selection, QOverload<const QString &>::of(&KComboBox::activated), selection, &KComboBox::setEditText);
    connect(selection->lineEdit(), &QLineEdit::returnPressed, this, &KRMaskChoice::accept);
    connect(preSelections, &KrListWidget::currentItemChanged, this, &KRMaskChoice::currentItemChanged);
    connect(preSelections, &KrListWidget::itemActivated, this, &KRMaskChoice::acceptFromList);
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

