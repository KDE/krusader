/*****************************************************************************
 * Copyright (C) 2006 Csaba Karai <krusader@users.sourceforge.net>           *
 * Copyright (C) 2006-2019 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#ifndef PERCENTALSPLITTER_H
#define PERCENTALSPLITTER_H

// QtWidgets
#include <QSplitter>
#include <QLabel>

class PercentalSplitter : public QSplitter
{
    Q_OBJECT

public:
    explicit PercentalSplitter(QWidget * parent = 0);
    virtual ~PercentalSplitter();

    QString toolTipString(int p);

protected:
    virtual void showEvent(QShowEvent * event) Q_DECL_OVERRIDE;

protected slots:
    void slotSplitterMoved(int pos, int index);

private:
    QLabel * label;
    int opaqueOldPos;
    QPoint labelLocation;
};

#endif /* __PERCENTAL_SPLITTER__ */
