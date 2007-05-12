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

#include <klistview.h>

class KrAction;
class QString;
class UserActionListViewItem;
class QDomDocument;

/**
 * @author Jonas Bähr
 */
class UserActionListView : public KListView {
public:
   UserActionListView( QWidget* parent = 0, const char* name = 0 );
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
   virtual void setCurrentItem( Q3ListViewItem* item );

protected:
   Q3ListViewItem* findCategoryItem( const QString& category );
   UserActionListViewItem* findActionItem( const KrAction* action );
};


/**
 * @author Jonas Bähr
 */
class UserActionListViewItem : public KListViewItem {
public:
   UserActionListViewItem( Q3ListView* view, KrAction* action );
   UserActionListViewItem( Q3ListViewItem* item, KrAction* action );
   ~UserActionListViewItem();

   void setAction( KrAction* action );
   KrAction* action() const;
   void update();

   /**
    * This reimplements qt's compare-function in order to have categories on the top of the list
    */
   int compare ( Q3ListViewItem * i, int col, bool ascending ) const;

private:
   KrAction* _action;
};


#endif //USERACTIONLISTVIEW_H
