/***************************************************************************
                                 kgadvancedimpl.cpp
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



#include "kgadvancedimpl.h"
#include <qcheckbox.h>
#include <qspinbox.h>
#include "../krusader.h"
#include "../defaults.h"

kgAdvancedImpl::kgAdvancedImpl(bool f, QWidget *parent, const char *name ) :
  kgAdvanced(parent,name), firstTime(f) {
  slotSetup();
}
kgAdvancedImpl::~kgAdvancedImpl(){
}

void kgAdvancedImpl::slotSetup() {
  krConfig->setGroup("Advanced");
  kgRootSwitch->setChecked(!krConfig->readBoolEntry("Permission Check",_PermCheck));
  kgAutomount->setChecked(krConfig->readBoolEntry("AutoMount",_AutoMount));
  kgNonEmpty->setChecked(krConfig->readBoolEntry("Confirm Unempty Dir",_ConfirmUnemptyDir));
  kgDelete->setChecked(krConfig->readBoolEntry("Confirm Delete",_ConfirmDelete));
  kgCopy->setChecked(krConfig->readBoolEntry("Confirm Copy",_ConfirmCopy));
  kgMove->setChecked(krConfig->readBoolEntry("Confirm Move",_ConfirmMove));
  kgCacheSize->setValue(krConfig->readNumEntry("Icon Cache Size",_IconCacheSize));
}

void kgAdvancedImpl::slotDefaultSettings() {
  krConfig->setGroup("Advanced");
  krConfig->writeEntry("Permission Check",_PermCheck);
  krConfig->writeEntry("AutoMount",_AutoMount);
  krConfig->writeEntry("Confirm Unempty Dir",_ConfirmUnemptyDir);
  krConfig->writeEntry("Confirm Delete",_ConfirmDelete);
  krConfig->writeEntry("Confirm Copy",_ConfirmCopy);
  krConfig->writeEntry("Confirm Move",_ConfirmMove);
  krConfig->writeEntry("Icon Cache Size",_IconCacheSize);

  krConfig->sync();
}

void kgAdvancedImpl::slotApplyChanges() {
  krConfig->setGroup("Advanced");
  krConfig->writeEntry("Permission Check",!kgRootSwitch->isChecked());
  krConfig->writeEntry("AutoMount",kgAutomount->isChecked());
  krConfig->writeEntry("Confirm Unempty Dir",kgNonEmpty->isChecked());
  krConfig->writeEntry("Confirm Delete",kgDelete->isChecked());
  krConfig->writeEntry("Confirm Copy",kgCopy->isChecked());
  krConfig->writeEntry("Confirm Move",kgMove->isChecked());
  krConfig->writeEntry("Icon Cache Size",kgCacheSize->value());

  krConfig->sync();
}

#include "kgadvancedimpl.moc"
