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

   void update( UserActionListViewItem* item = 0 );
   UserActionListViewItem* insertAction( KrAction* action );

   KrAction* currentAction() const;

   QDomDocument dumpSelectedActions( QDomDocument* mergeDoc = 0 ) const;

   void removeSelectedActions();

   /**
    * makes the first action in the list current
    */
   void setFirstActionCurrent();

   /**
    * makes @e item current and ensures its visibility
    */
   virtual void setCurrentItem( QListViewItem* item );

protected:
   QListViewItem* findCategoryItem( const QString& category );
};


/**
 * @author Jonas Bähr
 */
class UserActionListViewItem : public KListViewItem {
public:
   UserActionListViewItem( QListView* view, KrAction* action );
   UserActionListViewItem( QListViewItem* item, KrAction* action );
   ~UserActionListViewItem();

   void setAction( KrAction* action );
   KrAction* action() const;
   void update();

   /**
    * This reimplements qt's compare-function in order to have categories on the top of the list
    */
   int compare ( QListViewItem * i, int col, bool ascending ) const;

private:
   KrAction* _action;
};


#endif //USERACTIONLISTVIEW_H
