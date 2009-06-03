/*****************************************************************************
 * Copyright (C) 2006 Václav Juza <vaclavjuza@gmail.com>                     *
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

#include "kcmdmodebutton.h"

#include "../krusader.h"
#include "../krusaderview.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kmenu.h>
#include <kactionmenu.h>

#include <kdebug.h>

/**
 * KCMDModeButton class, which represents a button with popup menu to switch
 * the mode of the krusader built-in command-line
 */
KCMDModeButton::KCMDModeButton( QWidget *parent ) : QToolButton( parent ) {
  setFixedSize( 22, 20 );
/* // from the old terminal-button:
  setText( i18n( "If pressed, Krusader executes command line in a terminal." ) );
  setToolTip( i18n( "If pressed, Krusader executes command line in a terminal." ) );
  QWhatsThis::add( terminal, i18n(
                        "The 'run in terminal' button allows Krusader "
                        "to run console (or otherwise non-graphical) "
                        "programs in a terminal of your choice. If it's "
                        "pressed, terminal mode is active." ) );
*/
  setIcon( SmallIcon( "konsole" ) );
  adjustSize();
  action = new KActionMenu( i18n("Execution mode"), this );
  Q_CHECK_PTR( action );
  for( int i=0; Krusader::execTypeArray[i] != 0; i++ )
  {
    action->addAction( *Krusader::execTypeArray[i] );
  }
  QMenu *pP = action->menu();
  Q_CHECK_PTR( pP );
  setMenu( pP );
  setPopupMode( QToolButton::InstantPopup );
  setAcceptDrops( false );
}

KCMDModeButton::~KCMDModeButton() {
  delete action;
}

/** called when clicked to the button */
void KCMDModeButton::showMenu() {
  QMenu * pP = menu();
  if ( pP ) {
    menu() ->exec( mapToGlobal( QPoint( 0, 0 ) ) );
  }
}



#include "kcmdmodebutton.moc"
