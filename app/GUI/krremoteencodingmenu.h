/*
    SPDX-FileCopyrightText: 2005 Csaba Karai <cskarai@freemail.hu>
    SPDX-FileCopyrightText: 2005-2022 Krusader Krew <https://krusader.org>

    Based on KRemoteEncodingPlugin from Dawit Alemayehu <adawit@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRREMOTEENCODINGMENU_H
#define KRREMOTEENCODINGMENU_H

// QtCore
#include <QStringList>
// QtWidgets
#include <QAction>

#include <KActionMenu>

class KActionCollection;

class KrRemoteEncodingMenu : public KActionMenu
{
    Q_OBJECT
public:
    KrRemoteEncodingMenu(const QString &text, const QString &iconName, KActionCollection *parent = nullptr);

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
    bool settingsLoaded;
};

#endif /* REMOTEENCODING_MENU_H */
