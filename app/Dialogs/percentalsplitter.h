/*
    SPDX-FileCopyrightText: 2006 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PERCENTALSPLITTER_H
#define PERCENTALSPLITTER_H

// QtWidgets
#include <QLabel>
#include <QSplitter>

class PercentalSplitter : public QSplitter
{
    Q_OBJECT

public:
    explicit PercentalSplitter(QWidget *parent = nullptr);
    ~PercentalSplitter() override;

    QString toolTipString(int p);

protected:
    void showEvent(QShowEvent *event) override;

protected slots:
    void slotSplitterMoved(int pos, int index);

private:
    QLabel *label;
    int opaqueOldPos;
    QPoint labelLocation;
};

#endif /* __PERCENTAL_SPLITTER__ */
