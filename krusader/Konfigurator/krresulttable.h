/*****************************************************************************
 * Copyright (C) 2005 Dirk Eschler <deschler@users.sourceforge.net>          *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef KRRESULTTABLE_H
#define KRRESULTTABLE_H

// QtCore
#include <QString>
#include <QStringList>
#include <QList>
// QtWidgets
#include <QLabel>
#include <QLayout>
#include <QGridLayout>

#include <KIOWidgets/KRun>
#include <KWidgetsAddons/KSeparator>
#include <KWidgetsAddons/KUrlLabel>

#include "searchobject.h"

class KrResultTable : public QWidget
{
public:
    explicit KrResultTable(QWidget* parent);
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
    QLabel* _label; // generic label

    /**
    * Creates the main grid layout and attaches the table header
    *
    * @return bool  Pointer to the main grid layout
    */
    QGridLayout* initTable();

    /**
    * Applies settings to each cell of the grid layout
    * Supposed to be run after a row was added
    *
    * @param const QGridLayout* grid  The GridLayout
    */
    void adjustRow(QGridLayout* grid);
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class KrArchiverResultTable : public KrResultTable
{
    Q_OBJECT
public:
    explicit KrArchiverResultTable(QWidget* parent);
    virtual ~KrArchiverResultTable();

    bool addRow(SearchObject* search, QGridLayout* grid);

protected:
    KUrlLabel* _nameLabel;

protected slots:
    void website(const QString&);
};

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

class KrToolResultTable : public KrResultTable
{
    Q_OBJECT
public:
    explicit KrToolResultTable(QWidget* parent);
    virtual ~KrToolResultTable();

    bool addRow(SearchObject* search, QGridLayout* grid);

protected:
    QList<Application*> _apps;

protected slots:
    void website(const QString&);
};

#endif
