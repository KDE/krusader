/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2009 Fathi Boudra <fboudra@gmail.com>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "newftpgui.h"

// QtCore
#include <QEvent>
#include <QStringList>
// QtGui
#include <QFont>
// QtWidgets
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSizePolicy>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KProtocolInfo>
#include <KSharedConfig>

#include "../compat.h"
#include "../icon.h"
#include "../krglobal.h"

#define SIZE_MINIMUM QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed)

const QStringList sProtocols = QStringList() << QStringLiteral("ftp") << QStringLiteral("ftps") << QStringLiteral("sftp") << QStringLiteral("fish")
                                             << QStringLiteral("nfs") << QStringLiteral("smb") << QStringLiteral("webdav") << QStringLiteral("svn")
                                             << QStringLiteral("svn+file") << QStringLiteral("svn+http") << QStringLiteral("svn+https")
                                             << QStringLiteral("svn+ssh");

/**
 * Constructs a newFTPGUI which is a child of 'parent',
 * with the name 'name' and widget flags set to 'f'
 */
newFTPGUI::newFTPGUI(QWidget *parent)
    : QDialog(parent)
{
    setModal(true);
    setWindowTitle(i18n("New Network Connection"));
    resize(500, 240);

    auto *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    policy.setHeightForWidth(sizePolicy().hasHeightForWidth());
    setSizePolicy(policy);

    iconLabel = new QLabel(this);
    iconLabel->setPixmap(Icon("network-wired").pixmap(32));
    iconLabel->setSizePolicy(SIZE_MINIMUM);

    aboutLabel = new QLabel(i18n("About to connect to..."), this);
    QFont font(aboutLabel->font());
    font.setBold(true);
    aboutLabel->setFont(font);

    protocolLabel = new QLabel(i18n("Protocol:"), this);
    hostLabel = new QLabel(i18n("Host:"), this);
    portLabel = new QLabel(i18n("Port:"), this);

    prefix = new KComboBox(this);
    prefix->setObjectName(QString::fromUtf8("protocol"));
    prefix->setSizePolicy(SIZE_MINIMUM);

    url = new KrHistoryComboBox(this);
    url->setMaxCount(50);
    url->setMinimumContentsLength(10);

    const QStringList availableProtocols = KProtocolInfo::protocols();
    for (const QString &protocol : sProtocols) {
        if (availableProtocols.contains(protocol))
            prefix->addItem(protocol + QStringLiteral("://"));
    }

    // load the history and completion list after creating the history combo
    KConfigGroup group(krConfig, "Private");
    QStringList list = group.readEntry("newFTP Completion list", QStringList());
    url->completionObject()->setItems(list);
    list = group.readEntry("newFTP History list", QStringList());
    url->setHistoryItems(list);

    // Select last used protocol
    const QString lastUsedProtocol = group.readEntry("newFTP Protocol", QString());
    if (!lastUsedProtocol.isEmpty()) {
        prefix->setCurrentItem(lastUsedProtocol);
    }

    port = new QSpinBox(this);
    port->setMaximum(65535);
    port->setValue(21);
    port->setSizePolicy(SIZE_MINIMUM);

    usernameLabel = new QLabel(i18n("Username:"), this);
    username = new KLineEdit(this);
    passwordLabel = new QLabel(i18n("Password:"), this);
    password = new KLineEdit(this);
    password->setEchoMode(QLineEdit::Password);

    auto *horizontalLayout = new QHBoxLayout();
    horizontalLayout->addWidget(iconLabel);
    horizontalLayout->addWidget(aboutLabel);

    auto *gridLayout = new QGridLayout();
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

    auto *widgetLayout = new QGridLayout();
    widgetLayout->addLayout(horizontalLayout, 0, 0, 1, 1);
    widgetLayout->addLayout(gridLayout, 1, 0, 1, 1);
    mainLayout->addLayout(widgetLayout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setText(i18n("&Connect"));
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &newFTPGUI::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &newFTPGUI::reject);

    connect(prefix, QOverload<const QString &>::of(&KComboBox::QCOMBOBOX_ACTIVATED), this, &newFTPGUI::slotTextChanged);
    connect(url, QOverload<const QString &>::of(&KrHistoryComboBox::QCOMBOBOX_ACTIVATED), url, &KrHistoryComboBox::addToHistory);

    if (!lastUsedProtocol.isEmpty()) {
        // update the port field
        slotTextChanged(lastUsedProtocol);
    }

    setTabOrder(url, username);
    setTabOrder(username, password);
    setTabOrder(password, prefix);
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
    if (string.startsWith(QLatin1String("ftp")) || string.startsWith(QLatin1String("sftp")) || string.startsWith(QLatin1String("fish"))) {
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
    bool ret = QDialog::event(ev);
    if (ev->type() == QEvent::ApplicationFontChange) {
        QFont font(aboutLabel->font());
        font.setBold(true);
        aboutLabel->setFont(font);
    }
    return ret;
}
