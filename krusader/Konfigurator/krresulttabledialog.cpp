/*****************************************************************************
 * Copyright (C) 2005 Dirk Eschler <deschler@users.sourceforge.net>          *
 * Copyright (C) 2005-2018 Krusader Krew [https://krusader.org]              *
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

#include "krresulttabledialog.h"

// QtGui
#include <QFontDatabase>
// QtWidgets
#include <QDialogButtonBox>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <KConfigWidgets/KHelpClient>

#include "../krglobal.h"
#include "../icon.h"

KrResultTableDialog::KrResultTableDialog(QWidget *parent, DialogType type,
        const QString& caption, const QString& heading, const QString& headerIcon,
        const QString& hint)
        : QDialog(parent, 0)

{
    setWindowTitle(caption);
    setWindowModality(Qt::WindowModal);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QVBoxLayout *_topLayout = new QVBoxLayout();
    _topLayout->setAlignment(Qt::AlignTop);

    // +++ Heading +++
    // prepare the icon
    QWidget *_iconWidget = new QWidget(this);
    QHBoxLayout * _iconBox = new QHBoxLayout(_iconWidget);
    QLabel *_iconLabel = new QLabel(_iconWidget);
    _iconLabel->setPixmap(Icon(headerIcon).pixmap(32));
    _iconLabel->setMinimumWidth(fontMetrics().maxWidth()*20);
    _iconLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _iconLabel->setFixedSize(_iconLabel->sizeHint());
    _iconBox->addWidget(_iconLabel);
    QLabel *_headingLabel = new QLabel(heading, _iconWidget);
    QFont defFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    defFont.setBold(true);
    _headingLabel->setFont(defFont);
    _headingLabel->setIndent(10);
    _iconBox->addWidget(_headingLabel);
    _topLayout->addWidget(_iconWidget);

    // +++ Add some space between heading and table +++
    QSpacerItem* hSpacer1 = new QSpacerItem(0, 5);
    _topLayout->addItem(hSpacer1);

    // +++ Table +++
    switch (type) {
    case Archiver:
        _resultTable = new KrArchiverResultTable(this);
        helpAnchor = QStringLiteral("konfig-archives"); // launch handbook at sect1-id via help button
        break;
    case Tool:
        _resultTable = new KrToolResultTable(this);
        helpAnchor = QStringLiteral("konfig-dependencies"); // TODO find a good anchor
        break;
    default:
        break;
    }
    _topLayout->addWidget(_resultTable);

    // +++ Separator +++
    KSeparator* hSep = new KSeparator(Qt::Horizontal, this);
    hSep->setContentsMargins(5, 5, 5, 5);
    _topLayout->addWidget(hSep);

    // +++ Hint +++
    if (!hint.isEmpty()) {
        QLabel *_hintLabel = new QLabel(hint, this);
        _hintLabel->setIndent(5);
        _hintLabel->setAlignment(Qt::AlignRight);
        _topLayout->addWidget(_hintLabel);
    }
    mainLayout->addLayout(_topLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Help);
    mainLayout->addWidget(buttonBox);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &KrResultTableDialog::accept);
    connect(buttonBox, &QDialogButtonBox::helpRequested, this, &KrResultTableDialog::showHelp);
    buttonBox->button(QDialogButtonBox::Ok)->setDefault(true);

    this->setFixedSize(this->sizeHint());   // make non-resizeable
}

KrResultTableDialog::~KrResultTableDialog()
{
}

void KrResultTableDialog::showHelp()
{
    if(!helpAnchor.isEmpty()) {
        KHelpClient::invokeHelp(helpAnchor);
    }
}
