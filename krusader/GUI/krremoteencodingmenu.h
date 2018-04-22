/*****************************************************************************
 * Copyright (C) 2005 Csaba Karai <cskarai@freemail.hu>                      *
 * Copyright (C) 2005-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * Based on KRemoteEncodingPlugin from Dawit Alemayehu <adawit@kde.org>      *
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

#ifndef KRREMOTEENCODINGMENU_H
#define KRREMOTEENCODINGMENU_H

// QtCore
#include <QStringList>
// QtWidgets
#include <QAction>

# include <KWidgetsAddons/KActionMenu>

class KActionCollection;

class KrRemoteEncodingMenu: public KActionMenu
{
    Q_OBJECT
public:
    KrRemoteEncodingMenu(const QString &text, const QString &iconName, KActionCollection *parent = 0);

protected slots:

    void slotAboutToShow();

    void slotReload();
    void slotTriggered(QAction *);

    virtual void chooseDefault();
    virtual void chooseEncoding(QString encoding);

protected:
    virtual QString currentCharacterSet();

private:

    void loadSettings();
    void updateKIOSlaves();

    QStringList encodingNames;
    bool        settingsLoaded;
};

#endif /* REMOTEENCODING_MENU_H */
