/***************************************************************************
                                 kggeneralimpl.cpp
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



#include "kggeneralimpl.h"
#include <qradiobutton.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include "../krusader.h"
#include "../defaults.h"

kgGeneralImpl::kgGeneralImpl(bool f, QWidget *parent, const char *name ) :
  kgGeneral(parent,name), firstTime(f) {
  if (firstTime) slotFindTools();
  slotSetup();
}

kgGeneralImpl::~kgGeneralImpl(){
}

void kgGeneralImpl::slotSetup() {
  krConfig->setGroup("General");
  kgMimetypeMagic->setChecked(
    krConfig->readBoolEntry("Mimetype Magic",_MimetypeMagic));
  bool tmp=krConfig->readBoolEntry("Move To Trash",_MoveToTrash);
  kgMoveToTrash->setChecked(tmp);
  kgDeleteFiles->setChecked(!tmp);
  kgTerminal->setText(krConfig->readEntry("Terminal",_Terminal));
  kgEditor->setText(krConfig->readEntry("Editor",_Editor));
  kgTempDir->setText(krConfig->readEntry("Temp Directory",_TempDirectory));
}

void genericFileBrowse(QString startDir, QLineEdit *target, QString caption) {
  startDir=startDir.simplifyWhiteSpace();
  if (startDir.isEmpty()) startDir="/";
  QString dir=KFileDialog::getOpenFileName(startDir,0,0,caption);
  if (dir!=QString::null) target->setText(dir);
}

void genericDirBrowse(QString startDir, QLineEdit *target, QString caption) {
  startDir=startDir.simplifyWhiteSpace();
  if (startDir.isEmpty()) startDir="/";
  QString dir=KFileDialog::getExistingDirectory(startDir,0,caption);
  if (dir!=QString::null) target->setText(dir);
}

void kgGeneralImpl::slotBrowseEditor() {
  genericFileBrowse(kgEditor->text(),kgEditor,i18n("Choose an editor"));
}

void kgGeneralImpl::slotBrowseTerminal() {
  genericFileBrowse(kgTerminal->text(),kgTerminal,i18n("Choose a terminal"));
}

void kgGeneralImpl::slotBrowseTempDir() {
  genericDirBrowse(kgTempDir->text(),kgTempDir,i18n("Choose a temporary directory"));
}

void kgGeneralImpl::slotApplyChanges() {
  krConfig->setGroup("General");
  krConfig->writeEntry("Mimetype Magic",kgMimetypeMagic->isChecked());
  krConfig->writeEntry("Move To Trash",kgMoveToTrash->isChecked());
  krConfig->writeEntry("Terminal",kgTerminal->text());
  krConfig->writeEntry("Editor",kgEditor->text());
  krConfig->writeEntry("Temp Directory",QDir(kgTempDir->text()).path());
  krConfig->sync();
}

void kgGeneralImpl::slotDefaultSettings() {
  krConfig->setGroup("General");
  krConfig->writeEntry("Mimetype Magic",_MimetypeMagic);
  krConfig->writeEntry("Move To Trash",_MoveToTrash);
  krConfig->writeEntry("Terminal",_Terminal);
  krConfig->writeEntry("Editor",_Editor);
  krConfig->writeEntry("Temp Directory",_TempDirectory);
  krConfig->sync();
}

void kgGeneralImpl::slotFindTools() {
  #define PS(x) lst.contains(x)>0
  QString info;
  QStringList lst=Krusader::supportedTools(); // get list of availble tools

  info+=i18n("Searching for tools...\nSearch results:\n\n");
  if (PS("DIFF")) info+=i18n("diff: found ")+lst[lst.findIndex("DIFF") + 1]+i18n(", compare by content availble.\n");
  else info+=i18n("diff: no diff frontends found. Compare by content disabled.\nhint: Krusader supports kdiff and xxdiff\n\n");

  if (PS("MAIL")) info+=i18n("mail: found ")+lst[lst.findIndex("MAIL") + 1]+i18n(", sending files by email enabled.\n");
  else info+=i18n("mail: no compatible mail-programs found. Sending files by email is disabled.\nhint: Krusader supports kmail\n\n");

  if (PS("RENAME")) info+=i18n("rename: found ")+lst[lst.findIndex("RENAME") + 1]+i18n(", multipule rename enabled.\n");
  else info+=i18n("rename: no compatible renamer-programs found. multipule rename is disabled.\nhint: Krusader supports krename\n\n");


  info+=i18n("\nIf you install new tools, please install them");
  info+=i18n("\nto your path. (ie: /usr/bin, /usr/local/bin etc.)");
  KMessageBox::information(0,info,i18n("Results"));
}

#include "kggeneralimpl.moc"
