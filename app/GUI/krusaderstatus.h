/*
    SPDX-FileCopyrightText: 2000 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2000 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRUSADERSTATUS_H
#define KRUSADERSTATUS_H

// QtWidgets
#include <QLabel>
#include <QStatusBar>
#include <QWidget>

class KrusaderStatus : public QStatusBar
{
    Q_OBJECT
public:
    explicit KrusaderStatus(QWidget *parent = nullptr);
    ~KrusaderStatus() override;

private:
    QLabel *mess;
};

#endif
