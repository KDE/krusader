/*
    SPDX-FileCopyrightText: 2006 Jonas Bähr <jonas.baehr@web.de>
    SPDX-FileCopyrightText: 2006-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "useractionlistview.h"

// QtCore
#include <QDebug>
// QtXml
#include <QDomEntity>

#include <KLocalizedString>

#include "../UserAction/kraction.h"
#include "../UserAction/useraction.h"
#include "../icon.h"
#include "../krglobal.h"

#define COL_TITLE 0

// UserActionListView

UserActionListView::UserActionListView(QWidget *parent)
    : KrTreeWidget(parent)
{
    setHeaderLabel(i18n("Title"));

    setRootIsDecorated(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection); // normally select single items but one may use Ctrl or Shift to select multiple

    setSortingEnabled(true);
    sortItems(COL_TITLE, Qt::AscendingOrder);

    connect(this, &UserActionListView::currentItemChanged, this, &UserActionListView::slotCurrentItemChanged);

    update();
}

UserActionListView::~UserActionListView() = default;

QSize UserActionListView::sizeHint() const
{
    return QSize(200, 400);
}

void UserActionListView::update()
{
    clear();
    UserAction::KrActionList list = krUserAction->actionList();

    QListIterator<KrAction *> it(list);
    while (it.hasNext())
        insertAction(it.next());
}

void UserActionListView::update(KrAction *action)
{
    UserActionListViewItem *item = findActionItem(action);
    if (item) {
        // deleting & re-inserting is _much_easier then tracking all possible cases of category changes!
        bool current = (item == currentItem());
        bool selected = item->isSelected();
        delete item;
        item = insertAction(action);
        if (current)
            setCurrentItem(item);
        if (selected)
            item->setSelected(true);
    }
}

UserActionListViewItem *UserActionListView::insertAction(KrAction *action)
{
    if (!action)
        return nullptr;

    UserActionListViewItem *item;

    if (action->category().isEmpty())
        item = new UserActionListViewItem(this, action);
    else {
        QTreeWidgetItem *categoryItem = findCategoryItem(action->category());
        if (!categoryItem) {
            categoryItem = new QTreeWidgetItem(this); // create the new category item it not already present
            categoryItem->setText(0, action->category());
            categoryItem->setFlags(Qt::ItemIsEnabled);
        }
        item = new UserActionListViewItem(categoryItem, action);
    }

    item->setAction(action);
    return item;
}

QTreeWidgetItem *UserActionListView::findCategoryItem(const QString &category)
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        if ((*it)->text(COL_TITLE) == category && !((*it)->flags() & Qt::ItemIsSelectable))
            return *it;
        it++;
    }
    return nullptr;
}

UserActionListViewItem *UserActionListView::findActionItem(const KrAction *action)
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        if (auto *item = dynamic_cast<UserActionListViewItem *>(*it)) {
            if (item->action() == action)
                return item;
        }
        it++;
    }
    return nullptr;
}

KrAction *UserActionListView::currentAction() const
{
    if (auto *item = dynamic_cast<UserActionListViewItem *>(currentItem()))
        return item->action();
    else
        return nullptr;
}

void UserActionListView::setCurrentAction(const KrAction *action)
{
    UserActionListViewItem *item = findActionItem(action);
    if (item) {
        setCurrentItem(item);
    }
}

void UserActionListView::setFirstActionCurrent()
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        if (auto *item = dynamic_cast<UserActionListViewItem *>(*it)) {
            setCurrentItem(item);
            break;
        }
        it++;
    }
}

void UserActionListView::slotCurrentItemChanged(QTreeWidgetItem *item)
{
    if (!item)
        return;
    scrollTo(indexOf(item));
}

QDomDocument UserActionListView::dumpSelectedActions(QDomDocument *mergeDoc) const
{
    const QList<QTreeWidgetItem *> list = selectedItems();
    QDomDocument doc;
    if (mergeDoc)
        doc = *mergeDoc;
    else
        doc = UserAction::createEmptyDoc();
    QDomElement root = doc.documentElement();

    for (auto item : list) {
        if (auto *actionItem = dynamic_cast<UserActionListViewItem *>(item))
            root.appendChild(actionItem->action()->xmlDump(doc));
    }

    return doc;
}

void UserActionListView::removeSelectedActions()
{
    const QList<QTreeWidgetItem *> list = selectedItems();

    for (auto item : list) {
        if (auto *actionItem = dynamic_cast<UserActionListViewItem *>(item)) {
            delete actionItem->action(); // remove the action itself
            delete actionItem; // remove the action from the list
        } // if
    }
}

// UserActionListViewItem

UserActionListViewItem::UserActionListViewItem(QTreeWidget *view, KrAction *action)
    : QTreeWidgetItem(view)
{
    setAction(action);
}

UserActionListViewItem::UserActionListViewItem(QTreeWidgetItem *item, KrAction *action)
    : QTreeWidgetItem(item)
{
    setAction(action);
}

UserActionListViewItem::~UserActionListViewItem() = default;

void UserActionListViewItem::setAction(KrAction *action)
{
    if (!action)
        return;

    _action = action;
    update();
}

KrAction *UserActionListViewItem::action() const
{
    return _action;
}

void UserActionListViewItem::update()
{
    if (!_action)
        return;

    if (!_action->icon().isNull())
        setIcon(COL_TITLE, Icon(_action->iconName()));
    setText(COL_TITLE, _action->text());
}

bool UserActionListViewItem::operator<(const QTreeWidgetItem &other) const
{
    // FIXME some how this only produces bullshit :-/
    // if (i->text(COL_NAME).isEmpty()) { // categories only have titles
    //     // qDebug() << "this->title: " << text(COL_TITLE) << " |=|   i->title: " << i->text(COL_TITLE);
    //     return (ascending ? -1 : 1); // <0 means this is smaller then i
    // } else
    return QTreeWidgetItem::operator<(other);
}
