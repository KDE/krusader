/***************************************************************************
                                bookmaneditbase.h
                             -------------------
    begin                : Thu May 4 2000
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
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

/****************************************************************************
** Form interface generated from reading ui file 'bookmaneditbase.ui'
**
** Created: Fri Aug 11 18:22:56 2000
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef BOOKMANEDITBASE_H
#define BOOKMANEDITBASE_H

#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QLabel;
class QLineEdit;
class QPushButton;
class QToolButton;

class BookManEditBase : public QDialog
{ 
    Q_OBJECT

public:
    BookManEditBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~BookManEditBase();

    QPushButton* okButton;
    QPushButton* cancelButton;
    QLabel* TextLabel1;
    QLineEdit* nameData;
    QToolButton* clearButton;
    QLabel* TextLabel2;
    QLineEdit* urlData;
    QToolButton* browseButton;

public slots:
    virtual void browse();

protected:
    QHBoxLayout* hbox;
    QHBoxLayout* hbox_2;
    QHBoxLayout* hbox_3;
    QGridLayout* grid;
};

#endif // BOOKMANEDITBASE_H
