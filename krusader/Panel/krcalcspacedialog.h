/*****************************************************************************
 * Copyright (C) 2004 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2004 Rafi Yanai <krusader@users.sourceforge.net>            *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
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

#ifndef KRCALCSPACEDIALOG_H
#define KRCALCSPACEDIALOG_H

// QtWidgets
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>

class SizeCalculator;

/** Dialog showing the number of files and directories and its total size for a calculation. */
class KrCalcSpaceDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * Create and show a dialog. If delayed is true the dialog is shown with a delay of 2 seconds
     * to avoid a short appearance and is autoclosed when the calculation finished.
     */
    static void showDialog(QWidget *parent, SizeCalculator *calculator);
    void closeEvent(QCloseEvent *) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *e) Q_DECL_OVERRIDE;

private slots:
    void slotCancel(); // cancel was pressed
    void slotCalculatorFinished();
    void updateResult(); // show the current result in the dialog

private:
    KrCalcSpaceDialog(QWidget *parent, SizeCalculator *calculator);
    SizeCalculator *const m_calculator;

    QLabel *m_label;
    QDialogButtonBox *m_buttonBox;

    QTimer *m_updateTimer;
};

#endif
