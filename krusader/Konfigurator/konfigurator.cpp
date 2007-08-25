/***************************************************************************
                                konfigurator.cpp
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



#include "konfigurator.h"
#include "../krusader.h"
#include "../Dialogs/krdialogs.h"
#include "../kicons.h"

#include <kfiledialog.h>
#include <qwidget.h>
//Added by qt3to4:
#include <QPixmap>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include "../defaults.h"
#include "../krusaderview.h"
#include "../GUI/kfnkeys.h"

// the frames
#include "kgwelcome.h"
#include "kgstartup.h"
#include "kglookfeel.h"
#include "kggeneral.h"
#include "kgadvanced.h"
#include "kgarchives.h"
#include "kgdependencies.h"
#include "kgcolors.h"
#include "kguseractions.h"
#include "kgprotocols.h"

Konfigurator::Konfigurator( bool f, int startPage ) : KDialog( (QWidget *)0, 0 ), firstTime(f), internalCall( false ),
      restartGUI( false )
{
  setButtons( KDialog::Help | KDialog::User1 | KDialog::Apply | KDialog::Cancel );
  setDefaultButton( KDialog::User1 );
  setCaption( i18n( "Konfigurator" ) );
  setButtonGuiItem( KDialog::User1, KGuiItem( i18n("Defaults") ) );
  setWindowModality( Qt::WindowModal );

  setPlainCaption(i18n("Konfigurator - Creating Your Own Krusader"));
  kgFrames.setAutoDelete(true);
  widget=new KPageDialog(this);
  widget->setFaceType(KPageDialog::List);

  setButtonGuiItem( KDialog::Cancel, KGuiItem( i18n("Close") ) );

  setHelp("konfigurator");
  
  connect( widget, SIGNAL( currentPageChanged( KPageWidgetItem *, KPageWidgetItem * ) ), this, SLOT( slotPageSwitch() ) );
  connect( &restoreTimer, SIGNAL(timeout()), this, SLOT(slotRestorePage()));
  connect( this, SIGNAL( applyClicked() ), this, SLOT( slotApply() ) );
  connect( this, SIGNAL( cancelClicked() ), this, SLOT( slotCancel() ) );
  connect( this, SIGNAL( user1Clicked() ), this, SLOT( slotUser1() ) );
  
  createLayout( startPage );
  setMainWidget(widget);
  exec();
}

void Konfigurator::newContent(KonfiguratorPage *page)
{
  kgFrames.append(page);
  connect( page, SIGNAL( sigChanged() ), this, SLOT( slotApplyEnable() ) );
}

void Konfigurator::createLayout( int startPage )
{
  // startup
  newContent(new KgStartup(firstTime, widget->addPage(i18n("Startup"),
    i18n("Krusader's settings upon startup"),QPixmap(krLoader->loadIcon("kfm_home",
      KIcon::Desktop,32)))));
  // look n' feel
  newContent(new KgLookFeel(firstTime, widget->addPage(i18n("Look & Feel"),
    i18n("Look & Feel"),QPixmap(krLoader->loadIcon("looknfeel",KIcon::Desktop,32)))));
  // colors
  newContent(new KgColors(firstTime, widget->addPage(i18n("Colors"),
    i18n("Colors"),QPixmap(krLoader->loadIcon("colors",KIcon::Desktop,32)))));
  // general
  newContent(new KgGeneral(firstTime, widget->addPage(i18n("General"),
    i18n("Basic Operations"),QPixmap(krLoader->loadIcon("configure",KIcon::Desktop,32)))));
  // advanced
  newContent(new KgAdvanced(firstTime, widget->addPage(i18n("Advanced"),
    i18n("Be sure you know what you're doing!"),
    QPixmap(krLoader->loadIcon("messagebox_warning",KIcon::Desktop,32)))));
  // archives
  newContent(new KgArchives(firstTime, widget->addPage(i18n("Archives"),i18n("Customize the way Krusader deals with archives"),
    QPixmap(krLoader->loadIcon("tgz",KIcon::Desktop,32)))));
  // dependencies
  newContent(new KgDependencies(firstTime, widget->addPage(i18n("Dependencies"),i18n("Set the full path of the external applications"),
    QPixmap(krLoader->loadIcon("kr_dependencies",KIcon::Desktop,32)))));
  // useractions
  newContent(new KgUserActions(firstTime, widget->addPage(i18n("User Actions"),i18n("Configure your personal actions"),
    QPixmap(krLoader->loadIcon("kr_useractions",KIcon::Desktop,32)))));
  // protocols
  newContent(new KgProtocols(firstTime, widget->addPage(i18n("Protocols"),
    i18n("Link mimes to protocols"), QPixmap(krLoader->loadIcon("about_kde",KIcon::Desktop,32)))));
        
  widget->setCurrentPage( widget->pageIndex( kgFrames.at( startPage )->parentWidget() ) );
  slotApplyEnable();
}

void Konfigurator::slotUser1()
{
  int ndx = searchPage( lastPage = widget->activePageIndex() );
  kgFrames.at( ndx )->setDefaults();
}
#include <kdebug.h>
void Konfigurator::slotApply()
{
  int ndx = searchPage( lastPage = widget->activePageIndex() );
  if( kgFrames.at( ndx )->apply() )
  {
    restartGUI = true;
//    KMessageBox::information(this,i18n("Changes to the GUI will be updated next time you run Krusader."),
//     QString(),"konfigGUInotify");
  }

   // really ugly, but reload the Fn keys just in case - csaba: any better idea?
	krApp->mainView->fnKeys->updateShortcuts();
}

void Konfigurator::slotCancel()
{
  lastPage = widget->activePageIndex();
  if( slotPageSwitch() )
    reject();
}

int Konfigurator::searchPage( int pageNum )
{
  KonfiguratorPage *page;
  int i=0;

  while( ( page = kgFrames.at( i ) ) )
  {
    if( pageNum == widget->pageIndex( page->parentWidget() ) )
      return i;

    i++;
  }
  
  return 0;
}

void Konfigurator::slotApplyEnable()
{  
  int ndx = searchPage( lastPage = widget->activePageIndex() );
  enableButtonApply( kgFrames.at( ndx )->isChanged() );
}

bool Konfigurator::slotPageSwitch()
{
  int ndx = searchPage( lastPage );
  KonfiguratorPage *currentPage = kgFrames.at( ndx );

  if( internalCall )
  {
    internalCall = false;
    return true;
  }
  
  if( currentPage->isChanged() )
  {
    int result = KMessageBox::questionYesNoCancel( 0, i18n("The current page has been changed. Do you want to apply changes?" ));

    switch( result )
    {
    case KMessageBox::No:
      currentPage->loadInitialValues();
      break;
    case KMessageBox::Yes:
      if( currentPage->apply() )
      {
        restartGUI = true;
//        KMessageBox::information(this,i18n("Changes to the GUI will be updated next time you run Krusader."),
//          QString(),"konfigGUInotify");
      }
      break;
    default:
      restoreTimer.start( 0, true );
      return false;
    }
  }

  enableButtonApply( currentPage->isChanged() );
  lastPage = widget->activePageIndex();
  return true;
}

void Konfigurator::slotRestorePage()
{
  if( lastPage != widget->activePageIndex() )
  {
    internalCall = true;
    widget->setCurrentPage( lastPage );
  }
}

#include "konfigurator.moc"
