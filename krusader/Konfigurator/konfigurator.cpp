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
#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include "../defaults.h"

// the frames
#include "kgwelcome.h"
#include "kgstartupimpl.h"
#include "kglookfeelimpl.h"
#include "kggeneralimpl.h"
#include "kgadvancedimpl.h"
#include "kgarchivesimpl.h"

Konfigurator::Konfigurator(bool f) : KDialogBase(0,0,true,"Konfigurator"), firstTime(f) {
  setButtonApplyText(i18n("Defaults"),i18n("Reset ALL of Krusader's options to factory defaults"),0);
  setPlainCaption(i18n("Konfigurator - Creating Your Own Krusader"));
  kgFrames.setAutoDelete(true);
  widget=new KJanusWidget(this,0,KJanusWidget::IconList);
  createLayout();
  setMainWidget(widget);
  if (exec()) {
	  KMessageBox::information(this,i18n("Changes to the GUI will be updated next time you run Krusader."),
	    QString::null,"konfigGUInotify");
  }
}


Konfigurator::~Konfigurator(){
}

void Konfigurator::newContent(QFrame *widget) {
  kgFrames.append(widget);
  connect(this,SIGNAL(applyChanges()),widget,SLOT(slotApplyChanges()));
  connect(this,SIGNAL(defaultSettings()),widget,SLOT(slotDefaultSettings()));
}

void Konfigurator::createLayout() {
  // welcome
  new kgWelcome(widget->addPage(i18n("Welcome"),i18n("Welcome to Konfigurator"),
    QPixmap(krLoader->loadIcon("krusader",KIcon::Desktop,32))));
  // startup
  newContent(new kgStartupImpl(firstTime, widget->addPage(i18n("Startup"),
    i18n("Krusader's setting upon startup"),QPixmap(krLoader->loadIcon("gear",
      KIcon::Desktop,32)))));
  // look n' feel
  newContent(new kgLookFeelImpl(firstTime, widget->addPage(i18n("Look & Feel"),
    i18n("Look & Feel"),QPixmap(krLoader->loadIcon("appearance",KIcon::Desktop,32)))));
  // general
  newContent(new kgGeneralImpl(firstTime, widget->addPage(i18n("General"),
    i18n("Basic Operations"),QPixmap(krLoader->loadIcon("configure",KIcon::Desktop,32)))));
  // advanced
  newContent(new kgAdvancedImpl(firstTime, widget->addPage(i18n("Advanced"),
    i18n("Be sure you know what you're doing"),
    QPixmap(krLoader->loadIcon("file_important",KIcon::Desktop,32)))));
  // archives
  newContent(new kgArchivesImpl(firstTime, widget->addPage(i18n("Archives"),
    i18n("Costumize the way Krusader deals with archives"),QPixmap(krLoader->loadIcon(
    "tgz",KIcon::Desktop,32)))));
  widget->showPage(0);
}

void Konfigurator::slotOk(){
  emit applyChanges();
  accept();
}

// actually defaults all values
///////////////////////////////
void Konfigurator::slotApply() {
  emit defaultSettings();
  accept();
}

void Konfigurator::slotCancel() {
  reject();
}

void Konfigurator::genericBrowse(QLineEdit *&target, QString startDir, QString caption) {
  startDir=startDir.simplifyWhiteSpace();
  if (startDir.isEmpty()) startDir="/";
  QString dir=KFileDialog::getExistingDirectory (startDir,0,caption);
  if (dir!=QString::null)target->setText(dir);
}

#include "konfigurator.moc"
