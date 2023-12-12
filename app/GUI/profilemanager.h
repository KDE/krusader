/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

// QtCore
#include <QString>
// QtWidgets
#include <QPushButton>

/**
 * A generic profile manager: Profiles are arbitrary configurations groups and the manager handles
 * saving/loading multiple groups to/from the configuration.
 *
 * A manager instance is responsible for a profile type specified by a type name:
 * "Panel", "SelectionProfile", "SearcherProfile", "SynchronizerProfile", ...
 *
 * Profiles are numbered in the configuration group name and have an additional name property. E.g.
 *
 * [Panel - 2]
 * Name=Media Profile
 * ...
 *
 * This class is view and model at the same time :/
 * The GUI button opens a popup menu for selecting, creating, overwriting and removing profiles.
 */
class ProfileManager : public QPushButton
{
    Q_OBJECT

public:
    explicit ProfileManager(const QString &profileType, QWidget *parent = nullptr);

    /**
     * @param profileType Type of the profile (sync, search, ...)
     * @return A list of all available profile-names
     */
    static QStringList availableProfiles(const QString &profileType);

    QStringList getNames();

public slots:
    void profilePopup();

    void newProfile(const QString &defaultName = QString());
    void deleteProfile(const QString &name);
    void overwriteProfile(const QString &name);
    bool loadProfile(const QString &name);

signals:
    void saveToProfile(QString profileName);
    void loadFromProfile(QString profileName);

private:
    QString profileType;
    QStringList profileList;
};

#endif /* PROFILEMANAGER_H */
