/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "profilemanager.h"

// QtGui
#include <QCursor>
// QtWidgets
#include <QInputDialog>
#include <QMenu>

#include <KLocalizedString>
#include <KSharedConfig>

#include "../icon.h"
#include "../krglobal.h"

ProfileManager::ProfileManager(const QString &profileType, QWidget *parent)
    : QPushButton(parent)
{
    setText("");
    setIcon(Icon("user-identity"));
    setFixedWidth(16 + 15);
    setToolTip(i18n("Profiles"));

    this->profileType = profileType;

    connect(this, &ProfileManager::clicked, this, &ProfileManager::profilePopup);

    KConfigGroup group(krConfig, "Private");
    profileList = group.readEntry(profileType, QStringList());
}

void ProfileManager::profilePopup()
{
    // profile menu identifiers
#define ADD_NEW_ENTRY_ID 1000
#define LOAD_ENTRY_ID 2000
#define REMOVE_ENTRY_ID 3000
#define OVERWRITE_ENTRY_ID 4000

    // create the menu
    QMenu popup, removePopup, overwritePopup;
    popup.setTitle(i18n("Profiles"));

    for (int i = 0; i != profileList.count(); i++) {
        KConfigGroup group(krConfig, profileType + " - " + profileList[i]);
        QString name = group.readEntry("Name");
        popup.addAction(name)->setData(QVariant((int)(LOAD_ENTRY_ID + i)));
        removePopup.addAction(name)->setData(QVariant((int)(REMOVE_ENTRY_ID + i)));
        overwritePopup.addAction(name)->setData(QVariant((int)(OVERWRITE_ENTRY_ID + i)));
    }

    popup.addSeparator();

    if (profileList.count()) {
        popup.addMenu(&removePopup)->setText(i18n("Remove entry"));
        popup.addMenu(&overwritePopup)->setText(i18n("Overwrite entry"));
    }

    popup.addAction(i18n("Add new entry"))->setData(QVariant((int)(ADD_NEW_ENTRY_ID)));

    int result = 0;
    QAction *res = popup.exec(QCursor::pos());
    if (res && res->data().canConvert<int>())
        result = res->data().toInt();

    // check out the user's selection
    if (result == ADD_NEW_ENTRY_ID)
        newProfile();
    else if (result >= LOAD_ENTRY_ID && result < LOAD_ENTRY_ID + profileList.count()) {
        emit loadFromProfile(profileType + " - " + profileList[result - LOAD_ENTRY_ID]);
    } else if (result >= REMOVE_ENTRY_ID && result < REMOVE_ENTRY_ID + profileList.count()) {
        krConfig->deleteGroup(profileType + " - " + profileList[result - REMOVE_ENTRY_ID]);
        profileList.removeAll(profileList[result - REMOVE_ENTRY_ID]);

        KConfigGroup group(krConfig, "Private");
        group.writeEntry(profileType, profileList);
        krConfig->sync();
    } else if (result >= OVERWRITE_ENTRY_ID && result < OVERWRITE_ENTRY_ID + profileList.count()) {
        emit saveToProfile(profileType + " - " + profileList[result - OVERWRITE_ENTRY_ID]);
    }
}

void ProfileManager::newProfile(const QString &defaultName)
{
    QString profile = QInputDialog::getText(this, i18n("Krusader::ProfileManager"), i18n("Enter the profile name:"), QLineEdit::Normal, defaultName);
    if (!profile.isEmpty()) {
        int profileNum = 1;
        while (profileList.contains(QString("%1").arg(profileNum)))
            profileNum++;

        QString profileString = QString("%1").arg(profileNum);
        QString profileName = profileType + " - " + profileString;
        profileList.append(QString("%1").arg(profileString));

        KConfigGroup group(krConfig, "Private");
        group.writeEntry(profileType, profileList);

        KConfigGroup pg(krConfig, profileName);
        pg.writeEntry("Name", profile);
        emit saveToProfile(profileName);
        krConfig->sync();
    }
}

void ProfileManager::deleteProfile(const QString &name)
{
    for (int i = 0; i != profileList.count(); i++) {
        KConfigGroup group(krConfig, profileType + " - " + profileList[i]);
        QString currentName = group.readEntry("Name");

        if (name == currentName) {
            krConfig->deleteGroup(profileType + " - " + profileList[i]);
            profileList.removeAll(profileList[i]);

            KConfigGroup pg(krConfig, "Private");
            pg.writeEntry(profileType, profileList);
            krConfig->sync();
            return;
        }
    }
}

void ProfileManager::overwriteProfile(const QString &name)
{
    for (int i = 0; i != profileList.count(); i++) {
        KConfigGroup group(krConfig, profileType + " - " + profileList[i]);
        QString currentName = group.readEntry("Name");

        if (name == currentName) {
            emit saveToProfile(profileType + " - " + profileList[i]);
            return;
        }
    }
}

bool ProfileManager::loadProfile(const QString &name)
{
    for (int i = 0; i != profileList.count(); i++) {
        KConfigGroup group(krConfig, profileType + " - " + profileList[i]);
        QString currentName = group.readEntry("Name");

        if (name == currentName) {
            emit loadFromProfile(profileType + " - " + profileList[i]);
            return true;
        }
    }
    return false;
}

QStringList ProfileManager::availableProfiles(const QString &profileType)
{
    KConfigGroup group(krConfig, "Private");
    QStringList profiles = group.readEntry(profileType, QStringList());
    QStringList profileNames;

    for (int i = 0; i != profiles.count(); i++) {
        KConfigGroup pg(krConfig, profileType + " - " + profiles[i]);
        profileNames.append(pg.readEntry("Name"));
    }

    return profileNames;
}
