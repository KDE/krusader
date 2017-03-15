/***************************************************************************
                          krcalcspacedialog.h  -  description
                             -------------------
    begin                : Fri Jan 2 2004
    copyright            : (C) 2004 by Shie Erlich & Rafi Yanai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     H e a d e r    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
