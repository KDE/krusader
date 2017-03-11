/***************************************************************************
                                konfigurator.cpp
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#include "konfigurator.h"
#include "../krglobal.h"
#include "../Dialogs/krdialogs.h"
#include "../kicons.h"

// QtGui
#include <QPixmap>
#include <QResizeEvent>

#include <KConfigCore/KSharedConfig>
#include <KConfigWidgets/KHelpClient>
#include <KI18n/KLocalizedString>
#include <KWidgetsAddons/KMessageBox>

#include "../defaults.h"
#include "../krusaderview.h"
#include "../GUI/kfnkeys.h"

// the frames
#include "kgstartup.h"
#include "kgpanel.h"
#include "kggeneral.h"
#include "kgadvanced.h"
#include "kgarchives.h"
#include "kgdependencies.h"
#include "kgcolors.h"
#include "kguseractions.h"
#include "kgprotocols.h"

Konfigurator::Konfigurator(bool f, int startPage) : KPageDialog((QWidget *)0),
        firstTime(f), internalCall(false), sizeX(-1), sizeY(-1)
{
    setWindowTitle(i18n("Konfigurator - Creating Your Own Krusader"));
    setWindowModality(Qt::ApplicationModal);
    setFaceType(KPageDialog::List);

    setStandardButtons(QDialogButtonBox::Help|QDialogButtonBox::RestoreDefaults|QDialogButtonBox::Close|
                       QDialogButtonBox::Apply|QDialogButtonBox::Reset);
    button(QDialogButtonBox::Apply)->setDefault(true);

    connect(button(QDialogButtonBox::Close), SIGNAL(clicked()), SLOT(slotClose()));
    connect(button(QDialogButtonBox::Help), SIGNAL(clicked()), SLOT(slotShowHelp()));
    connect(button(QDialogButtonBox::RestoreDefaults), SIGNAL(clicked()), SLOT(slotRestoreDefaults()));
    connect(button(QDialogButtonBox::Reset), SIGNAL(clicked()), SLOT(slotReset()));
    connect(button(QDialogButtonBox::Apply), SIGNAL(clicked()), SLOT(slotApply()));
    connect(this, SIGNAL(currentPageChanged(KPageWidgetItem*,KPageWidgetItem*)), this, SLOT(slotPageSwitch(KPageWidgetItem*,KPageWidgetItem*)));
    connect(&restoreTimer, SIGNAL(timeout()), this, SLOT(slotRestorePage()));

    createLayout(startPage);

    KConfigGroup group(krConfig, "Konfigurator");
    int sx = group.readEntry("Window Width", -1);
    int sy = group.readEntry("Window Height", -1);

    if (sx != -1 && sy != -1)
        resize(sx, sy);
    else
        resize(900, 680);

    if (group.readEntry("Window Maximized",  false))
        showMaximized();
    else
        show();
}

void Konfigurator::resizeEvent(QResizeEvent *e)
{
    if (!isMaximized()) {
        sizeX = e->size().width();
        sizeY = e->size().height();
    }
    QDialog::resizeEvent(e);
}

void Konfigurator::closeDialog()
{
    KConfigGroup group(krConfig, "Konfigurator");

    group.writeEntry("Window Width", sizeX);
    group.writeEntry("Window Height", sizeY);
    group.writeEntry("Window Maximized", isMaximized());
}

void Konfigurator::reject()
{
    closeDialog();
    QDialog::reject();
}

void Konfigurator::newPage(KonfiguratorPage *page, const QString &name, const QString &desc, const QIcon &icon)
{
    KPageWidgetItem *item = new KPageWidgetItem(page, name);
    item->setIcon(icon);
    item->setHeader(desc);
    addPage(item);

    kgPages.append(item);
    connect(page, SIGNAL(sigChanged()), this, SLOT(slotApplyEnable()));
}

void Konfigurator::createLayout(int startPage)
{
    // startup
    newPage(new KgStartup(firstTime, this), i18n("Startup"), i18n("Krusader's settings upon startup"), QIcon::fromTheme("go-home"));
    // panel
    newPage(new KgPanel(firstTime, this), i18n("Panel"), i18n("Panel"), QIcon::fromTheme("view-choose"));
    // colors
    newPage(new KgColors(firstTime, this), i18n("Colors"), i18n("Colors"), QIcon::fromTheme("color-picker"));
    // general
    newPage(new KgGeneral(firstTime, this), i18n("General"), i18n("Basic Operations"), QIcon::fromTheme("system-run"));
    // advanced
    newPage(new KgAdvanced(firstTime, this), i18n("Advanced"), i18n("Be sure you know what you are doing."), QIcon::fromTheme("dialog-messages"));
    // archives
    newPage(new KgArchives(firstTime, this), i18n("Archives"), i18n("Customize the way Krusader deals with archives"),
            QIcon::fromTheme("archive-extract"));
    // dependencies
    newPage(new KgDependencies(firstTime, this), i18n("Dependencies"), i18n("Set the full path of the external applications"),
            QIcon::fromTheme("debug-run"));
    // useractions
    newPage(new KgUserActions(firstTime, this), i18n("User Actions"), i18n("Configure your personal actions"),
            QIcon::fromTheme("user-properties"));
    // protocols
    newPage(new KgProtocols(firstTime, this), i18n("Protocols"), i18n("Link MIMEs to protocols"),
            QIcon::fromTheme("document-preview"));

    setCurrentPage(kgPages.at(startPage));
    slotApplyEnable();
}

void Konfigurator::closeEvent(QCloseEvent *event)
{
    lastPage = currentPage();
    if(slotPageSwitch(lastPage, lastPage)) {
        hide();
        KPageDialog::closeEvent(event);
    } else
        event->ignore();
}

void Konfigurator::slotApplyEnable()
{
    lastPage = currentPage();
    bool isChanged = ((KonfiguratorPage *)(lastPage->widget()))->isChanged();
    button(QDialogButtonBox::Apply)->setEnabled(isChanged);
    button(QDialogButtonBox::Reset)->setEnabled(isChanged);
}

bool Konfigurator::slotPageSwitch(KPageWidgetItem *current, KPageWidgetItem *before)
{
    if (before == 0)
        return true;

    KonfiguratorPage *currentPg = (KonfiguratorPage *)(before->widget());

    if (internalCall) {
        internalCall = false;
        return true;
    }

    if (currentPg->isChanged()) {
        int result = KMessageBox::questionYesNoCancel(0, i18n("The current page has been changed. Do you want to apply changes?"));

        switch (result) {
        case KMessageBox::No:
            currentPg->loadInitialValues();
            currentPg->apply();
            break;
        case KMessageBox::Yes:
            emit configChanged(currentPg->apply());
            break;
        default:
            restoreTimer.setSingleShot(true);
            restoreTimer.start(0);
            return false;
        }
    }

    button(QDialogButtonBox::Apply)->setEnabled(currentPg->isChanged());
    lastPage = current;
    return true;
}

void Konfigurator::slotRestorePage()
{
    if (lastPage != currentPage()) {
        internalCall = true;
        setCurrentPage(lastPage);
    }
}

void Konfigurator::slotClose()
{
    lastPage = currentPage();
    if (slotPageSwitch(lastPage, lastPage)) {
        reject();
    }
}

void Konfigurator::slotApply()
{
    emit configChanged(((KonfiguratorPage*)(currentPage()->widget()))->apply());
}

void Konfigurator::slotReset()
{
    ((KonfiguratorPage *)(currentPage()->widget()))->loadInitialValues();
}

void Konfigurator::slotRestoreDefaults()
{
    ((KonfiguratorPage *)(currentPage()->widget()))->setDefaults();
}

void Konfigurator::slotShowHelp()
{
    KHelpClient::invokeHelp(QStringLiteral("konfigurator"));
}

