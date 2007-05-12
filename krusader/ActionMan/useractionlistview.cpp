//
// C++ Implementation: useractionlistview
//
// Description: 
//
//
// Author: Jonas Bähr, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "useractionlistview.h"

#include <klocale.h>
#include <kiconloader.h>
#include <q3ptrlist.h>
#include <qdom.h>

#include "../krusader.h"
#include "../UserAction/kraction.h"
#include "../UserAction/useraction.h"

#define COL_TITLE	0
#define COL_NAME	1


//////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////     UserActionListView    /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

UserActionListView::UserActionListView( QWidget * parent, const char * name )
 : K3ListView( parent, name )
{
   addColumn( i18n("Title") );
   //addColumn( i18n("Identifier") );
   setResizeMode( Q3ListView::AllColumns );

   setRootIsDecorated( true );
   setSelectionMode( Q3ListView::Extended ); // normaly select single items but one may use Ctrl or Shift to select multiple
   setSorting( COL_TITLE );

   update();
}

UserActionListView::~UserActionListView()
{
}

QSize UserActionListView::sizeHint() const {
   return QSize(200, 400);
}


void UserActionListView::update() {
   clear();
   UserAction::KrActionList list = krUserAction->actionList();
   for ( KrAction* action = list.first(); action; action = list.next() )
      insertAction( action );
   //sort(); // this is done automaticly
}

void UserActionListView::update( KrAction* action ) {
   UserActionListViewItem* item = findActionItem( action );
   if ( item ) {
      // deleting & re-inserting is _much_easyer then tracking all possible cases of category changes!
      bool current = ( item == currentItem() );
      bool selected = item->isSelected();
      delete item;
      item = insertAction( action );
      if ( current )
         setCurrentItem( item );
      if ( selected )
         setSelected( item, true );
   }
}

UserActionListViewItem* UserActionListView::insertAction( KrAction* action ) {
   if ( ! action )
      return 0;

   UserActionListViewItem* item;

   if ( action->category().isEmpty() )
      item = new UserActionListViewItem( this, action );
   else {
      Q3ListViewItem* categoryItem = findCategoryItem( action->category() );
      if ( ! categoryItem ) {
         categoryItem = new K3ListViewItem( this, action->category() ); // create the new category item it not already present
         categoryItem->setSelectable( false );
      }
      item = new UserActionListViewItem( categoryItem, action );
   }

   item->setAction( action );
   return item;
}

Q3ListViewItem* UserActionListView::findCategoryItem( const QString& category ) {
   for ( Q3ListViewItem* item = firstChild(); item; item = item->nextSibling() )
      if ( item->text( COL_TITLE ) == category && item->text( COL_NAME ).isEmpty() ) // because actions must have a name, items without name haveto be categories
         return item;

   return 0;
}

UserActionListViewItem* UserActionListView::findActionItem( const KrAction* action ) {
   for ( Q3ListViewItemIterator it( this ); it.current(); ++it ) {
      if ( UserActionListViewItem* item = dynamic_cast<UserActionListViewItem*>( it.current() ) ) {
         if ( item->action() == action )
            return item;
      }
   } //for
   return 0;
}

KrAction * UserActionListView::currentAction() const {
   if ( UserActionListViewItem* item = dynamic_cast<UserActionListViewItem*>( currentItem() ) )
      return item->action();
   else
      return 0;
}

void UserActionListView::setCurrentAction( const KrAction* action) {
   UserActionListViewItem* item = findActionItem( action );
   if ( item ) {
      setCurrentItem( item );
//       setSelected( item, true );
//       repaintItem( item );
   }
}

void UserActionListView::setFirstActionCurrent() {
  for ( Q3ListViewItemIterator it( this ); it.current(); ++it ) {
    if ( UserActionListViewItem* item = dynamic_cast<UserActionListViewItem*>( it.current() ) ) {
      setCurrentItem( item );
      break;
    }
  } //for
}

void UserActionListView::setCurrentItem( Q3ListViewItem* item ) {
   if ( ! item )
      return;
   ensureItemVisible( item );
   Q3ListView::setCurrentItem( item );
}

QDomDocument UserActionListView::dumpSelectedActions( QDomDocument* mergeDoc ) const {
   Q3PtrList<Q3ListViewItem> list = selectedItems();
   QDomDocument doc;
   if ( mergeDoc )
      doc = *mergeDoc;
   else
      doc = UserAction::createEmptyDoc();
   QDomElement root = doc.documentElement();

   for ( Q3ListViewItem* item = list.first(); item; item = list.next() )
      if ( UserActionListViewItem* actionItem = dynamic_cast<UserActionListViewItem*>( item ) )
         root.appendChild( actionItem->action()->xmlDump( doc ) );

   return doc;
}

void UserActionListView::removeSelectedActions() {
   Q3PtrList<Q3ListViewItem> list = selectedItems();

   for ( Q3ListViewItem* item = list.first(); item; item = list.next() )
      if ( UserActionListViewItem* actionItem = dynamic_cast<UserActionListViewItem*>( item ) ) {
         delete actionItem->action(); // remove the action itself
         delete actionItem; // remove the action from the list
      } // if

}

//////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////     UserActionListViewItem    ////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

UserActionListViewItem::UserActionListViewItem( Q3ListView* view, KrAction* action )
 : K3ListViewItem( view )
{
   setAction( action );
}

UserActionListViewItem::UserActionListViewItem( Q3ListViewItem* item, KrAction * action )
 : K3ListViewItem( item )
{
   setAction( action );
}

UserActionListViewItem::~UserActionListViewItem() {
/*   // remove category-item if the last member ofthiscategory disappears
   if ( QListViewItem* item = dynamic_cast<QListViewItem*>( parent() ) ) {
      if ( item->childCount() <= 1 )
         item->deleteLater(); // not possible since not inherited from QObject
   }*/
}


void UserActionListViewItem::setAction( KrAction * action ) {
   if ( ! action )
      return;

   _action = action;
   update();
}

KrAction * UserActionListViewItem::action() const {
   return _action;
}

void UserActionListViewItem::update() {
   if ( ! _action )
      return;

   if ( ! _action->icon().isEmpty() )
      setPixmap( COL_TITLE, KIconLoader::global()->loadIcon( _action->icon(), KIcon::Small ) );
   setText( COL_TITLE, _action->text() );
   setText( COL_NAME, _action->name() );
}

int UserActionListViewItem::compare( Q3ListViewItem* i, int col, bool ascending ) const {
// FIXME some how this only produces bullshit :-/
//   if ( i->text( COL_NAME ).isEmpty() ) { // categories only have titles
//      //kDebug() << "this->title: " << text(COL_TITLE) << " |=|   i->title: " << i->text(COL_TITLE)  << endl;
//       return ( ascending ? -1 : 1 ); // <0 means this is smaller then i
//    }
//    else
      return Q3ListViewItem::compare( i, col, ascending );
}


