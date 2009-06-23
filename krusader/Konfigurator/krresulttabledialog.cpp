/*****************************************************************************
 * Copyright (C) 2005 Dirk Eschler <deschler@users.sourceforge.net>          *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#include "krresulttabledialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>

KrResultTableDialog::KrResultTableDialog(QWidget *parent, DialogType type,
        const QString& caption, const QString& heading, const QString& headerIcon,
        const QString& hint)
        : KDialog(parent, 0)

{
    setButtons(KDialog::Help | KDialog::Ok);
    setDefaultButton(KDialog::Ok);
    setWindowTitle(caption);
    setWindowModality(Qt::WindowModal);

    _page = new QWidget(this);
    setMainWidget(_page);
    _topLayout = new QVBoxLayout(_page);
    _topLayout->setContentsMargins(0, 0, 0, 0);
    _topLayout->setSpacing(spacingHint());
    _topLayout->setAlignment(Qt::AlignTop);

    // +++ Heading +++
    // prepare the icon
    QWidget *_iconWidget = new QWidget(_page);
    QHBoxLayout * _iconBox = new QHBoxLayout(_iconWidget);
    _iconLabel = new QLabel(_iconWidget);
    _iconLabel->setPixmap(krLoader->loadIcon(headerIcon, KIconLoader::Desktop, 32));
    _iconLabel->setMinimumWidth(fontMetrics().maxWidth()*20);
    _iconLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    _iconLabel->setFixedSize(_iconLabel->sizeHint());
    _iconBox->addWidget(_iconLabel);
    _headingLabel = new QLabel(heading, _iconWidget);
    QFont defFont = KGlobalSettings::generalFont();
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
        _resultTable = new KrArchiverResultTable(_page);
        setHelp("konfig-archives"); // launch handbook at sect1-id via help button
        break;
    case Tool:
        _resultTable = new KrToolResultTable(_page);
        setHelp(""); // TODO find a good anchor
        break;
    default:
        break;
    }
    _topLayout->addWidget(_resultTable);

    // +++ Separator +++
    KSeparator* hSep = new KSeparator(Qt::Horizontal, _page);
    hSep->setContentsMargins(5, 5, 5, 5);
    _topLayout->addWidget(hSep);

    // +++ Hint +++
    if (!hint.isEmpty()) {
        _hintLabel = new QLabel(hint, _page);
        _hintLabel->setIndent(5);
        _hintLabel->setAlignment(Qt::AlignRight);
        _topLayout->addWidget(_hintLabel);
    }

    this->setFixedSize(this->sizeHint());   // make non-resizeable
}

KrResultTableDialog::~KrResultTableDialog()
{
}
