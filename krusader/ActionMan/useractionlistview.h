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

#ifndef USERACTIONLISTVIEW_H
#define USERACTIONLISTVIEW_H

#include "../GUI/krtreewidget.h"

class KrAction;
class QString;
class UserActionListViewItem;
class QDomDocument;

class UserActionListView : public KrTreeWidget {
   Q_OBJECT

public:
   UserActionListView( QWidget* parent = 0 );
   ~UserActionListView();
   virtual QSize sizeHint() const;

   void update();
   void update( KrAction* action );
   UserActionListViewItem* insertAction( KrAction* action );

   KrAction* currentAction() const;
   void setCurrentAction( const KrAction* );

   QDomDocument dumpSelectedActions( QDomDocument* mergeDoc = 0 ) const;

   void removeSelectedActions();

   /**
    * makes the first action in the list current
    */
   void setFirstActionCurrent();

   /**
    * makes @e item current and ensures its visibility
    */
protected slots:
   void slotCurrentItemChanged( QTreeWidgetItem* );

protected:
   QTreeWidgetItem* findCategoryItem( const QString& category );
   UserActionListViewItem* findActionItem( const KrAction* action );
};

class UserActionListViewItem : public QTreeWidgetItem {
public:
   UserActionListViewItem( QTreeWidget* view, KrAction* action );
   UserActionListViewItem( QTreeWidgetItem* item, KrAction* action );
   ~UserActionListViewItem();

   void setAction( KrAction* action );
   KrAction* action() const;
   void update();

   /**
    * This reimplements qt's compare-function in order to have categories on the top of the list
    */
   virtual bool operator<(const QTreeWidgetItem &other) const;

private:
   KrAction* _action;
};

#endif
