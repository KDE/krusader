/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 * Copyright (C) 2009 Fathi Boudra <fboudra@gmail.com>                       *
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

#include "newftpgui.h"

#include <QtCore/QStringList>
#include <QtGui/QFont>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QSizePolicy>
#include <QtGui/QWidget>
#include <QEvent>

#include <KConfigGroup>
#include <KIconLoader>
#include <KLocale>
#include <KProtocolInfo>

#include "../krglobal.h"

#define SIZE_MINIMUM QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed)

/**
 * Constructs a newFTPGUI which is a child of 'parent',
 * with the name 'name' and widget flags set to 'f'
 */
newFTPGUI::newFTPGUI(QWidget* parent) : KDialog(parent)
{
    QWidget *widget = new QWidget(this);

    resize(320, 240);
    setButtonText(KDialog::Ok, i18n("&Connect"));
    setMainWidget(widget);
    setModal(true);
    setWindowTitle(i18n("New Network Connection"));

    QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    policy.setHeightForWidth(sizePolicy().hasHeightForWidth());
    setSizePolicy(policy);

    iconLabel = new QLabel(widget);
    iconLabel->setPixmap(krLoader->loadIcon("network-wired", KIconLoader::Desktop, 32));
    iconLabel->setSizePolicy(SIZE_MINIMUM);

    aboutLabel = new QLabel(i18n("About to connect to..."), widget);
    QFont font(aboutLabel->font());
    font.setBold(true);
    aboutLabel->setFont(font);

    protocolLabel = new QLabel(i18n("Protocol:"), widget);
    hostLabel = new QLabel(i18n("Host:"), widget);
    portLabel = new QLabel(i18n("Port:"), widget);

    prefix = new KComboBox(widget);
    prefix->setObjectName(QString::fromUtf8("protocol"));
    prefix->setSizePolicy(SIZE_MINIMUM);

    url = new KHistoryComboBox(widget);
    url->setMaxCount(25);
    url->setMinimumContentsLength(10);

    QStringList protocols = KProtocolInfo::protocols();
    if (protocols.contains("ftp"))
        prefix->addItem(i18n("ftp://"));
    if (protocols.contains("smb"))
        prefix->addItem(i18n("smb://"));
    if (protocols.contains("fish"))
        prefix->addItem(i18n("fish://"));
    if (protocols.contains("sftp"))
        prefix->addItem(i18n("sftp://"));

    // load the history and completion list after creating the history combo
    KConfigGroup group(krConfig, "Private");
    QStringList list = group.readEntry("newFTP Completion list", QStringList());
    url->completionObject()->setItems(list);
    list = group.readEntry("newFTP History list", QStringList());
    url->setHistoryItems(list);

    port = new QSpinBox(widget);
    port->setMaximum(65535);
    port->setValue(21);
    port->setSizePolicy(SIZE_MINIMUM);

    usernameLabel = new QLabel(i18n("Username:"), widget);
    username = new KLineEdit(widget);
    passwordLabel = new QLabel(i18n("Password:"), widget);
    password = new KLineEdit(widget);
    password->setEchoMode(QLineEdit::Password);

    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    horizontalLayout->addWidget(iconLabel);
    horizontalLayout->addWidget(aboutLabel);

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(protocolLabel, 0, 0, 1, 1);
    gridLayout->addWidget(hostLabel, 0, 1, 1, 1);
    gridLayout->addWidget(portLabel, 0, 2, 1, 1);
    gridLayout->addWidget(prefix, 1, 0, 1, 1);
    gridLayout->addWidget(url, 1, 1, 1, 1);
    gridLayout->addWidget(port, 1, 2, 1, 1);
    gridLayout->addWidget(usernameLabel, 2, 0, 1, 1);
    gridLayout->addWidget(username, 3, 0, 1, 3);
    gridLayout->addWidget(passwordLabel, 4, 0, 1, 1);
    gridLayout->addWidget(password, 5, 0, 1, 3);

    QGridLayout *widgetLayout = new QGridLayout(widget);
    widgetLayout->addLayout(horizontalLayout, 0, 0, 1, 1);
    widgetLayout->addLayout(gridLayout, 1, 0, 1, 1);

    connect(prefix, SIGNAL(activated(const QString &)),
            this, SLOT(slotTextChanged(const QString &)));
    connect(url, SIGNAL(activated(const QString &)),
            url, SLOT(addToHistory(const QString &)));

    setTabOrder(url, username);
    setTabOrder(username, password);
    setTabOrder(password, prefix);
    setTabOrder(prefix, url);
}

/**
 * Destroys the object and frees any allocated resources
 */
newFTPGUI::~newFTPGUI()
{
    // no need to delete child widgets, Qt does it all for us
}

void newFTPGUI::slotTextChanged(const QString &string)
{
    if (string.startsWith(QLatin1String("ftp")) ||
            string.startsWith(QLatin1String("sftp")) ||
            string.startsWith(QLatin1String("fish"))) {
        if (port->value() == 21 || port->value() == 22) {
            port->setValue(string.startsWith(QLatin1String("ftp")) ? 21 : 22);
        }
        port->setEnabled(true);
    } else {
        port->setEnabled(false);
    }
}

/**
 * Main event handler. Reimplemented to handle application font changes
 */
bool newFTPGUI::event(QEvent *ev)
{
    bool ret = KDialog::event(ev);
    if (ev->type() == QEvent::ApplicationFontChange) {
        QFont font(aboutLabel->font());
        font.setBold(true);
        aboutLabel->setFont(font);
    }
    return ret;
}

#include "newftpgui.moc"
