/***************************************************************************
                          usermenu.cpp  -  description
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

#include "usermenu.h"
#include "../krusaderview.h"
#include "../krusader.h"
#include "../Panel/listpanel.h"
#include <kdebug.h>

/**

Command: %xYYY%
         x - can be either 'a' for active panel, or 'o' for other panel
         YYY - the specific command

         For example:
           %ap% - active panel path
           %op% - other panel path

In the following commands, we'll use '_' instead of 'a'/'o'. Please substitute as needed.

%_p%    - panel path
%_c%    - current file (or folder). Note: current != selected
%_s%    - selected files and folders
%_cs%   - selected files including current file (if it's not already selected)
%_afd%  - all files and folders
%_af%   - all files (not including folders)
%_ad%   - all folders (not including files)
%_an%   - number of files and folders
%_anf%  - number of files
%_and%  - number of folders
%_fm%   - filter mask (for example: *, *.cpp, *.h etc.)

*/
UMCmd UserMenu::_expressions[NUM_EXPS] = {
  {"%_p%", expPath}
};

#define ACTIVE  krApp->mainView->activePanel
#define OTHER   krApp->mainView->activePanel->otherPanel

QString UserMenu::expPath(const QString& str) {
  if (str.lower() == "%ap%") {
    return ACTIVE->virtualPath;
  } else if (str.lower() == "%op%") {
    return OTHER->virtualPath;
  } else return QString::null;
}

/**
 * only interface function. Executes the menu, does the work and returns a QString
 * containing a command to run. Run that command from a shell and that's it.
 */
QString UserMenu::exec() {
  // execute menu and wait for selection
  int idx = _popup.exec();
  if (idx == -1) return QString::null; // nothing was selected
  if (idx == 0) return QString::null; // todo: insert gui here

  // idx is {1..n} while _entries is {0..n-1}X2
  // so, normalize idx to _entries, and add 1 since we want
  // the command part of the {description,command} pair
  QString cmd = _entries[(idx-1)*2 + 1];

  // replace %% and prepare string
  //cmd = expand(cmd);
  //kdWarning() << "expanded " << cmd << endl;

  return cmd;
}

/**
 * cycle through the input line, replacing every %% expression with valid
 * data from krusader. return the expanded string
 */
QString UserMenu::expand(QString str) {
  QString result;
  int beg, end;
  unsigned int idx = 0;
  while (idx < str.length()) {
    if ((beg = str.find('%', idx)) == -1) break;
    if ((end = str.find('%', beg+1)) == -1) {
      kdWarning() << "Error: unterminated % in UserMenu::expand" << endl;
      return QString::null;
    }
    kdWarning() << str.mid(beg, end-beg+1) << endl;
    idx = end+1;
  }
}

UserMenu::UserMenu(QWidget *parent, const char *name ) : QWidget(parent,name) {
  _popup.insertTitle("User Menu");

  // read entries from config file. Note: entries are marked 1..n, so that entry 0 is always
  // available. It is used by the "add new entry" command.
  ///////// for now, create a dummy list
  _entries += "ll in active";
  _entries += "ll %ap%";

  _entries += "ll in active to file";
  _entries += "ll %ap% > /tmp/list.txt";

  // fill popup menu. Note: this code assumes that the _entries list contains pairs
  // of {descripion, command}. If this is not the case, the code will fail.
  // However, it should have been checked above (read entries from file)
  int idx = 1;
  for ( QStringList::Iterator it = _entries.begin(); it != _entries.end(); ++it ) {
    _popup.insertItem(*it, idx++);
    ++it;
  }

  // add the "add new entry" command
  _popup.insertSeparator();
  _popup.insertItem("Add new entry", 0);
}

UserMenu::~UserMenu() {
}


