/***************************************************************************
                                 kgstartupimpl.cpp
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



#include "kgstartupimpl.h"
#include "../resources.h"
#include "../krusader.h"
#include "../defaults.h"
#include "konfigurator.h"
#include <qgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qtoolbutton.h>
#include <klocale.h>

kgStartupImpl::kgStartupImpl(bool f, QWidget *parent, const char *name ) :
  kgStartup(parent,name), firstTime(f) {
  slotSetup();
}

kgStartupImpl::~kgStartupImpl(){
}

void kgStartupImpl::slotSetup() {
  // setup the 'user interface' part
  krConfig->setGroup("Startup");
  uiSaveSettings->setChecked(krConfig->readBoolEntry("UI Save Settings",_UiSave));
  uiToolbar->setChecked(krConfig->readBoolEntry("Show tool bar",_ShowToolBar));
  uiStatusbar->setChecked(krConfig->readBoolEntry("Show status bar",_ShowStatusBar));
  uiFnKeys->setChecked(krConfig->readBoolEntry("Show FN Keys",_ShowFNkeys));
  uiCmdLine->setChecked(krConfig->readBoolEntry("Show Cmd Line",_ShowCmdline));
  uiTerminalEmulator->setChecked(krConfig->readBoolEntry("Show Terminal Emulator",_ShowTerminalEmulator));
  uiPositionSize->setChecked(krConfig->readBoolEntry("Remember Position",_RememberPos));
  slotUiSave();

  // setup the 'panels' part
  QString ptype=krConfig->readEntry("Left Panel Type",_LeftPanelType)+", "+
                krConfig->readEntry("Right Panel Type",_RightPanelType);
  for (int i=0; i<panelsTypes->count(); ++i) {
    panelsTypes->setCurrentItem(i);
    if (panelsTypes->currentText()==i18n(ptype.local8Bit())) break;
  }

  ptype=krConfig->readEntry("Left Panel Origin",_LeftPanelOrigin);
  for (int i=0; i<panelsLeftType->count(); ++i) {
    panelsLeftType->setCurrentItem(i);
    if (panelsLeftType->currentText()==i18n(ptype.local8Bit())) break;
  }
  ptype=krConfig->readEntry("Right Panel Origin",_RightPanelOrigin);
  for (int i=0; i<panelsRightType->count(); ++i) {
    panelsRightType->setCurrentItem(i);
    if (panelsRightType->currentText()==i18n(ptype.local8Bit())) break;
  }
  panelsLeftHomepage->setText(krConfig->readEntry("Left Panel Homepage",_LeftHomepage));
  panelsRightHomepage->setText(krConfig->readEntry("Right Panel Homepage",_RightHomepage));
  slotPanelsTypes();

  bool tmp=krConfig->readBoolEntry("Panels Save Settings",_PanelsSave);
  panelsSave->setChecked(tmp);
  panelsDontSave->setChecked(!tmp);
  if (tmp)
    slotPanelsSave();
  else slotPanelsDontSave();
}

void kgStartupImpl::slotApplyChanges() {
  // 'user interface' part
  krConfig->setGroup("Startup");
  krConfig->writeEntry("UI Save Settings",uiSaveSettings->isChecked());
  krConfig->writeEntry("Show tool bar",uiToolbar->isChecked());
  krConfig->writeEntry("Show status bar",uiStatusbar->isChecked());
  krConfig->writeEntry("Show FN Keys",uiFnKeys->isChecked());
  krConfig->writeEntry("Show Cmd Line",uiCmdLine->isChecked());
  krConfig->writeEntry("Show Terminal Emulator",uiTerminalEmulator->isChecked());
  krConfig->writeEntry("Remember Position",uiPositionSize->isChecked());

  // setup the 'panels' part
  QString s=panelsTypes->currentText();
  QString leftP=s.left(s.find(","));
  QString rightP=s.right(s.length()-s.find(",")-2);
  krConfig->writeEntry("Left Panel Type",leftP);
  krConfig->writeEntry("Right Panel Type",rightP);
  krConfig->writeEntry("Left Panel Origin",panelsLeftType->currentText());
  krConfig->writeEntry("Right Panel Origin",panelsRightType->currentText());
  krConfig->writeEntry("Left Panel Homepage",panelsLeftHomepage->text());
  krConfig->writeEntry("Right Panel Homepage",panelsRightHomepage->text());
  krConfig->writeEntry("Panels Save Settings",panelsSave->isChecked());
  krConfig->sync();
}

void kgStartupImpl::slotPanelsSave() {
  // deactivate all other objects in the 'panels' group except the
  // other radio button
  TextLabel1->setEnabled(false);
  panelsTypes->setEnabled(false);
  panelsLeftLabel1->setEnabled(false);
  panelsLeftLabel2->setEnabled(false);
  panelsLeftType->setEnabled(false);
  panelsLeftHomepage->setEnabled(false);
  panelsLeftBrowse->setEnabled(false);
  panelsRightLabel1->setEnabled(false);
  panelsRightLabel2->setEnabled(false);
  panelsRightType->setEnabled(false);
  panelsRightHomepage->setEnabled(false);
  panelsRightBrowse->setEnabled(false);

}

void kgStartupImpl::slotPanelsDontSave() {
  // reactivate all other objects in the 'panels' group
  TextLabel1->setEnabled(true);
  panelsTypes->setEnabled(true);
  panelsLeftLabel1->setEnabled(true);
  panelsLeftLabel2->setEnabled(true);
  panelsLeftType->setEnabled(true);
  panelsLeftHomepage->setEnabled(true);
  panelsLeftBrowse->setEnabled(true);
  panelsRightLabel1->setEnabled(true);
  panelsRightLabel2->setEnabled(true);
  panelsRightType->setEnabled(true);
  panelsRightHomepage->setEnabled(true);
  panelsRightBrowse->setEnabled(true);
  // now, call the panelsTypes slot to determine panel's state
  slotPanelsTypes();

}

void kgStartupImpl::slotPanelsTypes() {
  // check what panel configuration was chosen by the user and
  // activate/deactivate widgets accordingly
  QString s=panelsTypes->currentText();
  QString leftP=s.left(s.find(","));
  QString rightP=s.right(s.length()-s.find(",")-2);
  bool val;
  if (leftP!=i18n("List"))
    val=false;   // disable relevent items
  else val=true; // enable them
  panelsLeftLabel1->setEnabled(val);
  panelsLeftLabel2->setEnabled(val);
  panelsLeftType->setEnabled(val);
  panelsLeftHomepage->setEnabled(val);
  panelsLeftBrowse->setEnabled(val);
  leftPanelIsList=val;
  slotPanelsLeftType();

  if (rightP!=i18n("List"))
    val=false;   // disable relevent items
  else val=true; // enable them
  panelsRightLabel1->setEnabled(val);
  panelsRightLabel2->setEnabled(val);
  panelsRightType->setEnabled(val);
  panelsRightHomepage->setEnabled(val);
  panelsRightBrowse->setEnabled(val);
  rightPanelIsList=val;
  slotPanelsRightType();
}

void kgStartupImpl::slotPanelsLeftType() {
  bool val=(panelsLeftType->currentText()==i18n("homepage") && leftPanelIsList);
  panelsLeftLabel2->setEnabled(val);
  panelsLeftHomepage->setEnabled(val);
  panelsLeftBrowse->setEnabled(val);
}

void kgStartupImpl::slotPanelsRightType() {
  bool val=(panelsRightType->currentText()==i18n("homepage") && rightPanelIsList);
  panelsRightLabel2->setEnabled(val);
  panelsRightHomepage->setEnabled(val);
  panelsRightBrowse->setEnabled(val);
}

void kgStartupImpl::slotLeftBrowse() {
  Konfigurator::genericBrowse(panelsLeftHomepage,
    panelsLeftHomepage->text(),i18n("Left panel homepage"));
}

void kgStartupImpl::slotRightBrowse() {
  Konfigurator::genericBrowse(panelsRightHomepage,
    panelsRightHomepage->text(),i18n("Right panel homepage"));
}

void kgStartupImpl::slotUiSave() {
  bool val=!(uiSaveSettings->isChecked());
  uiTerminalEmulator->setEnabled(val);
  uiToolbar->setEnabled(val);
  uiStatusbar->setEnabled(val);
  uiFnKeys->setEnabled(val);
  uiCmdLine->setEnabled(val);
  uiPositionSize->setEnabled(val);
}

#include "kgstartupimpl.moc"
