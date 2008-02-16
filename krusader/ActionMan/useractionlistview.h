//
// C++ Interface: useractionlistview
//
// Description: 
//
//
// Author: Jonas Bähr, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef USERACTIONLISTVIEW_H
#define USERACTIONLISTVIEW_H

#include "../GUI/krtreewidget.h"

class KrAction;
class QString;
class UserActionListViewItem;
class QDomDocument;

/**
 * @author Jonas Bähr
 */
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


/**
 * @author Jonas Bähr
 */
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


#endif //USERACTIONLISTVIEW_H
