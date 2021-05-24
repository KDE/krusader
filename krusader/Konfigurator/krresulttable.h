/*
    SPDX-FileCopyrightText: 2005 Dirk Eschler <deschler@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005-2020 Krusader Krew [https://krusader.org]

    This file is part of Krusader [https://krusader.org].

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    ~KrResultTable() override;

    /**
    * Adds a row of search results to the end of a QGridLayout
    * Each KrResultTable has to implement it
    *
    * @param search  Name of the SearchObject
    * @param grid    The GridLayout where the row is inserted
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
    * @param grid  The GridLayout
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
    ~KrArchiverResultTable() override;

    bool addRow(SearchObject* search, QGridLayout* grid) override;

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
    ~KrToolResultTable() override;

    bool addRow(SearchObject* search, QGridLayout* grid) override;

protected:
    QList<Application*> _apps;

protected slots:
    void website(const QString&);
};

#endif
