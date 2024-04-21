/*
    SPDX-FileCopyrightText: 2018-2020 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2018-2020 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2018-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRHISTORYCOMBOBOX_H
#define KRHISTORYCOMBOBOX_H

#include <KHistoryComboBox>

/**
 * A specialized version of a KHistoryComboBox, e.g. it deletes the current
 *  item when the user presses Shift+Del
 */
class KrHistoryComboBox : public KHistoryComboBox
{
    Q_OBJECT

public:
    explicit KrHistoryComboBox(bool useCompletion, QWidget *parent = nullptr);
    explicit KrHistoryComboBox(QWidget *parent = nullptr)
        : KrHistoryComboBox(true, parent)
    {
    }
};

#endif // KRHISTORYCOMBOBOX_H
