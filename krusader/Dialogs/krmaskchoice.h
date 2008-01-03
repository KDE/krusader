/***************************************************************************
                                 krmaskchoice.h
                             -------------------
    copyright            : (C) 2000 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
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
#ifndef KRMASKCHOICE_H
#define KRMASKCHOICE_H

#include <qdialog.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QComboBox;
class Q3GroupBox;
class QLabel;
class Q3ListBox;
class Q3ListBoxItem;
class QPushButton;

class KRMaskChoice : public QDialog
{ 
    Q_OBJECT

public:
    KRMaskChoice( QWidget* parent = 0 );
    ~KRMaskChoice();

    QComboBox* selection;
    QLabel* PixmapLabel1;
    QLabel* label;
    Q3GroupBox* GroupBox1;
    Q3ListBox* preSelections;
    QPushButton* PushButton7;
    QPushButton* PushButton7_2;
    QPushButton* PushButton7_3;
    QPushButton* PushButton3;
    QPushButton* PushButton3_2;

public slots:
    virtual void addSelection();
    virtual void clearSelections();
    virtual void deleteSelection();
    virtual void acceptFromList(Q3ListBoxItem *);

protected:
    QHBoxLayout* hbox;
    QHBoxLayout* hbox_2;
    QHBoxLayout* hbox_3;
    QVBoxLayout* vbox;
};

#endif // KRMASKCHOICE_H
