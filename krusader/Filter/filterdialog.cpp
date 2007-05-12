/***************************************************************************
                      filterdialog.cpp  -  description
                             -------------------
    copyright            : (C) 2005 + by Csaba Karai
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

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "filterdialog.h"
#include "filtertabs.h"
#include "generalfilter.h"

#include <klocale.h>
//Added by qt3to4:
#include <Q3GridLayout>

FilterDialog::FilterDialog(  QWidget *parent, const char *name )
    : KDialogBase( parent, name, true, i18n("Krusader::Choose Files"), Ok|Cancel )
{
  Q3GridLayout *filterGrid = new Q3GridLayout( this->layout() );
  filterGrid->setSpacing( 6 );
  filterGrid->setMargin( 11 );

  QTabWidget *filterWidget = new QTabWidget( this, "filterTabs" );

  filterTabs = FilterTabs::addTo( filterWidget, FilterTabs::HasProfileHandler );
  generalFilter = (GeneralFilter *)filterTabs->get( "GeneralFilter" );
  generalFilter->searchFor->setEditText( "*" );

  filterGrid->addWidget( filterWidget, 0, 0 );
  setMainWidget(filterWidget);

  generalFilter->searchFor->setFocus();

  connect( filterTabs, SIGNAL( closeRequest(bool) ), this, SLOT( slotCloseRequest(bool) ) );

  exec();
}

KRQuery FilterDialog::getQuery()
{
  return query;
}

void FilterDialog::slotCloseRequest( bool doAccept )
{
  if( doAccept )
  {
    slotOk();
    accept();
  }
  else
    reject();
}

void FilterDialog::slotOk()
{
  if( filterTabs->fillQuery( &query ) )
  {
    KDialogBase::slotOk();
    return;
  }

  query = KRQuery();
}

#include "filterdialog.moc"
