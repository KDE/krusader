/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krspwidgets.h"
#include "../Filter/filtertabs.h"
#include "../GUI/krlistwidget.h"
#include "../icon.h"
#include "../krglobal.h"

// QtCore
#include <QEvent>
// QtGui
#include <QBitmap>
// QtWidgets
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <qnamespace.h> // missing ?

#include <KComboBox>
#include <KCursor>
#include <KLocalizedString>
#include <KSharedConfig>

///////////////////// initiation of the static members ////////////////////////
QStringList KrSpWidgets::maskList;

///////////////////////////////////////////////////////////////////////////////

KrSpWidgets::KrSpWidgets() = default;

KrQuery KrSpWidgets::getMask(const QString &caption, bool nameOnly, QWidget *parent)
{
    if (!nameOnly) {
        return FilterTabs::getQuery(parent);
    } else {
        QPointer<KrMaskChoiceSub> p = new KrMaskChoiceSub(parent);
        p->setWindowTitle(caption);
        p->exec();
        QString selection = p->selection->currentText();
        delete p;
        if (selection.isEmpty()) {
            return KrQuery();
        } else {
            return KrQuery(selection);
        }
    }
}

/////////////////////////// newFTP ////////////////////////////////////////
QUrl KrSpWidgets::newFTP()
{
    QPointer<newFTPSub> p = new newFTPSub();
    p->exec();
    QString uri = p->url->currentText();
    if (uri.isEmpty()) {
        delete p;
        return QUrl(); // empty url
    }

    QString protocol = p->prefix->currentText();
    protocol.truncate(protocol.length() - 3); // remove the trailing ://

    QString username = p->username->text().simplified();
    QString password = p->password->text().simplified();

    qsizetype uriStart = uri.lastIndexOf('@'); /* lets the user enter user and password in the URI field */
    if (uriStart != -1) {
        QString uriUser = uri.left(uriStart);
        QString uriPsw;
        uri = uri.mid(uriStart + 1);

        qsizetype pswStart = uriUser.indexOf(':'); /* getting the password name from the URL */
        if (pswStart != -1) {
            uriPsw = uriUser.mid(pswStart + 1);
            uriUser = uriUser.left(pswStart);
        }

        if (!uriUser.isEmpty()) { /* handling the ftp proxy username and password also */
            username = username.isEmpty() ? uriUser : username + '@' + uriUser;
        }

        if (!uriPsw.isEmpty()) { /* handling the ftp proxy username and password also */
            password = password.isEmpty() ? uriPsw : password + '@' + uriPsw;
        }
    }

    QString host = uri; /* separating the hostname and path from the uri */
    QString path;
    qsizetype pathStart = uri.indexOf("/");
    if (pathStart != -1) {
        path = host.mid(pathStart);
        host = host.left(pathStart);
    }

    /* setting the parameters of the URL */
    QUrl url;
    url.setScheme(protocol);
    url.setHost(host);
    url.setPath(path);
    if (protocol == "ftp" || protocol == "fish" || protocol == "sftp") {
        url.setPort(p->port->cleanText().toInt());
    }
    if (!username.isEmpty()) {
        url.setUserName(username);
    }
    if (!password.isEmpty()) {
        url.setPassword(password);
    }

    delete p;
    return url;
}

newFTPSub::newFTPSub()
    : newFTPGUI(nullptr)
{
    url->setFocus();
    setGeometry(krMainWindow->x() + krMainWindow->width() / 2 - width() / 2, krMainWindow->y() + krMainWindow->height() / 2 - height() / 2, width(), height());
}

void newFTPSub::accept()
{
    url->addToHistory(url->currentText());
    newFTPGUI::accept();
}

void newFTPSub::reject()
{
    url->lineEdit()->setText("");
    newFTPGUI::reject();
}

newFTPSub::~newFTPSub()
{
    // Save the history and the completion list of the url comboBox
    KConfigGroup group(krConfig, "Private");
    QStringList list = url->completionObject()->items();
    group.writeEntry("newFTP Completion list", list);
    list = url->historyItems();
    group.writeEntry("newFTP History list", list);
    QString protocol = prefix->currentText();
    group.writeEntry("newFTP Protocol", protocol);
}

/////////////////////////// KrMaskChoiceSub ///////////////////////////////
KrMaskChoiceSub::KrMaskChoiceSub(QWidget *parent)
    : KrMaskChoice(parent)
{
    PixmapLabel1->setPixmap(Icon("edit-select").pixmap(32));
    label->setText(i18n("Enter a selection:"));
    // the predefined selections list
    KConfigGroup group(krConfig, "Private");
    QStringList lst = group.readEntry("Predefined Selections", QStringList());
    if (lst.size() > 0)
        preSelections->addItems(lst);
    // the combo-box tweaks
    selection->setDuplicatesEnabled(false);
    selection->addItems(KrSpWidgets::maskList);
    selection->lineEdit()->setText("*");
    selection->lineEdit()->selectAll();
    selection->setFocus();
}

void KrMaskChoiceSub::reject()
{
    selection->clear();
    KrMaskChoice::reject();
}

void KrMaskChoiceSub::accept()
{
    bool add = true;
    // make sure we don't have that already
    for (int i = 0; i != KrSpWidgets::maskList.count(); i++)
        if (KrSpWidgets::maskList[i].simplified() == selection->currentText().simplified()) {
            // break if we found one such as this
            add = false;
            break;
        }

    if (add)
        KrSpWidgets::maskList.insert(0, selection->currentText().toLocal8Bit());
    // write down the predefined selections list
    QStringList list;

    for (int j = 0; j != preSelections->count(); j++) {
        QListWidgetItem *i = preSelections->item(j);
        list.append(i->text());
    }

    KConfigGroup group(krConfig, "Private");
    group.writeEntry("Predefined Selections", list);
    KrMaskChoice::accept();
}

void KrMaskChoiceSub::addSelection()
{
    QString temp = selection->currentText();
    bool itemExists = false;

    // check if the selection already exists
    for (int j = 0; j != preSelections->count(); j++) {
        QListWidgetItem *i = preSelections->item(j);

        if (i->text() == temp) {
            itemExists = true;
            break;
        }
    }

    if (!temp.isEmpty() && !itemExists) {
        preSelections->addItem(selection->currentText());
        preSelections->update();
    }
}

void KrMaskChoiceSub::deleteSelection()
{
    delete preSelections->currentItem();
    preSelections->update();
}

void KrMaskChoiceSub::clearSelections()
{
    preSelections->clear();
    preSelections->update();
}

void KrMaskChoiceSub::acceptFromList(QListWidgetItem *i)
{
    selection->addItem(i->text(), 0);
    accept();
}

void KrMaskChoiceSub::currentItemChanged(QListWidgetItem *i)
{
    if (i)
        selection->setEditText(i->text());
}
