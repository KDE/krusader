/***************************************************************************
                                kfnkeys.cpp
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

#include <klocale.h>
#include <kglobalsettings.h>
#include "kfnkeys.h"
#include "../krusader.h"
#include "../defaults.h"
#include "../krslots.h"

KFnKeys::KFnKeys(QWidget *parent, char *name): QWidget(parent,name) {
		krConfig->setGroup("Look&Feel");
		////////////////////////////////
		
		setFont( KGlobalSettings::generalFont() );
		layout=new QGridLayout(this,1,9);	// 9 keys
		F2=new QPushButton( i18n("F2 Term  ") ,this);	
			connect(F2,SIGNAL(clicked()), SLOTS,
						SLOT(terminal()));
		F3=new QPushButton( i18n("F3 View  ") ,this);	
			connect(F3,SIGNAL(clicked()), SLOTS,
						SLOT(view()));
		F4=new QPushButton( i18n("F4 Edit  ") ,this);
			connect(F4,SIGNAL(clicked()), SLOTS,
						SLOT(edit()));
		F5=new QPushButton( i18n("F5 Copy  ") ,this);	
			connect(F5,SIGNAL(clicked()), SLOTS,
						SLOT(copyFiles()));
		F6=new QPushButton( i18n("F6 Move") ,this);
			connect(F6,SIGNAL(clicked()), SLOTS,
						SLOT(moveFiles()));
		F7=new QPushButton( i18n("F7 Mkdir ") ,this);	
			connect(F7,SIGNAL(clicked()), SLOTS,
						SLOT(mkdir()));
		F8=new QPushButton( i18n("F8 Delete") ,this);
			connect(F8,SIGNAL(clicked()), SLOTS,
						SLOT(deleteFiles()));
		F9=new QPushButton( i18n("F9 Rename") ,this);	
			connect(F9,SIGNAL(clicked()), SLOTS,
						SLOT(rename()));
		F10=new QPushButton( i18n("F10 Quit ") ,this);
			connect(F10,SIGNAL(clicked()),krApp,
						SLOT(quitKrusader()));
		
		layout->addWidget(F2,0,0);
		layout->addWidget(F3,0,1);
		layout->addWidget(F4,0,2);
		layout->addWidget(F5,0,3);
		layout->addWidget(F6,0,4);
		layout->addWidget(F7,0,5);
		layout->addWidget(F8,0,6);
		layout->addWidget(F9,0,7);
		layout->addWidget(F10,0,8);
		
		layout->activate();
}

#include "kfnkeys.moc"

