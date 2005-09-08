/***************************************************************************
                             krresulttable.h
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

#ifndef KRRESULTTABLE_H
#define KRRESULTTABLE_H

#include <qstring.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qgrid.h>
#include <qstringlist.h>
#include <qvaluevector.h>

#include <kiconloader.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <klocale.h>
#include <krun.h>
#include <kseparator.h>
#include <kurllabel.h>

#include "searchobject.h"
#include "../krusader.h"
#include "../krservices.h"
#include "../VFS/krarchandler.h"

/**
@author Dirk Eschler <deschler@users.sourceforge.net>
*/
class KrResultTable : public QWidget
{
public:
  KrResultTable(QWidget* parent);
  virtual ~KrResultTable();

  /**
  * Adds a row of search results to the end of a QGridLayout
  * Each KrResultTable has to implement it
  *
  * @param const SearchObject* search  Name of the SearchObject
  * @param const QGridLayout*  grid    The GridLayout where the row is inserted
  *
  * @return bool  True if row was added successfully to rows, else false
  */
  virtual bool addRow(SearchObject* search, QGridLayout* grid) = 0;

protected:
  QStringList _supported;
  QStringList _tableHeaders;
  int _numColumns;
  int _numRows;

  QGridLayout* _grid;
  QHBox* _iconBox;
  QLabel* _label; // generic label
  QLabel* _nameLabel;
  QLabel* _foundLabel;
  QLabel* _noteLabel;

  /**
  * Creates the main grid layout and attaches the table header
  *
  * @return bool  Pointer to the main grid layout
  */
  QGridLayout* initTable();
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class KrArchiverResultTable : public KrResultTable
{
  Q_OBJECT
public:
  KrArchiverResultTable(QWidget* parent);
  virtual ~KrArchiverResultTable();

  bool addRow(SearchObject* search, QGridLayout* grid);

protected:
  KURLLabel* _nameLabel;
  QLabel* _packingLabel;
  QLabel* _unpackingLabel;

protected slots:
  void website(const QString&);
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class KrToolResultTable : public KrResultTable
{
  Q_OBJECT
public:
  KrToolResultTable(QWidget* parent);
  virtual ~KrToolResultTable();

  bool addRow(SearchObject* search, QGridLayout* grid);

protected:
  QLabel* _supportedLabel;
  QLabel* _statusLabel;
  QValueVector<Application*> _apps;

protected slots:
  void website(const QString&);
};

#endif
