/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "konfigurator.h"
#include "../Dialogs/krdialogs.h"
#include "../icon.h"
#include "../krglobal.h"

// QtGui
#include <QPixmap>
#include <QResizeEvent>

#include <KHelpClient>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

#include "../GUI/kfnkeys.h"
#include "../defaults.h"
#include "../krusaderview.h"

// the frames
#include "kgadvanced.h"
#include "kgarchives.h"
#include "kgcolors.h"
#include "kgdependencies.h"
#include "kggeneral.h"
#include "kgpanel.h"
#include "kgprotocols.h"
#include "kgstartup.h"
#include "kguseractions.h"

Konfigurator::Konfigurator(bool f, int startPage)
    : KPageDialog((QWidget *)nullptr)
    , firstTime(f)
    , internalCall(false)
    , sizeX(-1)
    , sizeY(-1)
{
    setWindowTitle(i18n("Konfigurator - Creating Your Own Krusader"));
    setWindowModality(Qt::ApplicationModal);
    setFaceType(KPageDialog::List);

    setStandardButtons(QDialogButtonBox::Help | QDialogButtonBox::RestoreDefaults | QDialogButtonBox::Close | QDialogButtonBox::Apply
                       | QDialogButtonBox::Reset);
    button(QDialogButtonBox::Apply)->setDefault(true);

    connect(button(QDialogButtonBox::Close), &QPushButton::clicked, this, &Konfigurator::slotClose);
    connect(button(QDialogButtonBox::Help), &QPushButton::clicked, this, &Konfigurator::slotShowHelp);
    connect(button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, this, &Konfigurator::slotRestoreDefaults);
    connect(button(QDialogButtonBox::Reset), &QPushButton::clicked, this, &Konfigurator::slotReset);
    connect(button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &Konfigurator::slotApply);
    connect(this, &Konfigurator::currentPageChanged, this, &Konfigurator::slotPageSwitch);
    connect(&restoreTimer, &QTimer::timeout, this, &Konfigurator::slotRestorePage);

    createLayout(startPage);

    KConfigGroup group(krConfig, "Konfigurator");
    int sx = group.readEntry("Window Width", -1);
    int sy = group.readEntry("Window Height", -1);

    if (sx != -1 && sy != -1)
        resize(sx, sy);
    else
        resize(900, 680);

    if (group.readEntry("Window Maximized", false))
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
    auto *item = new KPageWidgetItem(page, name);
    item->setIcon(icon);
    item->setHeader(desc);
    addPage(item);

    kgPages.append(item);
    connect(page, &KonfiguratorPage::sigChanged, this, &Konfigurator::slotApplyEnable);
}

void Konfigurator::createLayout(int startPage)
{
    // startup
    newPage(new KgStartup(firstTime, this), i18n("Startup"), i18n("Krusader's settings upon startup"), Icon("go-home"));
    // panel
    newPage(new KgPanel(firstTime, this), i18n("Panel"), i18n("Panel"), Icon("view-choose"));
    // colors
    newPage(new KgColors(firstTime, this), i18n("Colors"), i18n("Colors"), Icon("color-picker"));
    // general
    newPage(new KgGeneral(firstTime, this), i18n("General"), i18n("Basic Operations"), Icon("system-run"));
    // advanced
    newPage(new KgAdvanced(firstTime, this), i18n("Advanced"), i18n("Be sure you know what you are doing."), Icon("dialog-messages"));
    // archives
    newPage(new KgArchives(firstTime, this), i18n("Archives"), i18n("Customize the way Krusader deals with archives"), Icon("archive-extract"));
    // dependencies
    newPage(new KgDependencies(firstTime, this), i18n("Dependencies"), i18n("Set the full path of the external applications"), Icon("debug-run"));
    // useractions
    newPage(new KgUserActions(firstTime, this), i18n("User Actions"), i18n("Configure your personal actions"), Icon("user-properties"));
    // protocols
    newPage(new KgProtocols(firstTime, this), i18n("Protocols"), i18n("Link MIMEs to protocols"), Icon("document-preview"));

    setCurrentPage(kgPages.at(startPage));
    slotApplyEnable();
}

void Konfigurator::closeEvent(QCloseEvent *event)
{
    lastPage = currentPage();
    if (slotPageSwitch(lastPage, lastPage)) {
        hide();
        KPageDialog::closeEvent(event);
    } else
        event->ignore();
}

void Konfigurator::slotApplyEnable()
{
    lastPage = currentPage();
    bool isChanged = (qobject_cast<KonfiguratorPage *>(lastPage->widget()))->isChanged();
    button(QDialogButtonBox::Apply)->setEnabled(isChanged);
    button(QDialogButtonBox::Reset)->setEnabled(isChanged);
}

bool Konfigurator::slotPageSwitch(KPageWidgetItem *current, KPageWidgetItem *before)
{
    if (before == nullptr)
        return true;

    auto *currentPg = qobject_cast<KonfiguratorPage *>(before->widget());

    if (internalCall) {
        internalCall = false;
        return true;
    }

    if (currentPg->isChanged()) {
        int result = KMessageBox::questionTwoActionsCancel(nullptr,
                                                      i18n("The current page has been changed. Do you want to apply changes?"),
                                                      {},
                                                      KStandardGuiItem::apply(),
                                                      KStandardGuiItem::discard());

        switch (result) {
        case KMessageBox::SecondaryAction:
            currentPg->loadInitialValues();
            currentPg->apply();
            break;
        case KMessageBox::PrimaryAction:
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
    emit configChanged((qobject_cast<KonfiguratorPage *>(currentPage()->widget()))->apply());
}

void Konfigurator::slotReset()
{
    (qobject_cast<KonfiguratorPage *>(currentPage()->widget()))->loadInitialValues();
}

void Konfigurator::slotRestoreDefaults()
{
    (qobject_cast<KonfiguratorPage *>(currentPage()->widget()))->setDefaults();
}

void Konfigurator::slotShowHelp()
{
    KHelpClient::invokeHelp(QStringLiteral("konfigurator"));
}
