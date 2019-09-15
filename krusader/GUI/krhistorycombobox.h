/*****************************************************************************
 * Copyright (C) 2018-2019 Shie Erlich <krusader@users.sourceforge.net>      *
 * Copyright (C) 2018-2019 Rafi Yanai <krusader@users.sourceforge.net>       *
 * Copyright (C) 2018-2019 Krusader Krew [https://krusader.org]              *
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

#ifndef KRHISTORYCOMBOBOX_H
#define KRHISTORYCOMBOBOX_H

#include <KCompletion/KHistoryComboBox>

/**
 * A specialized version of a KHistoryComboBox, e.g. it deletes the current
 *  item when the user presses Shift+Del
 */
class KrHistoryComboBox : public KHistoryComboBox
{
    Q_OBJECT

public:
    explicit KrHistoryComboBox(bool useCompletion, QWidget *parent = nullptr);
};

#endif // KRHISTORYCOMBOBOX_H
