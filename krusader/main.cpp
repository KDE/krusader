/***************************************************************************
                                main.cpp
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

// KDE includes
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Krusader includes
#include "krusader.h"

static const char *description =
	I18N_NOOP("Krusader");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE

static KCmdLineOptions options[] =
{
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[]) {
  // ABOUT data information
  KAboutData aboutData( "krusader", I18N_NOOP("Krusader"),
    VERSION, description, KAboutData::License_GPL,
    "(c) 2000-2003, Shie Erlich and Rafi Yanai",
    0,
    "http://krusader.sourceforge.net",
    "krusader@users.sourceforge.net");
  aboutData.addAuthor("Rafi Yanai","Author", "yanai@users.sourceforge.net");
  aboutData.addAuthor("Shie Erlich","Author", "manson@users.sourceforge.net");
  aboutData.addAuthor("Dirk Eschler", "Webmaster and i18n coordinator", "homebass@gmx.net", 0);
  aboutData.addAuthor("Jan Halasa", "Developer", "xhalasa@fi.muni.cz", 0);
  aboutData.addCredit("Heiner Eichmann","FreeBSD port, patchs and general help", "h.eichmann@gmx.de", 0);
  aboutData.addCredit("Cristi Dumitrescu","QA, bug-hunting, patches and general help","cristid@chip.ro",0);
  aboutData.addCredit("Aurelien Gateau","patch for KViewer","aurelien.gateau@free.fr",0);
  aboutData.addCredit("Rafael Munoz","Spanish translation","muror@hotpop.com",0);
  aboutData.addCredit("Zdenko Podobna","Slovak translation","zdpo@mailbox.sk",0);
  aboutData.addCredit("Frank Schoolmeesters","Dutch translation","frank_schoolmeesters@yahoo.com",0);
  aboutData.addCredit("Giuseppe Bordoni", "Italian translation", "geppo@geppozone.com", 0);
  aboutData.addCredit("UTUMI Hirosi", "Japanese translation", "utuhiro@mx12.freecom.ne.jp", 0);
  aboutData.addCredit("Anders Bruun Olsen", "Danish translation", "anders@bruun-olsen.net", 0);
  aboutData.addCredit("Erik Johanssen","Swedish translation","erre@telia.com",0);
  aboutData.addCredit("Anders Linden","Updated Swedish translation","connyosis@gmx.net",0);
  aboutData.addCredit("Mitek","Czech translation","mitek@email.cz",0);
  aboutData.addCredit("Christoph Thielecke","German translation","crissi99@gmx.de",0);
  aboutData.addCredit("René-Pierre Lehmann","French translation","ripi@lepi.org",0);
  aboutData.addCredit("Rafa Munoz","Spanish translation","muror@iespana.es",0);
  aboutData.addCredit("Lukasz Janyst","Polish translation","ljan@wp.pl",0);
  aboutData.addCredit("Kukk Zoltan","Hungarian translation","kukkzoli@freemail.hu",0);
  aboutData.addCredit("Milan Brabec","the first patch ever !","mbrabec@volny.cz",0);

  // Command line arguments ...
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

  // create the application
  KApplication app;
  Krusader *krusader = new Krusader();

  // and set krusader to be the main widget in it
  app.setMainWidget(krusader);
  krusader->show();

  // let's go.
  return app.exec();
}
