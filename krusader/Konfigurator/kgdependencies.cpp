/*****************************************************************************
 * Copyright (C) 2004 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2004-2020 Krusader Krew [https://krusader.org]              *
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

#include "kgdependencies.h"
#include "../krservices.h"
#include "../krglobal.h"
#include "../compat.h"

// QtCore
#include <QUrl>
// QtWidgets
#include <QTabWidget>
#include <QGridLayout>
#include <QScrollArea>

#include <KConfigCore/KSharedConfig>
#include <KI18n/KLocalizedString>
#include <KWidgetsAddons/KMessageBox>

#define PAGE_GENERAL   0
#define PAGE_PACKERS   1
#define PAGE_CHECKSUM  2

KgDependencies::KgDependencies(bool first, QWidget* parent) :
        KonfiguratorPage(first, parent)
{
    auto *kgDependenciesLayout = new QGridLayout(this);
    kgDependenciesLayout->setSpacing(6);

    //  ---------------------------- GENERAL TAB -------------------------------------
    tabWidget = new QTabWidget(this);

    QWidget *general_tab = new QWidget(tabWidget);
    auto* general_scroll = new QScrollArea(tabWidget);
    general_scroll->setFrameStyle(QFrame::NoFrame);
    general_scroll->setWidget(general_tab);   // this also sets scrollacrea as the new parent for widget
    general_scroll->setWidgetResizable(true);   // let the widget use every space available
    tabWidget->addTab(general_scroll, i18n("General"));

    auto *pathsGrid = new QGridLayout(general_tab);
    pathsGrid->setSpacing(6);
    pathsGrid->setContentsMargins(11, 11, 11, 11);
    pathsGrid->setAlignment(Qt::AlignTop);

    addApplication("kget",     pathsGrid, 0, general_tab, PAGE_GENERAL);
    addApplication("mailer",   pathsGrid, 1, general_tab, PAGE_GENERAL);
    addApplication("diff utility",  pathsGrid, 2, general_tab, PAGE_GENERAL);
    addApplication("krename",  pathsGrid, 3, general_tab, PAGE_GENERAL);
    addApplication("locate",   pathsGrid, 4, general_tab, PAGE_GENERAL);
    addApplication("mount",    pathsGrid, 5, general_tab, PAGE_GENERAL);
    addApplication("umount",   pathsGrid, 6, general_tab, PAGE_GENERAL);
    addApplication("updatedb", pathsGrid, 7, general_tab, PAGE_GENERAL);

    //  ---------------------------- PACKERS TAB -------------------------------------
    QWidget *packers_tab = new QWidget(tabWidget);
    auto* packers_scroll = new QScrollArea(tabWidget);
    packers_scroll->setFrameStyle(QFrame::NoFrame);
    packers_scroll->setWidget(packers_tab);   // this also sets scrollacrea as the new parent for widget
    packers_scroll->setWidgetResizable(true);   // let the widget use every space available
    tabWidget->addTab(packers_scroll, i18n("Packers"));

    auto *archGrid1 = new QGridLayout(packers_tab);
    archGrid1->setSpacing(6);
    archGrid1->setContentsMargins(11, 11, 11, 11);
    archGrid1->setAlignment(Qt::AlignTop);

    addApplication("7z",    archGrid1, 0, packers_tab, PAGE_PACKERS, "7za");
    addApplication("arj",   archGrid1, 1, packers_tab, PAGE_PACKERS);
    addApplication("bzip2", archGrid1, 2, packers_tab, PAGE_PACKERS);
    addApplication("cpio",  archGrid1, 3, packers_tab, PAGE_PACKERS);
    addApplication("dpkg",  archGrid1, 4, packers_tab, PAGE_PACKERS);
    addApplication("gzip",  archGrid1, 5, packers_tab, PAGE_PACKERS);
    addApplication("lha",   archGrid1, 6, packers_tab, PAGE_PACKERS);
    addApplication("lzma",  archGrid1, 7, packers_tab, PAGE_PACKERS);
    addApplication("rar",   archGrid1, 8, packers_tab, PAGE_PACKERS);
    addApplication("tar",   archGrid1, 9, packers_tab, PAGE_PACKERS);
    addApplication("unace", archGrid1, 10, packers_tab, PAGE_PACKERS);
    addApplication("unarj", archGrid1, 11, packers_tab, PAGE_PACKERS);
    addApplication("unrar", archGrid1, 12, packers_tab, PAGE_PACKERS);
    addApplication("unzip", archGrid1, 13, packers_tab, PAGE_PACKERS);
    addApplication("zip",   archGrid1, 14, packers_tab, PAGE_PACKERS);
    addApplication("xz",    archGrid1, 15, packers_tab, PAGE_PACKERS);

    //  ---------------------------- CHECKSUM TAB -------------------------------------
    QWidget *checksum_tab = new QWidget(tabWidget);
    auto* checksum_scroll = new QScrollArea(tabWidget);
    checksum_scroll->setFrameStyle(QFrame::NoFrame);
    checksum_scroll->setWidget(checksum_tab);   // this also sets scrollacrea as the new parent for widget
    checksum_scroll->setWidgetResizable(true);   // let the widget use every space available
    tabWidget->addTab(checksum_scroll, i18n("Checksum Utilities"));

    auto *archGrid2 = new QGridLayout(checksum_tab);
    archGrid2->setSpacing(6);
    archGrid2->setContentsMargins(11, 11, 11, 11);
    archGrid2->setAlignment(Qt::AlignTop);

    addApplication("md5sum",         archGrid2, 0, checksum_tab, PAGE_CHECKSUM);
    addApplication("sha1sum",        archGrid2, 1, checksum_tab, PAGE_CHECKSUM);
    addApplication("sha224sum",      archGrid2, 2, checksum_tab, PAGE_CHECKSUM);
    addApplication("sha256sum",      archGrid2, 3, checksum_tab, PAGE_CHECKSUM);
    addApplication("sha384sum",      archGrid2, 4, checksum_tab, PAGE_CHECKSUM);
    addApplication("sha512sum",      archGrid2, 5, checksum_tab, PAGE_CHECKSUM);

    kgDependenciesLayout->addWidget(tabWidget, 0, 0);
}

void KgDependencies::addApplication(const QString& name, QGridLayout *grid, int row, QWidget *parent,
                                    int page, const QString& additionalList)
{
    // try to autodetect the full path name
    QString defaultValue = KrServices::fullPathName(name);

    if (defaultValue.isEmpty()) {
        QStringList list = additionalList.split(',', SKIP_EMPTY_PARTS);
        for (int i = 0; i != list.count(); i++)
            if (!KrServices::fullPathName(list[ i ]).isEmpty()) {
                defaultValue = KrServices::fullPathName(list[ i ]);
                break;
            }
    }

    QLabel *labelPath = addLabel(grid, row, 0, name, parent);

    KonfiguratorURLRequester *fullPath =
        createURLRequester("Dependencies", name, defaultValue, labelPath,
                           parent, false, QString(), page);
    connect(fullPath->extension(), &KonfiguratorExtension::applyManually, this, &KgDependencies::slotApply);
    grid->addWidget(fullPath, row, 1);
}

void KgDependencies::slotApply(QObject *obj, const QString& configGroup, const QString& name)
{
    auto *urlRequester = qobject_cast<KonfiguratorURLRequester *>( obj);

    KConfigGroup group(krConfig, configGroup);
    group.writeEntry(name, urlRequester->url().toDisplayString(QUrl::PreferLocalFile));

    QString usedPath = KrServices::fullPathName(name);

    if (urlRequester->url().toDisplayString(QUrl::PreferLocalFile) != usedPath) {
        group.writeEntry(name, usedPath);
        if (usedPath.isEmpty())
            KMessageBox::error(this,
                               i18n("The %1 path is incorrect, no valid path found.",
                                    urlRequester->url().toDisplayString(QUrl::PreferLocalFile)));
        else
            KMessageBox::error(
                this, i18n("The %1 path is incorrect, %2 used instead.",
                           urlRequester->url().toDisplayString(QUrl::PreferLocalFile), usedPath));
        urlRequester->setUrl(QUrl::fromLocalFile(usedPath));
    }
}

int KgDependencies::activeSubPage()
{
    return tabWidget->currentIndex();
}
