/***************************************************************************
                               krusaderview.cpp
                            -------------------
   copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
   e-mail               : krusader@users.sourceforge.net
   web site             : http://krusader.sourceforge.net
---------------------------------------------------------------------------
 Description
***************************************************************************

 A

    db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
    88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
    88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
    88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
    88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
    YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                    S o u r c e    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

// Qt includes
#include <q3whatsthis.h> 
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3ValueList>
#include <QKeyEvent>
#include <QEvent>
#include <kstatusbar.h>
#include <kmenubar.h>
#include <kshortcut.h>
#include <ktoolbar.h>
#include <ktoggleaction.h>
// Krusader includes
#include "krusaderview.h"
#include "krusader.h"
#include "krslots.h"
#include "defaults.h"
#include "Panel/listpanel.h"
#include "Panel/panelfunc.h"
#include "GUI/kcmdline.h"
#include "GUI/kfnkeys.h"
#include "resources.h"
#include "panelmanager.h"
#include <klibloader.h> //<>
#include "GUI/profilemanager.h"
#include "Dialogs/percentalsplitter.h"
#include "krservices.h"
#include <qclipboard.h>

KrusaderView::KrusaderView( QWidget *parent ) : QWidget( parent, "KrusaderView" ), activePanel(0), 
								konsole_part( 0L ) {}

void KrusaderView::start( QStringList leftTabs, QStringList leftTypes, int leftActiveTab, QStringList rightTabs, QStringList rightTypes, int rightActiveTab ) {
  ////////////////////////////////
  // make a 1x1 mainLayout, it will auto-expand:
  mainLayout = new Q3GridLayout( this, 1, 1 );
  // vertical splitter
  vert_splitter = new QSplitter( this ); // splits between panels and terminal/cmdline
  vert_splitter->setOrientation( Qt::Vertical );
  // horizontal splitter
  horiz_splitter = new PercentalSplitter( vert_splitter );
  ( terminal_dock = new Q3HBox( vert_splitter ) ) ->hide(); // create it hidden
  // create a command line thing
  cmdLine = new KCMDLine( this );

  // add a panel manager for each side of the splitter
  leftMng  = new PanelManager(horiz_splitter, true );
  rightMng = new PanelManager(horiz_splitter, false );

  // now, create the panels inside the manager
  //left = new ListPanel( leftMng, true );
  //right = new ListPanel( rightMng, false );
  left = leftMng->createPanel( leftTypes[ 0 ] );
  right = rightMng->createPanel( rightTypes[ 0 ] );

  left->setOther( right );
  right->setOther( left );

  // create the function keys widget
  fnKeys = new KFnKeys( this );
  fnKeys->hide();
  Q3WhatsThis::add
    ( fnKeys, i18n( "Function keys allow performing fast "
                    "operations on files." ) );

  // and insert the whole thing into the main layout... at last
  mainLayout->addWidget( vert_splitter, 0, 0 );  //<>
  mainLayout->addWidget( cmdLine, 1, 0 );
  mainLayout->addWidget( fnKeys, 2, 0 );
  mainLayout->activate();

  // get the last saved sizes of the splitter
  krConfig->setGroup( "Private" );
  Q3ValueList<int> lst = krConfig->readIntListEntry( "Splitter Sizes" );
  if ( lst.isEmpty() )
  {
    lst = horiz_splitter->sizes();
    int avg = (lst[ 0 ] + lst[ 1 ] )/2;
    lst[ 0 ] = lst[ 1 ] = avg;
  }
  horiz_splitter->setSizes( lst );  

  verticalSplitterSizes = krConfig->readIntListEntry( "Terminal Emulator Splitter Sizes" );
    
  show();

  qApp->processEvents();
  
  // make the left panel focused at program start
  rightMng->startPanel( right, rightTabs[ 0 ] );
  leftMng->startPanel( left, leftTabs[ 0 ] );
  activePanel = left;
  activePanel->slotFocusOnMe();  // left starts out active
     
  for(unsigned int i = 1; i < leftTabs.count(); i++ )
    leftMng->slotNewTab( leftTabs[ i ], false, leftTypes[ i ] );

  for(unsigned int j = 1; j < rightTabs.count(); j++ )
    rightMng->slotNewTab( rightTabs[ j ], false, rightTypes[ j ] );
       
  leftMng->setActiveTab( leftActiveTab );
  rightMng->setActiveTab( rightActiveTab );
}

// updates the command line whenever current panel changes
//////////////////////////////////////////////////////////
void KrusaderView::slotCurrentChanged( QString p ) {
  cmdLine->setCurrent( p );
  if ( konsole_part != 0L && konsole_part->widget() != 0L ) {
	 KConfigGroup cfg = krConfig->group("General");
    if (cfg.readEntry("Send CDs", _SendCDs)) // hopefully, this is cached in kconfig
        if( !konsole_part->url().equals( KUrl( p ), KUrl::CompareWithoutTrailingSlash ) )
           konsole_part->openUrl( KUrl( p ) );
  }
}

void KrusaderView::cmdLineFocus() {  // command line receive's keyboard focus
    cmdLine->setFocus();
}

void KrusaderView::cmdLineUnFocus() { // return focus to the active panel
  activePanel->slotFocusOnMe();
}

/** if the KonsolePart for the Terminal Emulator is not yet loaded, load it */
void KrusaderView::createTE() {
  if ( konsole_part == 0L ) {  // konsole part is not yet loaded
    KLibFactory * factory = KLibLoader::self() ->factory( "libkonsolepart" );
    if ( factory ) {
      QWidget *focusW = qApp->focusWidget();
      // Create the part
      konsole_part = ( KParts::ReadOnlyPart * )
                          factory->create( (QObject*)terminal_dock, /*"konsolepart",*/
                                           "KParts::ReadOnlyPart" );
      if( konsole_part ) { //loaded successfully
        connect( konsole_part, SIGNAL( destroyed() ),
                 this, SLOT( killTerminalEmulator() ) );
        qApp->installEventFilter( this );
      } else {
        qApp->removeEventFilter( this );
      }
      /*the Terminal Emulator may be hidden (if we are creating it only
        to send command there and see the results later */
      if( focusW ) {
        focusW->setFocus();
      }
      else {
        activePanel->slotFocusOnMe();    
      }
    } else
      konsole_part = 0L;
  }
}

// Tab - switch focus
void KrusaderView::panelSwitch() { activePanel->otherPanel->slotFocusOnMe(); }
void KrusaderView::slotSetActivePanel( ListPanel *p ) { activePanel = p; }

void KrusaderView::slotTerminalEmulator( bool show ) {
  KConfigGroup cfg = krConfig->group("Look&Feel");
  bool fullscreen = cfg.readEntry("Fullscreen Terminal Emulator", false);
  static bool fnKeysShown=true; // first time init. should be overridden
  static bool cmdLineShown=true;
  static bool statusBarShown=true;
  static bool toolBarShown=true;
  static bool menuBarShown=true;

  if ( !show ) {  // hiding the terminal
    activePanel->slotFocusOnMe();
    if( terminal_dock->isVisible() && !fullscreen )
      verticalSplitterSizes = vert_splitter->sizes();

    // BUGFIX: when the terminal emulator is toggled on, first it is shown in minimum size
    //         then QSplitter resizes it to the desired size.
    //         this minimum resize scrolls up the content of the konsole widget
    // SOLUTION:
    //         we hide the console widget while the resize ceremony happens, then reenable it
    if( konsole_part )
      konsole_part->widget()->hide(); // hide the widget to prevent from resize

    terminal_dock->hide();
    Q3ValueList<int> newSizes;
    newSizes.push_back( vert_splitter->height() );
    newSizes.push_back( 0 );
    vert_splitter->setSizes( newSizes );
    // in full screen, we unhide everything that was hidden before
    if (fullscreen) {
	leftMng->show(); 
	rightMng->show();
	if (fnKeysShown) fnKeys->show();
	if (cmdLineShown) cmdLine->show();
	if (statusBarShown) krApp->statusBar()->show();
	if (toolBarShown) {
		krApp->toolBar()->show();
		krApp->toolBar("actionsToolBar")->show();
	}
	if (menuBarShown) krApp->menuBar()->show();
    }
    return ;
  }
  // else implied
  createTE();
  if ( konsole_part ) {      // if we succeeded in creating the konsole
    if( !verticalSplitterSizes.empty() )
      vert_splitter->setSizes( verticalSplitterSizes );
      
    terminal_dock->show();
    slotCurrentChanged( activePanel->realPath() );

    // BUGFIX: TE scrolling bug (see upper)
    //         show the Konsole part delayed
    QTimer::singleShot( 0, konsole_part->widget(), SLOT( show() ) );

    if( konsole_part->widget() ) {
      konsole_part->widget()->setFocus();

    }
    krToggleTerminal->setChecked( true );
    // in full screen mode, we hide everything else, but first, see what was actually shown
    if (fullscreen) {
	fnKeysShown = !fnKeys->isHidden();
    	cmdLineShown = !cmdLine->isHidden();
    	statusBarShown = !krApp->statusBar()->isHidden();
    	toolBarShown = !krApp->toolBar()->isHidden();
	menuBarShown = !krApp->menuBar()->isHidden();
    	leftMng->hide(); 
    	rightMng->hide();
    	fnKeys->hide();
    	cmdLine->hide();
    	krApp->statusBar()->hide();
    	krApp->toolBar()->hide();
    	krApp->toolBar("actionsToolBar")->hide();
	krApp->menuBar()->hide();
    }
  } else {
    terminal_dock->hide();
    krToggleTerminal->setChecked( false );
  }
}

void KrusaderView::focusTerminalEmulator()
{
  if ( MAIN_VIEW->terminal_dock->isVisible() && MAIN_VIEW->konsole_part && MAIN_VIEW->konsole_part->widget() )
    MAIN_VIEW->konsole_part->widget()->setFocus();
}

void KrusaderView::switchFullScreenTE()
{
  if( terminal_dock->isVisible() && konsole_part && konsole_part->widget() && konsole_part->widget()->isVisible() ) {
    KConfigGroup grp = krConfig->group("Look&Feel");
    bool fullscreen=grp.readEntry("Fullscreen Terminal Emulator", false);
    slotTerminalEmulator( false );
    grp.writeEntry("Fullscreen Terminal Emulator", !fullscreen);
    slotTerminalEmulator( true );
  }
}


bool KrusaderView::eventFilter ( QObject * watched, QEvent * e ) {
  if( e->type() == QEvent::AccelOverride && konsole_part && konsole_part->widget() == watched ) {
    QKeyEvent *ke = (QKeyEvent *)e;
    if( ( ke->key() ==  Qt::Key_Insert ) && ( ke->modifiers()  == Qt::ShiftModifier ) ) {
      ke->accept();
      return true;
    }
  }
  else if( e->type() == QEvent::KeyPress && konsole_part && konsole_part->widget() == watched ) {
    QKeyEvent *ke = (QKeyEvent *)e;
    int pressedKey = (ke->key() | ke->modifiers());

    if( Krusader::actToggleTerminal->shortcut().contains( pressedKey ) ) {
        Krusader::actToggleTerminal->activate(QAction::Trigger);
        return true;
    }

    if( Krusader::actSwitchFullScreenTE->shortcut().contains( pressedKey ) ) {
        Krusader::actSwitchFullScreenTE->activate(QAction::Trigger);
        return true;
    }

    if( ( ke->key() == Qt::Key_Enter || ke->key() == Qt::Key_Return ) && ( ( ke->modifiers() & ~Qt::ShiftModifier ) == Qt::ControlModifier ) ) {

      QString filename = ACTIVE_PANEL->view->getCurrentItem();
      if( filename == QString() || filename == ".." )
        return true;
      if( ke->modifiers() & Qt::ShiftModifier ) {
        QString path=vfs::pathOrUrl( ACTIVE_FUNC->files()->vfs_getOrigin(), KUrl::AddTrailingSlash );
        filename = path+filename;
      }

      filename = KrServices::quote( filename );

      QKeyEvent keyEvent( QEvent::KeyPress, 0, -1, 0, QString( " " ) + filename + QString( " " ));
      QApplication::sendEvent( konsole_part->widget(), &keyEvent );
      return true;
    } else if( ( ke->key() ==  Qt::Key_Down ) && ( ke->modifiers() == Qt::ControlModifier ) ) {
      if( cmdLine->isVisible() )
        cmdLine->setFocus();
      return true;
    } else if( ( ( ke->key() ==  Qt::Key_Up ) && ( ke->modifiers()  == Qt::ControlModifier ) ) || 
               ( ke->modifiers()  == ( Qt::ControlModifier | Qt::ShiftModifier ) ) ) {
      ACTIVE_PANEL->slotFocusOnMe();
      return true;
    } else if( Krusader::actPaste->shortcut().contains( pressedKey ) ) {
      QString text = QApplication::clipboard()->text();
      if ( ! text.isEmpty() )
      {
        text.replace("\n", "\r");
        QKeyEvent keyEvent(QEvent::KeyPress, 0,-1,0, text);
        QApplication::sendEvent( konsole_part->widget(), &keyEvent );
      }
      return true;
    }
  }
  return false;
}

QList<int> KrusaderView::getTerminalEmulatorSplitterSizes() {
  if( terminal_dock->isVisible() )
    return vert_splitter->sizes();
  else
    return verticalSplitterSizes;
}

void KrusaderView::killTerminalEmulator() {
  konsole_part = 0L;
  slotTerminalEmulator( false );  // hide the docking widget
  krToggleTerminal->setChecked( false );
}


void KrusaderView::profiles( QString profileName )
{
  ProfileManager profileManager( "Panel" );
  profileManager.hide();
  connect( &profileManager, SIGNAL( saveToProfile( QString ) ), this, SLOT( savePanelProfiles( QString ) ) );
  connect( &profileManager, SIGNAL( loadFromProfile( QString ) ), this, SLOT( loadPanelProfiles( QString ) ) );
  if( profileName.isEmpty() )
    profileManager.profilePopup();
  else
    profileManager.loadProfile( profileName );
}

void KrusaderView::loadPanelProfiles( QString group )
{
  krConfig->setGroup( group );
  MAIN_VIEW->leftMng->loadSettings( krConfig, "Left Tabs" );
  krConfig->setGroup( group );
  MAIN_VIEW->rightMng->loadSettings( krConfig, "Right Tabs" );
  krConfig->setGroup( group );
  MAIN_VIEW->leftMng->setActiveTab( krConfig->readNumEntry( "Left Active Tab", 0 ) );
  krConfig->setGroup( group );
  MAIN_VIEW->rightMng->setActiveTab( krConfig->readNumEntry( "Right Active Tab", 0 ) );
  krConfig->setGroup( group );  
  if( krConfig->readBoolEntry( "Left Side Is Active", true ) )
    MAIN_VIEW->left->slotFocusOnMe();
  else
    MAIN_VIEW->right->slotFocusOnMe();
}

void KrusaderView::savePanelProfiles( QString group )
{
  krConfig->setGroup( group );
  
  MAIN_VIEW->leftMng->saveSettings( krConfig, "Left Tabs", false );
  krConfig->writeEntry( "Left Active Tab", MAIN_VIEW->leftMng->activeTab() );
  MAIN_VIEW->rightMng->saveSettings( krConfig, "Right Tabs", false );
  krConfig->writeEntry( "Right Active Tab", MAIN_VIEW->rightMng->activeTab() );
  krConfig->writeEntry( "Left Side Is Active", MAIN_VIEW->activePanel->isLeft() );
}

void KrusaderView::toggleVerticalMode() {
	if (horiz_splitter->orientation() == Qt::Vertical)
		horiz_splitter->setOrientation(Qt::Horizontal);
	else horiz_splitter->setOrientation(Qt::Vertical);
}

#include "krusaderview.moc"
