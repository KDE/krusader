/*****************************************************************************
 * Copyright (C) 2004 Csaba Karai <krusader@users.sourceforge.net>           *
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

#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

// QtCore
#include <QString>
// QtWidgets
#include <QPushButton>

/**
 * A generic profile manager: Profiles are arbitray configurations groups and the manager handles
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
    explicit ProfileManager(QString profileType, QWidget * parent = 0);

    /**
     * @param profileType Type of the profile (sync, search, ...)
     * @return A list of all available profile-names
     */
    static QStringList availableProfiles(QString profileType);

    QStringList getNames();

public slots:
    void profilePopup();

    void newProfile(QString defaultName = QString());
    void deleteProfile(QString name);
    void overwriteProfile(QString name);
    bool loadProfile(QString name);

signals:
    void saveToProfile(QString profileName);
    void loadFromProfile(QString profileName);

private:
    QString profileType;
    QStringList profileList;
};

#endif /* PROFILEMANAGER_H */
