/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kgprotocols.h"
#include "../Archive/krarchandler.h"
#include "../icon.h"
#include "../krglobal.h"
#include "../krservices.h"

// QtCore
#include <QMimeDatabase>
#include <QMimeType>
// QtWidgets
#include <QGridLayout>
#include <QHeaderView>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KProtocolManager>
#include <KSharedConfig>

KgProtocols::KgProtocols(bool first, QWidget *parent)
    : KonfiguratorPage(first, parent)
{
    auto *KgProtocolsLayout = new QGridLayout(this);
    KgProtocolsLayout->setSpacing(6);

    //  -------------------------- LINK VIEW ----------------------------------

    QGroupBox *linkGrp = createFrame(i18n("Links"), this);
    QGridLayout *linkGrid = createGridLayout(linkGrp);

    QStringList labels;
    labels << i18n("Defined Links");

    linkList = new KrTreeWidget(linkGrp);
    linkList->setHeaderLabels(labels);
    linkList->setRootIsDecorated(true);

    linkGrid->addWidget(linkList, 0, 0);
    KgProtocolsLayout->addWidget(linkGrp, 0, 0, 2, 1);

    //  -------------------------- BUTTONS ----------------------------------

    QWidget *vbox1Widget = new QWidget(this);
    auto *vbox1 = new QVBoxLayout(vbox1Widget);

    addSpacer(vbox1);
    btnAddProtocol = new QPushButton(vbox1Widget);
    btnAddProtocol->setIcon(Icon("arrow-left"));
    btnAddProtocol->setWhatsThis(i18n("Add protocol to the link list."));
    vbox1->addWidget(btnAddProtocol);

    btnRemoveProtocol = new QPushButton(vbox1Widget);
    btnRemoveProtocol->setIcon(Icon("arrow-right"));
    btnRemoveProtocol->setWhatsThis(i18n("Remove protocol from the link list."));
    vbox1->addWidget(btnRemoveProtocol);
    addSpacer(vbox1);

    KgProtocolsLayout->addWidget(vbox1Widget, 0, 1);

    QWidget *vbox2Widget = new QWidget(this);
    auto *vbox2 = new QVBoxLayout(vbox2Widget);

    addSpacer(vbox2);
    btnAddMime = new QPushButton(vbox2Widget);
    btnAddMime->setIcon(Icon("arrow-left"));
    btnAddMime->setWhatsThis(i18n("Add MIME to the selected protocol on the link list."));
    vbox2->addWidget(btnAddMime);

    btnRemoveMime = new QPushButton(vbox2Widget);
    btnRemoveMime->setIcon(Icon("arrow-right"));
    btnRemoveMime->setWhatsThis(i18n("Remove MIME from the link list."));
    vbox2->addWidget(btnRemoveMime);
    addSpacer(vbox2);

    KgProtocolsLayout->addWidget(vbox2Widget, 1, 1);

    //  -------------------------- PROTOCOLS LISTBOX ----------------------------------

    QGroupBox *protocolGrp = createFrame(i18n("Protocols"), this);
    QGridLayout *protocolGrid = createGridLayout(protocolGrp);

    protocolList = new KrListWidget(protocolGrp);
    loadProtocols();
    protocolGrid->addWidget(protocolList, 0, 0);

    KgProtocolsLayout->addWidget(protocolGrp, 0, 2);

    //  -------------------------- MIMES LISTBOX ----------------------------------

    QGroupBox *mimeGrp = createFrame(i18n("MIMEs"), this);
    QGridLayout *mimeGrid = createGridLayout(mimeGrp);

    mimeList = new KrListWidget(mimeGrp);
    loadMimes();
    mimeGrid->addWidget(mimeList, 0, 0);

    KgProtocolsLayout->addWidget(mimeGrp, 1, 2);

    //  -------------------------- CONNECT TABLE ----------------------------------

    connect(protocolList, &KrListWidget::itemSelectionChanged, this, &KgProtocols::slotDisableButtons);
    connect(linkList, &KrTreeWidget::itemSelectionChanged, this, &KgProtocols::slotDisableButtons);
    connect(mimeList, &KrListWidget::itemSelectionChanged, this, &KgProtocols::slotDisableButtons);
    connect(linkList, &KrTreeWidget::currentItemChanged, this, &KgProtocols::slotDisableButtons);
    connect(btnAddProtocol, &QPushButton::clicked, this, &KgProtocols::slotAddProtocol);
    connect(btnRemoveProtocol, &QPushButton::clicked, this, &KgProtocols::slotRemoveProtocol);
    connect(btnAddMime, &QPushButton::clicked, this, &KgProtocols::slotAddMime);
    connect(btnRemoveMime, &QPushButton::clicked, this, &KgProtocols::slotRemoveMime);

    loadInitialValues();
    slotDisableButtons();
}

void KgProtocols::addSpacer(QBoxLayout *layout)
{
    layout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

void KgProtocols::loadProtocols()
{
    QStringList protocols = KProtocolInfo::protocols();
    protocols.sort();

    for (const QString &protocol : std::as_const(protocols)) {
        QUrl u;
        u.setScheme(protocol);
        if (KProtocolManager::inputType(u) == KProtocolInfo::T_FILESYSTEM) {
            protocolList->addItem(protocol);
        }
    }
}

void KgProtocols::loadMimes()
{
    QMimeDatabase db;
    QList<QMimeType> mimes = db.allMimeTypes();

    for (QList<QMimeType>::const_iterator it = mimes.constBegin(); it != mimes.constEnd(); ++it)
        mimeList->addItem((*it).name());

    mimeList->sortItems();
}

void KgProtocols::slotDisableButtons()
{
    btnAddProtocol->setEnabled(protocolList->selectedItems().count() != 0);
    QTreeWidgetItem *listViewItem = linkList->currentItem();
    bool isProtocolSelected = (listViewItem == nullptr ? false : listViewItem->parent() == nullptr);
    btnRemoveProtocol->setEnabled(isProtocolSelected);
    btnAddMime->setEnabled(listViewItem != nullptr && mimeList->selectedItems().count() != 0);
    btnRemoveMime->setEnabled(listViewItem == nullptr ? false : listViewItem->parent() != nullptr);

    if (linkList->currentItem() == nullptr && linkList->topLevelItemCount() != 0)
        linkList->setCurrentItem(linkList->topLevelItem(0));

    QList<QTreeWidgetItem *> list = linkList->selectedItems();
    if (list.count() == 0 && linkList->currentItem() != nullptr)
        linkList->currentItem()->setSelected(true);
}

void KgProtocols::slotAddProtocol()
{
    QList<QListWidgetItem *> list = protocolList->selectedItems();

    if (list.count() > 0) {
        addProtocol(list[0]->text(), true);
        slotDisableButtons();
        emit sigChanged();
    }
}

void KgProtocols::addProtocol(const QString &name, bool changeCurrent)
{
    QList<QListWidgetItem *> list = protocolList->findItems(name, Qt::MatchExactly);
    if (list.count() > 0) {
        delete list[0];
        auto *listViewItem = new QTreeWidgetItem(linkList);
        listViewItem->setText(0, name);
        QString iconName = KProtocolInfo::icon(name);
        if (iconName.isEmpty())
            iconName = "go-next-view";
        listViewItem->setIcon(0, Icon(iconName));

        if (changeCurrent)
            linkList->setCurrentItem(listViewItem);
    }
}

void KgProtocols::slotRemoveProtocol()
{
    QTreeWidgetItem *item = linkList->currentItem();
    if (item) {
        removeProtocol(item->text(0));
        slotDisableButtons();
        emit sigChanged();
    }
}

void KgProtocols::removeProtocol(const QString &name)
{
    QList<QTreeWidgetItem *> itemList = linkList->findItems(name, Qt::MatchExactly, 0);

    if (itemList.count()) {
        QTreeWidgetItem *item = itemList[0];

        while (item->childCount() != 0)
            removeMime(item->child(0)->text(0));

        linkList->takeTopLevelItem(linkList->indexOfTopLevelItem(item));
        protocolList->addItem(name);
        protocolList->sortItems();
    }
}

void KgProtocols::slotAddMime()
{
    QList<QListWidgetItem *> list = mimeList->selectedItems();
    if (list.count() > 0 && linkList->currentItem() != nullptr) {
        QTreeWidgetItem *itemToAdd = linkList->currentItem();
        if (itemToAdd->parent())
            itemToAdd = itemToAdd->parent();

        addMime(list[0]->text(), itemToAdd->text(0));
        slotDisableButtons();
        emit sigChanged();
    }
}

void KgProtocols::addMime(QString name, const QString &protocol)
{
    QList<QListWidgetItem *> list = mimeList->findItems(name, Qt::MatchExactly);

    QList<QTreeWidgetItem *> itemList = linkList->findItems(protocol, Qt::MatchExactly | Qt::MatchRecursive, 0);

    QTreeWidgetItem *currentListItem = nullptr;
    if (itemList.count() != 0)
        currentListItem = itemList[0];

    if (list.count() > 0 && currentListItem && currentListItem->parent() == nullptr) {
        delete list[0];
        auto *listViewItem = new QTreeWidgetItem(currentListItem);
        listViewItem->setText(0, name);
        listViewItem->setIcon(0, Icon(name.replace(QLatin1Char('/'), QLatin1Char('-')), Icon("unknown")));
        linkList->expandItem(currentListItem);
    }
}

void KgProtocols::slotRemoveMime()
{
    QTreeWidgetItem *item = linkList->currentItem();
    if (item) {
        removeMime(item->text(0));
        slotDisableButtons();
        emit sigChanged();
    }
}

void KgProtocols::removeMime(const QString &name)
{
    QList<QTreeWidgetItem *> itemList = linkList->findItems(name, Qt::MatchExactly | Qt::MatchRecursive, 0);

    QTreeWidgetItem *currentMimeItem = nullptr;
    if (itemList.count() != 0)
        currentMimeItem = itemList[0];

    if (currentMimeItem && currentMimeItem->parent() != nullptr) {
        mimeList->addItem(currentMimeItem->text(0));
        mimeList->sortItems();
        currentMimeItem->parent()->removeChild(currentMimeItem);
    }
}

void KgProtocols::loadInitialValues()
{
    if (linkList->model()->rowCount() > 0)
        while (linkList->topLevelItemCount() != 0)
            removeProtocol(linkList->topLevelItem(0)->text(0));

    KConfigGroup group(krConfig, "Protocols");
    QStringList protList = group.readEntry("Handled Protocols", QStringList());

    for (auto &it : protList) {
        addProtocol(it);

        QStringList mimes = group.readEntry(QString("Mimes For %1").arg(it), QStringList());

        for (auto &mime : mimes)
            addMime(mime, it);
    }

    if (linkList->topLevelItemCount() != 0)
        linkList->setCurrentItem(linkList->topLevelItem(0));
    slotDisableButtons();
    linkList->expandAll();
    emit sigChanged();
}

void KgProtocols::setDefaults()
{
    while (linkList->topLevelItemCount() != 0)
        removeProtocol(linkList->topLevelItem(0)->text(0));

    slotDisableButtons();

    if (isChanged())
        emit sigChanged();
}

bool KgProtocols::isChanged()
{
    KConfigGroup group(krConfig, "Protocols");
    QStringList protList = group.readEntry("Handled Protocols", QStringList());

    if ((int)protList.count() != linkList->topLevelItemCount())
        return true;

    for (int i = 0; i != linkList->topLevelItemCount(); i++) {
        QTreeWidgetItem *item = linkList->topLevelItem(i);

        if (!protList.contains(item->text(0)))
            return true;

        QStringList mimes = group.readEntry(QString("Mimes For %1").arg(item->text(0)), QStringList());

        if ((int)mimes.count() != item->childCount())
            return true;

        for (int j = 0; j != item->childCount(); j++) {
            QTreeWidgetItem *children = item->child(j);

            if (!mimes.contains(children->text(0)))
                return true;
        }
    }

    return false;
}

bool KgProtocols::apply()
{
    KConfigGroup group(krConfig, "Protocols");

    QStringList protocolList;

    for (int i = 0; i != linkList->topLevelItemCount(); i++) {
        QTreeWidgetItem *item = linkList->topLevelItem(i);

        protocolList.append(item->text(0));

        QStringList mimes;

        for (int j = 0; j != item->childCount(); j++) {
            QTreeWidgetItem *children = item->child(j);

            mimes.append(children->text(0));
        }
        group.writeEntry(QString("Mimes For %1").arg(item->text(0)), mimes);
    }
    group.writeEntry("Handled Protocols", protocolList);
    krConfig->sync();

    KrArcHandler::clearProtocolCache();

    emit sigChanged();
    return false;
}

void KgProtocols::init()
{
}
