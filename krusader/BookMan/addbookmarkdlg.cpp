/***************************************************************************
                          addbookmarkdlg.cpp  -  description
                             -------------------
    begin                : St máj 28 2003
    copyright            : (C) 2003 by Ján Hala?a
    email                : xhalasa@fi.muni.cz
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "addbookmarkdlg.h"

#include <klocale.h>
#include <qlabel.h>
#include <qpushbutton.h>

AddBookmarkDlg::AddBookmarkDlg(){
  QObject::connect((QObject*)addButton, SIGNAL(clicked()), (QObject*)this, SLOT(accept()));
  QObject::connect((QObject*)cancelButton, SIGNAL(clicked()), (QObject*)this, SLOT(reject()));

  // internationalization
  bmNameLabel->setText(i18n("Bookmark name:"));
  bmUrlLabel->setText(i18n("URL:"));
  addButton->setText(i18n("Add"));
  cancelButton->setText(i18n("Cancel"));
}

AddBookmarkDlg::~AddBookmarkDlg(){
}

QString AddBookmarkDlg::getName()
{
  return bmNameLineEdit->text();
}

QString AddBookmarkDlg::getUrl()
{
  return bmUrlLineEdit->text();
}

void AddBookmarkDlg::setUrl(QString url)
{
  bmUrlLineEdit->setText(url);
}
