/*
    SPDX-FileCopyrightText: 2006 Jonas Bähr <jonas.baehr@web.de>
    SPDX-FileCopyrightText: 2006-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "actionman.h"

// QtWidgets
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KMessageBox>

#include "../UserAction/useraction.h"
#include "../krusader.h"
#include "useractionpage.h"

ActionMan::ActionMan(QWidget *parent)
    : QDialog(parent)
{
    setWindowModality(Qt::WindowModal);
    setWindowTitle(i18n("ActionMan - Manage Your Useractions"));

    auto *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    userActionPage = new UserActionPage(this);
    mainLayout->addWidget(userActionPage);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close | QDialogButtonBox::Apply);
    mainLayout->addWidget(buttonBox);

    applyButton = buttonBox->button(QDialogButtonBox::Apply);
    applyButton->setEnabled(false);

    connect(buttonBox, &QDialogButtonBox::rejected, this, &ActionMan::slotClose);
    connect(applyButton, &QPushButton::clicked, this, &ActionMan::slotApply);
    connect(userActionPage, &UserActionPage::changed, this, &ActionMan::slotEnableApplyButton);
    connect(userActionPage, &UserActionPage::applied, this, &ActionMan::slotDisableApplyButton);

    exec();

    krApp->updateUserActions();
}

ActionMan::~ActionMan() = default;

void ActionMan::slotClose()
{
    if (userActionPage->readyToQuit())
        reject();
}

void ActionMan::slotApply()
{
    userActionPage->applyChanges();
}

void ActionMan::slotEnableApplyButton()
{
    applyButton->setEnabled(true);
}

void ActionMan::slotDisableApplyButton()
{
    applyButton->setEnabled(false);
}
