/***************************************************************************
                          usermenu.h  -  description
                             -------------------
    begin                : Sat Dec 6 2003
    copyright            : (C) 2003 by Shie Erlich & Rafi Yanai
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef USERMENU_H
#define USERMENU_H

/**

Command: %xYYY%
         x - can be either 'a' for active panel, or 'o' for other panel
         YYY - the specific command

         For example:
           %ap% - active panel path
           %op% - other panel path

In the following commands, we'll use '_' instead of 'a'/'o'. Please substitute as needed.

%_p% - panel path
%_c% - current file (or folder). Note: current != selected
%_s% - selected files and folders
%_cs% - selected files including current file (if it's not already selected)
%_afd% - all files and folders
%_af% - all files (not including folders)
%_ad% - all folders (not including files)
%_an% - number of files and folders
%_anf% - number of files
%_and% - number of folders
%_fm% - filter mask (for example: *, *.cpp, *.h etc.)

*/

#include <qwidget.h>

class UserMenu : public QWidget  {
   Q_OBJECT
public:
	UserMenu(QWidget *parent=0, const char *name=0);
	~UserMenu();
};

#endif
