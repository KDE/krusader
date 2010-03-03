/*****************************************************************************
 * Copyright (C) 2006 Jonas Bähr <jonas.baehr@web.de>                        *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#include "useractionlistview.h"

#include <klocale.h>
#include <kiconloader.h>
#include <qdom.h>

#include "../krglobal.h"
#include "../UserAction/kraction.h"
#include "../UserAction/useraction.h"

#define COL_TITLE 0

// UserActionListView

UserActionListView::UserActionListView(QWidget * parent)
        : KrTreeWidget(parent)
{
    setHeaderLabel(i18n("Title"));

    setRootIsDecorated(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);   // normaly select single items but one may use Ctrl or Shift to select multiple

    setSortingEnabled(true);
    sortItems(COL_TITLE, Qt::AscendingOrder);

    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), SLOT(slotCurrentItemChanged(QTreeWidgetItem*)));

    update();
}

UserActionListView::~UserActionListView()
{
}

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

void UserActionListView::update(KrAction* action)
{
    UserActionListViewItem* item = findActionItem(action);
    if (item) {
        // deleting & re-inserting is _much_easyer then tracking all possible cases of category changes!
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

UserActionListViewItem* UserActionListView::insertAction(KrAction* action)
{
    if (! action)
        return 0;

    UserActionListViewItem* item;

    if (action->category().isEmpty())
        item = new UserActionListViewItem(this, action);
    else {
        QTreeWidgetItem* categoryItem = findCategoryItem(action->category());
        if (! categoryItem) {
            categoryItem = new QTreeWidgetItem(this);   // create the new category item it not already present
            categoryItem->setText(0, action->category());
            categoryItem->setFlags(Qt::ItemIsEnabled);
        }
        item = new UserActionListViewItem(categoryItem, action);
    }

    item->setAction(action);
    return item;
}

QTreeWidgetItem* UserActionListView::findCategoryItem(const QString& category)
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        if ((*it)->text(COL_TITLE) == category && !((*it)->flags() & Qt::ItemIsSelectable))
            return *it;
        it++;
    }
    return 0;
}

UserActionListViewItem* UserActionListView::findActionItem(const KrAction* action)
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        if (UserActionListViewItem* item = dynamic_cast<UserActionListViewItem*>(*it)) {
            if (item->action() == action)
                return item;
        }
        it++;
    }
    return 0;
}

KrAction * UserActionListView::currentAction() const
{
    if (UserActionListViewItem* item = dynamic_cast<UserActionListViewItem*>(currentItem()))
        return item->action();
    else
        return 0;
}

void UserActionListView::setCurrentAction(const KrAction* action)
{
    UserActionListViewItem* item = findActionItem(action);
    if (item) {
        setCurrentItem(item);
    }
}

void UserActionListView::setFirstActionCurrent()
{
    QTreeWidgetItemIterator it(this);
    while (*it) {
        if (UserActionListViewItem* item = dynamic_cast<UserActionListViewItem*>(*it)) {
            setCurrentItem(item);
            break;
        }
        it++;
    }
}

void UserActionListView::slotCurrentItemChanged(QTreeWidgetItem* item)
{
    if (! item)
        return;
    scrollTo(indexOf(item));
}

QDomDocument UserActionListView::dumpSelectedActions(QDomDocument* mergeDoc) const
{
    QList<QTreeWidgetItem*> list = selectedItems();
    QDomDocument doc;
    if (mergeDoc)
        doc = *mergeDoc;
    else
        doc = UserAction::createEmptyDoc();
    QDomElement root = doc.documentElement();

    for (int i = 0; i < list.size(); ++i) {
        QTreeWidgetItem* item = list.at(i);
        if (UserActionListViewItem* actionItem = dynamic_cast<UserActionListViewItem*>(item))
            root.appendChild(actionItem->action()->xmlDump(doc));
    }

    return doc;
}

void UserActionListView::removeSelectedActions()
{
    QList<QTreeWidgetItem*> list = selectedItems();

    for (int i = 0; i < list.size(); ++i) {
        QTreeWidgetItem* item = list.at(i);
        if (UserActionListViewItem* actionItem = dynamic_cast<UserActionListViewItem*>(item)) {
            delete actionItem->action(); // remove the action itself
            delete actionItem; // remove the action from the list
        } // if
    }
}

// UserActionListViewItem

UserActionListViewItem::UserActionListViewItem(QTreeWidget* view, KrAction* action)
        : QTreeWidgetItem(view)
{
    setAction(action);
}

UserActionListViewItem::UserActionListViewItem(QTreeWidgetItem* item, KrAction * action)
        : QTreeWidgetItem(item)
{
    setAction(action);
}

UserActionListViewItem::~UserActionListViewItem()
{
}

void UserActionListViewItem::setAction(KrAction * action)
{
    if (! action)
        return;

    _action = action;
    update();
}

KrAction * UserActionListViewItem::action() const
{
    return _action;
}

void UserActionListViewItem::update()
{
    if (! _action)
        return;

    if (! _action->icon().isNull())
        setIcon(COL_TITLE, KIconLoader::global()->loadIcon(_action->iconName(), KIconLoader::Small));
    setText(COL_TITLE, _action->text());
}

bool UserActionListViewItem::operator<(const QTreeWidgetItem &other) const
{
// FIXME some how this only produces bullshit :-/
//   if ( i->text( COL_NAME ).isEmpty() ) { // categories only have titles
//      //kDebug() << "this->title: " << text(COL_TITLE) << " |=|   i->title: " << i->text(COL_TITLE)  << endl;
//       return ( ascending ? -1 : 1 ); // <0 means this is smaller then i
//    }
//    else
    return QTreeWidgetItem::operator<(other);
}
