/***************************************************************************
                             krresulttabledialog.h
                             -------------------
    copyright            : (C) 2005 by Dirk Eschler & Krusader Krew
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

#ifndef KRRESULTTABLEDIALOG_H
#define KRRESULTTABLEDIALOG_H

#include <qlabel.h>
#include <qlayout.h>
#include <qstringlist.h>
#include <QVBoxLayout>

#include <kdialog.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <klocale.h>

#include "../krusader.h"
#include "../Konfigurator/krresulttable.h"
#include "../Konfigurator/searchobject.h"

/**
@author Dirk Eschler <deschler@users.sourceforge.net>
*/
class KrResultTableDialog : public KDialog
{
public:

  enum DialogType
  {
    Archiver = 1,
    Tool = 2
  };

  KrResultTableDialog(QWidget *parent, DialogType type, const QString& caption, const QString& heading, const QString& headerIcon=QString(), const QString& hint=QString());
  virtual ~KrResultTableDialog();

  const QString& getHeading() const { return _heading; }
  const QString& getHint() const { return _hint; }
  void setHeading(const QString& s) { _heading = s; }
  void setHint(const QString& s) { _hint = s; }

protected:
  QString _heading;
  QString _hint;

  QLabel* _headingLabel;
  QLabel* _iconLabel;
  QLabel* _hintLabel;
  QWidget* _page;
  QVBoxLayout* _topLayout;
  KrResultTable* _resultTable;
};

#endif
