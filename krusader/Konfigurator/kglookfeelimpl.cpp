/***************************************************************************
                                 kglookfeelimpl.cpp
                             -------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
    web site		 : http://krusader.sourceforge.net
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



#include "kglookfeelimpl.h"
#include "../krusader.h"
#include "../krusaderview.h"
#include "../defaults.h"
#include "../resources.h"
#include "konfigurator.h"
#include <qcheckbox.h>
#include <qspinbox.h>
#include <kfontdialog.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <kdeversion.h>

kgLookFeelImpl::kgLookFeelImpl(bool f, QWidget *parent, const char *name ) :
  kgLookFeel(parent,name), firstTime(f) {
  slotSetup();
}

kgLookFeelImpl::~kgLookFeelImpl(){
}

QString fontType(QFont font) {
  QString t1;
  t1.sprintf("%d",font.pointSize());
  QString t=font.family()+", "+t1;
  return t;
}

void kgLookFeelImpl::slotSetup() {
  krConfig->setGroup("Look&Feel");
  kgMarkDirs->setChecked(krConfig->readBoolEntry("Mark Dirs",_MarkDirs));
  kgMinimizeToTray->setChecked(krConfig->readBoolEntry(
    "Minimize To Tray",_MinimizeToTray));
  kgShowHidden->setChecked(krConfig->readBoolEntry("Show Hidden",_ShowHidden));
  kgWarnOnExit->setChecked(krConfig->readBoolEntry("Warn On Exit",_WarnOnExit));
  kgCaseSensativeSort->setChecked(krConfig->readBoolEntry("Case Sensative Sort",_CaseSensativeSort));
  kgFontLabel->setFont(krConfig->readFontEntry("Filelist Font",_FilelistFont));
  kgFontLabel->setText(fontType(kgFontLabel->font()));
  kgHTMLMinFontSize->setValue(krConfig->readNumEntry("Html Min Font Size",_HtmlMinFontSize));
  // mouse selection
  int i=krConfig->readNumEntry("Mouse Selection",_MouseSelection);
  kgMouseSelectionLeft->setChecked(i==1);
  kgMouseSelectionRight->setChecked(i==2);
  kgMouseSelectionClassic->setChecked(i==0);
  /* disable mouse selection modes */
  kgMouseSelectionLeft->setEnabled(false);
  kgMouseSelectionRight->setEnabled(false);
  kgMouseSelectionClassic->setEnabled(false);

  // combo box handler
  QString tmp = krConfig->readEntry("Filelist Icon Size",_FilelistIconSize);
  if (tmp=="16") kgIconSize->setCurrentItem(0);
  else if (tmp=="22") kgIconSize->setCurrentItem(1);
  else if (tmp=="32") kgIconSize->setCurrentItem(2);
  else kgIconSize->setCurrentItem(3);
  // embed the toolbar editor
  QGridLayout *tab2Layout = new QGridLayout( tab_2 );
  editToolbar = new KEditToolbarWidget(krApp->factory(),tab_2);
  tab2Layout->addWidget(editToolbar,0,0);

  // embed the key-bindings editor
  QGridLayout *tab3Layout = new QGridLayout( tab_3 );
  editKeys = new KKeyChooser(krApp->actionCollection(),tab_3);
  tab3Layout->addWidget(editKeys,0,0);
}

void kgLookFeelImpl::slotDefaultSettings() {
  krConfig->setGroup("Look&Feel");
  krConfig->writeEntry("Mark Dirs",_MarkDirs);
  krConfig->writeEntry("Minimize To Tray",_MinimizeToTray);
  krConfig->writeEntry("Show Hidden",_ShowHidden);
  krConfig->writeEntry("Warn On Exit",_WarnOnExit);
  krConfig->writeEntry("Case Sensative Sort",_CaseSensativeSort);
  krConfig->writeEntry("Filelist Font",_FilelistFont);
  krConfig->writeEntry("Html Min Font Size",_HtmlMinFontSize);
  krConfig->writeEntry("Filelist Icon Size",_FilelistIconSize);
  krConfig->writeEntry("Mouse Selection",_MouseSelection);
  krConfig->sync();
}

void kgLookFeelImpl::slotApplyChanges() {
  krConfig->setGroup("Look&Feel");
  krConfig->writeEntry("Mark Dirs",kgMarkDirs->isChecked());
  krConfig->writeEntry("Minimize To Tray",kgMinimizeToTray->isChecked());
  krConfig->writeEntry("Show Hidden",kgShowHidden->isChecked());
  krConfig->writeEntry("Case Sensative Sort",kgCaseSensativeSort->isChecked());
  krConfig->writeEntry("Warn On Exit",kgWarnOnExit->isChecked());
  krConfig->writeEntry("Filelist Font",kgFontLabel->font());
  krConfig->writeEntry("Html Min Font Size",kgHTMLMinFontSize->value());
  krConfig->writeEntry("Filelist Icon Size",kgIconSize->currentText());
  // mouse selection
  int i;
  if (kgMouseSelectionLeft->isChecked()) i=1;
  else if (kgMouseSelectionRight->isChecked()) i=2;
  else i=0;
  krConfig->writeEntry("Mouse Selection",i);
  editToolbar->save();  // save toolbar settings
	editKeys->save();     // save keyboard shortcuts
  krConfig->sync();

  // return the toolbar to its previous state (thankx to Tin)
  bool toolbar = krApp->actShowToolBar->isChecked();
  krApp->updateGUI();
  if (toolbar) krApp->toolBar()->show();
  else krApp->toolBar()->hide();
}

void kgLookFeelImpl::slotBrowseFont() {
  QFont f;
  int ok=KFontDialog::getFont(f);
  if (ok!=1) return;  // cancelled by the user
  QString temp=fontType(f);
  kgFontLabel->setFont(f);
  kgFontLabel->setText(temp);
}

#include "kglookfeelimpl.moc"
