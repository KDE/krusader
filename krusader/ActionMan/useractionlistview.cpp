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
#include <qptrlist.h>
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
 : KListView( parent, name )
{
   addColumn( i18n("Title") );
   //addColumn( i18n("Identifier") );
   setResizeMode( QListView::AllColumns );

   setRootIsDecorated( true );
   setSelectionMode( QListView::Extended ); // normaly select single items but one may use Ctrl or Shift to select multiple
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
      // This is _much_easyer then tracking all possible cases of category changes!
      delete item;
      insertAction( action );
   }
}

UserActionListViewItem* UserActionListView::insertAction( KrAction* action ) {
   if ( ! action )
      return 0;

   UserActionListViewItem* item;

   if ( action->category().isEmpty() )
      item = new UserActionListViewItem( this, action );
   else {
      QListViewItem* categoryItem = findCategoryItem( action->category() );
      if ( ! categoryItem ) {
         categoryItem = new KListViewItem( this, action->category() ); // create the new category item it not already present
         categoryItem->setSelectable( false );
      }
      item = new UserActionListViewItem( categoryItem, action );
   }

   item->setAction( action );
   return item;
}

QListViewItem* UserActionListView::findCategoryItem( const QString& category ) {
   for ( QListViewItem* item = firstChild(); item; item = item->nextSibling() )
      if ( item->text( COL_TITLE ) == category && item->text( COL_NAME ).isEmpty() ) // because actions must have a name, items without name haveto be categories
         return item;

   return 0;
}

UserActionListViewItem* UserActionListView::findActionItem( const KrAction* action ) {
   for ( QListViewItemIterator it( this ); it.current(); ++it ) {
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
  for ( QListViewItemIterator it( this ); it.current(); ++it ) {
    if ( UserActionListViewItem* item = dynamic_cast<UserActionListViewItem*>( it.current() ) ) {
      setCurrentItem( item );
      break;
    }
  } //for
}

void UserActionListView::setCurrentItem( QListViewItem* item ) {
   if ( ! item )
      return;
   ensureItemVisible( item );
   QListView::setCurrentItem( item );
}

QDomDocument UserActionListView::dumpSelectedActions( QDomDocument* mergeDoc ) const {
   QPtrList<QListViewItem> list = selectedItems();
   QDomDocument doc;
   if ( mergeDoc )
      doc = *mergeDoc;
   else
      doc = UserAction::createEmptyDoc();
   QDomElement root = doc.documentElement();

   for ( QListViewItem* item = list.first(); item; item = list.next() )
      if ( UserActionListViewItem* actionItem = dynamic_cast<UserActionListViewItem*>( item ) )
         root.appendChild( actionItem->action()->xmlDump( doc ) );

   return doc;
}

void UserActionListView::removeSelectedActions() {
   QPtrList<QListViewItem> list = selectedItems();

   for ( QListViewItem* item = list.first(); item; item = list.next() )
      if ( UserActionListViewItem* actionItem = dynamic_cast<UserActionListViewItem*>( item ) ) {
         delete actionItem->action(); // remove the action itself
         delete actionItem; // remove the action from the list
      } // if

}

//////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////     UserActionListViewItem    ////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

UserActionListViewItem::UserActionListViewItem( QListView* view, KrAction* action )
 : KListViewItem( view )
{
   setAction( action );
}

UserActionListViewItem::UserActionListViewItem( QListViewItem* item, KrAction * action )
 : KListViewItem( item )
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
      setPixmap( COL_TITLE, KGlobal::iconLoader()->loadIcon( _action->icon(), KIcon::Small ) );
   setText( COL_TITLE, _action->text() );
   setText( COL_NAME, _action->name() );
}

int UserActionListViewItem::compare( QListViewItem* i, int col, bool ascending ) const {
// FIXME some how this only produces bullshit :-/
//   if ( i->text( COL_NAME ).isEmpty() ) { // categories only have titles
//      //kdDebug() << "this->title: " << text(COL_TITLE) << " |=|   i->title: " << i->text(COL_TITLE)  << endl;
//       return ( ascending ? -1 : 1 ); // <0 means this is smaller then i
//    }
//    else
      return QListViewItem::compare( i, col, ascending );
}


