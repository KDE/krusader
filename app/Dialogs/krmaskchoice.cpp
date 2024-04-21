/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "krmaskchoice.h"

// QtCore
#include <QVariant>
// QtWidgets
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include <KComboBox>
#include <KLocalizedString>

#include "../GUI/krlistwidget.h"
#include "../compat.h"

/**
 * Constructs a KrMaskChoice which is a child of 'parent', with the
 * name 'name' and widget flags set to 'f'
 *
 * The dialog will by default be modeless, unless you set 'modal' to
 * TRUE to construct a modal dialog.
 */
KrMaskChoice::KrMaskChoice(QWidget *parent)
    : QDialog(parent)
{
    setModal(true);
    resize(401, 314);
    setWindowTitle(i18n("Choose Files"));

    auto *MainLayout = new QVBoxLayout(this);

    auto *HeaderLayout = new QHBoxLayout();
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

    auto *GroupBox1 = new QGroupBox(this);
    GroupBox1->setTitle(i18n("Predefined Selections"));
    MainLayout->addWidget(GroupBox1);

    auto *gbLayout = new QHBoxLayout(GroupBox1);

    preSelections = new KrListWidget(GroupBox1);
    preSelections->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    preSelections->setWhatsThis(
        i18n("A predefined selection is a file-mask which you often use.\nSome examples are: \"*.c, *.h\", \"*.c, *.o\", etc.\nYou can add these masks to the "
             "list by typing them and pressing the Add button.\nDelete removes a predefined selection and Clear removes all of them.\nNotice that the line in "
             "which you edit the mask has its own history, you can scroll it, if needed."));
    gbLayout->addWidget(preSelections);

    auto *vbox = new QVBoxLayout();
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

    QDialogButtonBox *ButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    MainLayout->addWidget(ButtonBox);

    // signals and slots connections
    connect(ButtonBox, &QDialogButtonBox::rejected, this, &KrMaskChoice::reject);
    connect(ButtonBox, &QDialogButtonBox::accepted, this, &KrMaskChoice::accept);
    connect(PushButton7, &QPushButton::clicked, this, &KrMaskChoice::addSelection);
    connect(PushButton7_2, &QPushButton::clicked, this, &KrMaskChoice::deleteSelection);
    connect(PushButton7_3, &QPushButton::clicked, this, &KrMaskChoice::clearSelections);
    connect(selection, QOverload<const QString &>::of(&KComboBox::QCOMBOBOX_ACTIVATED), selection, &KComboBox::setEditText);
    connect(selection->lineEdit(), &QLineEdit::returnPressed, this, &KrMaskChoice::accept);
    connect(preSelections, &KrListWidget::currentItemChanged, this, &KrMaskChoice::currentItemChanged);
    connect(preSelections, &KrListWidget::itemActivated, this, &KrMaskChoice::acceptFromList);
}

/*
 *  Destroys the object and frees any allocated resources
 */
KrMaskChoice::~KrMaskChoice()
{
    // no need to delete child widgets, Qt does it all for us
}

void KrMaskChoice::addSelection()
{
    qWarning("KrMaskChoice::addSelection(): Not implemented yet!");
}

void KrMaskChoice::clearSelections()
{
    qWarning("KrMaskChoice::clearSelections(): Not implemented yet!");
}

void KrMaskChoice::deleteSelection()
{
    qWarning("KrMaskChoice::deleteSelection(): Not implemented yet!");
}

void KrMaskChoice::acceptFromList(QListWidgetItem *)
{
    qWarning("KrMaskChoice::acceptFromList(QListWidgetItem *): Not implemented yet!");
}

void KrMaskChoice::currentItemChanged(QListWidgetItem *)
{
    qWarning("KrMaskChoice::currentItemChanged(QListWidgetItem *): Not implemented yet!");
}
