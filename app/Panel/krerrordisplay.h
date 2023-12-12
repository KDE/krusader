/*
    SPDX-FileCopyrightText: 2010 Jan Lepper <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2010-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRERRORDISPLAY_H
#define KRERRORDISPLAY_H

// QtCore
#include <QTimer>
// QtGui
#include <QColor>
// QtWidgets
#include <QLabel>
#include <QWidget>

class KrErrorDisplay : public QLabel
{
    Q_OBJECT
public:
    explicit KrErrorDisplay(QWidget *parent);

    void setText(const QString &text);

private slots:
    void slotTimeout();

private:
    void dim();

    QTimer _dimTimer;
    QColor _startColor;
    QColor _targetColor;
    int _currentDim;
};

#endif
