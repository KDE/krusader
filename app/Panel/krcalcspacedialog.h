/*
    SPDX-FileCopyrightText: 2004 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    void closeEvent(QCloseEvent *) override;
    void keyPressEvent(QKeyEvent *e) override;

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
