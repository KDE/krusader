/***************************************************************************
                          addbookmarkdlg.h  -  description
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

#ifndef ADDBOOKMARKDLG_H
#define ADDBOOKMARKDLG_H

#include <addbookmarkform.h>
#include <klineedit.h>

/**
  *@author Ján Hala¹a
  */

class AddBookmarkDlg : public AddBookmarkForm  {
public: 
  AddBookmarkDlg();
  ~AddBookmarkDlg();

  QString getName();
  QString getUrl();

  void setUrl(QString url);
};

#endif
