#include <qwidgetstack.h>
#include <qtoolbutton.h>
#include <klocale.h>
#include <qimage.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kiconloader.h>
#include "panelmanager.h"
#include "Panel/listpanel.h"
#include "krusaderview.h"

#define HIDE_ON_SINGLE_TAB  false

#define SHOW  { _newTab->show(); _tabbar->show(); _closeTab->show(); }
#define HIDE  { _newTab->hide(); _tabbar->hide(); _closeTab->hide(); }

PanelManager::PanelManager( QWidget *parent, bool left, ListPanel* &self, ListPanel* &other, ListPanel* &active ) :
QWidget( parent ), _layout( 0 ), _left( left ), _self( self ), _other( other ), _active( active ) {
   _layout = new QGridLayout( this, 1, 1 );
   _stack = new QWidgetStack( this );

   // new tab button
   _newTab = new QToolButton( this );
   _newTab->setFixedSize( 22, 22 );
   _newTab->setTextLabel( i18n( "Open a new tab in home" ) );
   QImage im = krLoader->loadIcon( "favorites", KIcon::Panel ).convertToImage();
   _newTab->setPixmap( im.scale( _newTab->height() - 5, _newTab->height() - 5 ) );
   connect( _newTab, SIGNAL( clicked() ), this, SLOT( slotNewTab() ) );

   // close tab button
   _closeTab = new QToolButton( this );
   _closeTab->setFixedSize( 22, 22 );
   _closeTab->setTextLabel( i18n( "Close current tab" ) );
   im = krLoader->loadIcon( "cancel", KIcon::Panel ).convertToImage();
   _closeTab->setPixmap( im.scale( _closeTab->height() - 5, _closeTab->height() - 5 ) );
   connect( _closeTab, SIGNAL( clicked() ), this, SLOT( slotCloseTab() ) );
   _closeTab->setEnabled( false ); // disabled when there's only 1 tab

   // tab-bar
   _tabbar = new PanelTabBar( this );
   connect( _tabbar, SIGNAL( changePanel( ListPanel* ) ), this, SLOT( slotChangePanel( ListPanel * ) ) );
   connect( _tabbar, SIGNAL( closeCurrentTab() ), this, SLOT( slotCloseTab() ) );
   connect( _tabbar, SIGNAL( newTab( QString ) ), this, SLOT( slotNewTab( QString ) ) );

   _layout->addMultiCellWidget( _stack, 0, 0, 0, 2 );
   _layout->addWidget( _newTab, 1, 0 );
   _layout->addWidget( _tabbar, 1, 1 );
   _layout->addWidget( _closeTab, 1, 2 );

   if ( HIDE_ON_SINGLE_TAB ) HIDE
      else SHOW
      }

void PanelManager::slotChangePanel( ListPanel *p ) {
   _self = p;
   _self->otherPanel = _other;
   _other->otherPanel = _self;

   _stack->raiseWidget( _self );
   kapp->processEvents();
   _self->slotFocusOnMe();
}

ListPanel* PanelManager::createPanel() {
   // create the panel and add it into the widgetstack
   ListPanel * p = new ListPanel( _stack, _left );
   _stack->addWidget( p );

   // now, create the corrosponding tab
   _tabbar->addPanel( p );

   // allow close button if more than 1 tab
   if ( _tabbar->count() > 1 ) {
      _closeTab->setEnabled( true );
      SHOW // needed if we were hidden
   }
   _stack->raiseWidget( p );

   // connect the activePanelChanged signal to enable/disable actions
   connect( p, SIGNAL( activePanelChanged( ListPanel* ) ), this, SLOT( slotRefreshActions() ) );
   return p;
}

void PanelManager::startPanel( ListPanel *panel, QString path ) {
   panel->start( path );
}

void PanelManager::saveSettings( KConfig *config, const QString& key ) {
   QStringList l;
   int i=0, cnt=0;
   while (cnt < _tabbar->count()) {
      PanelTab *t = dynamic_cast<PanelTab*>(_tabbar->tabAt(i));
      if (t && t->panel) {
         l << t->panel->realPath;
         ++cnt;
      }
      ++i;
   }
   config->writePathEntry( key, l );
}

void PanelManager::loadSettings( KConfig *config, const QString& key ) {
   QStringList l = config->readPathListEntry( key );
   if( l.count() < 1 )
     return;
     
   int i=0, totalTabs = _tabbar->count();
   
   while (i < totalTabs && i < (int)l.count() ) 
   {
      PanelTab *t = dynamic_cast<PanelTab*>(_tabbar->tabAt(i));
      if (t && t->panel) 
      {
         _tabbar->setCurrentTab( t );
         slotChangePanel( t->panel );
         t->panel->start( l[ i ] );
      }
      ++i;
   }
   
   while( i <  totalTabs )
   {
      PanelTab *t = dynamic_cast<PanelTab*>(_tabbar->tabAt( --totalTabs ));
      if (t && t->panel) 
      {
        _tabbar->setCurrentTab( _tabbar->tabAt( totalTabs ) );
        slotChangePanel( t->panel );
        slotCloseTab();
      }
   }
   
   for(; i < (int)l.count(); i++ )
   {
     slotNewTab( l[i] );
   }
}

void PanelManager::slotNewTab() {
   slotNewTab( QDir::home().absPath() );
}

void PanelManager::slotNewTab( QString path ) {
   _self = createPanel();
   // update left/right pointers
   _self->otherPanel = _other;
   _other->otherPanel = _self;

   startPanel( _self, path );
}

void PanelManager::slotCloseTab() {
   if ( _tabbar->count() <= 1 )  /* if this is the last tab don't close it */
      return ;

   // setup current one
   ListPanel * oldp;
   _self = _tabbar->removeCurrentPanel( oldp );
   _stack->raiseWidget( _self );
   _stack->removeWidget( oldp );
   delete oldp;

   // setup pointers
   _self->otherPanel = _other;
   _other->otherPanel = _self;
   _self->slotFocusOnMe();

   // disable close button if only 1 tab is left
   if ( _tabbar->count() == 1 ) {
      _closeTab->setEnabled( false );
      if ( HIDE_ON_SINGLE_TAB ) HIDE
      }
}

void PanelManager::slotRefreshActions() {
   krCloseTab->setEnabled( _tabbar->count() > 1 );
   krNextTab->setEnabled(_tabbar->count() > 1);
   krPreviousTab->setEnabled(_tabbar->count() > 1);	
}

int PanelManager::activeTab()
{
  return _tabbar->indexOf( _tabbar->currentTab() );
}

void PanelManager::setActiveTab( int panelIndex )
{
  QTab *current = _tabbar->tabAt( panelIndex );
  if( current == 0 )
    return;  
  _tabbar->setCurrentTab( current );
  slotChangePanel ( dynamic_cast<PanelTab*>( _tabbar->tabAt( panelIndex ) )->panel );
}

void PanelManager::recreatePanels() {
   int panelCount = _tabbar->count(), identifier = 0;
   ListPanel *oldCurrent = _self, *newCurrent = 0;

   while ( panelCount -- ) {
      ListPanel * panel = dynamic_cast<PanelTab*>( _tabbar->tabAt( 0 ) ) ->panel;
      slotNewTab( panel->virtualPath );

      if ( panel == oldCurrent )
         newCurrent = _self, identifier = _tabbar->currentTab();

      _tabbar->setCurrentTab( _tabbar->tabAt( 0 ) );
      slotChangePanel( panel );

      slotCloseTab();
   }

   if ( newCurrent ) {
      _tabbar->setCurrentTab( identifier );
      slotChangePanel( newCurrent );
   }
}

void PanelManager::slotNextTab() {
   int currTab = _tabbar->currentTab();
	int nextInd = (_tabbar->indexOf(currTab) == _tabbar->count()-1 ? 0 : _tabbar->indexOf(currTab)+1);
	ListPanel *nextp = dynamic_cast<PanelTab*>(_tabbar->tabAt(nextInd))->panel;
	_tabbar->setCurrentTab(_tabbar->tabAt(nextInd));	
	slotChangePanel(nextp);   
}


void PanelManager::slotPreviousTab() {
   int currTab = _tabbar->currentTab();
	int nextInd = (_tabbar->indexOf(currTab) == 0 ? _tabbar->count()-1 : _tabbar->indexOf(currTab)-1);
	ListPanel *nextp = dynamic_cast<PanelTab*>(_tabbar->tabAt(nextInd))->panel;
	_tabbar->setCurrentTab(_tabbar->tabAt(nextInd));	
	slotChangePanel(nextp);   
}
