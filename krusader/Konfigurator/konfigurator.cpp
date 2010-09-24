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

#include <kfiledialog.h>
#include <qwidget.h>
#include <QPixmap>
#include <QResizeEvent>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include "../defaults.h"
#include "../krusaderview.h"
#include "../GUI/kfnkeys.h"

// the frames
#include "kgwelcome.h"
#include "kgstartup.h"
#include "kgpanel.h"
#include "kggeneral.h"
#include "kgadvanced.h"
#include "kgarchives.h"
#include "kgdependencies.h"
#include "kgcolors.h"
#include "kguseractions.h"
#include "kgprotocols.h"
#include <QtCore/QEvent>

Konfigurator::Konfigurator(bool f, int startPage) : KPageDialog((QWidget *)0),
        firstTime(f), internalCall(false), sizeX(-1), sizeY(-1)
{
    setButtons(KDialog::Help | KDialog::User1 | KDialog::Apply | KDialog::Cancel);
    setDefaultButton(KDialog::Apply);
    setWindowTitle(i18n("Konfigurator"));
    setButtonGuiItem(KDialog::User1, KGuiItem(i18n("Defaults")));
    setWindowModality(Qt::WindowModal);

    setPlainCaption(i18n("Konfigurator - Creating Your Own Krusader"));
    setFaceType(KPageDialog::List);

    setButtonGuiItem(KDialog::Cancel, KGuiItem(i18n("Close")));

    setHelp("konfigurator");

    connect(this, SIGNAL(currentPageChanged(KPageWidgetItem *, KPageWidgetItem *)), this, SLOT(slotPageSwitch(KPageWidgetItem *, KPageWidgetItem *)));
    connect(&restoreTimer, SIGNAL(timeout()), this, SLOT(slotRestorePage()));
    connect(this, SIGNAL(applyClicked()), this, SLOT(slotApply()));
    connect(this, SIGNAL(cancelClicked()), this, SLOT(slotCancel())); 
    connect(this, SIGNAL(user1Clicked()), this, SLOT(slotUser1()));

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

    KDialog::resizeEvent(e);
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
    KDialog::reject();
}

void Konfigurator::accept()
{
    closeDialog();
    KDialog::accept();
}

void Konfigurator::newPage(KonfiguratorPage *page, const QString &name, const QString &desc, const KIcon &kicon)
{
    KPageWidgetItem *item = new KPageWidgetItem(page, name);
    item->setIcon(kicon);
    item->setHeader(desc);
    addPage(item);

    kgPages.append(item);
    connect(page, SIGNAL(sigChanged()), this, SLOT(slotApplyEnable()));
}

void Konfigurator::createLayout(int startPage)
{
    // startup
    newPage(new KgStartup(firstTime, this), i18n("Startup"), i18n("Krusader's settings upon startup"), KIcon("go-home", krLoader));
    // panel
    newPage(new KgPanel(firstTime, this), i18n("Panel"), i18n("Panel"), KIcon("view-choose", krLoader));
    // colors
    newPage(new KgColors(firstTime, this), i18n("Colors"), i18n("Colors"), KIcon("preferences-desktop-color", krLoader));
    // general
    newPage(new KgGeneral(firstTime, this), i18n("General"), i18n("Basic Operations"), KIcon("preferences-other", krLoader));
    // advanced
    newPage(new KgAdvanced(firstTime, this), i18n("Advanced"), i18n("Be sure you know what you're doing!"), KIcon("dialog-warning", krLoader));
    // archives
    newPage(new KgArchives(firstTime, this), i18n("Archives"), i18n("Customize the way Krusader deals with archives"),
            KIcon("utilities-file-archiver", krLoader));
    // dependencies
    newPage(new KgDependencies(firstTime, this), i18n("Dependencies"), i18n("Set the full path of the external applications"),
            KIcon("kr_dependencies", krLoader));
    // useractions
    newPage(new KgUserActions(firstTime, this), i18n("User Actions"), i18n("Configure your personal actions"),
            KIcon("user-properties", krLoader));
    // protocols
    newPage(new KgProtocols(firstTime, this), i18n("Protocols"), i18n("Link mimes to protocols"),
            KIcon("kde", krLoader));

    setCurrentPage(kgPages.at(startPage));
    slotApplyEnable();
}

void Konfigurator::slotUser1()
{
    ((KonfiguratorPage *)(currentPage()->widget()))->setDefaults();
}

void Konfigurator::slotApply()
{
    emit configChanged(((KonfiguratorPage*)(currentPage()->widget()))->apply());
}

void Konfigurator::slotCancel()
{
    lastPage = currentPage();
    if (slotPageSwitch(lastPage, lastPage))
        reject();
}

void Konfigurator::slotApplyEnable()
{
    lastPage = currentPage();
    enableButtonApply(((KonfiguratorPage *)(lastPage->widget()))->isChanged());
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

    enableButtonApply(currentPg->isChanged());
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

#include "konfigurator.moc"
