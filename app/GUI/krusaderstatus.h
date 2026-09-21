/*
    SPDX-FileCopyrightText: Krusader Krew <https://krusader.org>
    SPDX-FileCopyrightText: Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: Rafi Yanai <krusader@users.sourceforge.net>

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
