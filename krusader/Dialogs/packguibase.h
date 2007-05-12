/***************************************************************************
                                 packguibase.h
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
#ifndef PACKGUIBASE_H
#define PACKGUIBASE_H

#include <klocale.h>
#include <qdialog.h>
#include <qmap.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>


class Q3VBoxLayout; 
class Q3HBoxLayout; 
class Q3GridLayout; 
class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QToolButton;
class QSpinBox;
class QSlider;
class KHistoryComboBox;

class PackGUIBase : public QDialog
{ 
    Q_OBJECT

public:
    PackGUIBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );
    ~PackGUIBase();

    QLabel* TextLabel3;
    QLineEdit* nameData;
    QComboBox* typeData;
    QLabel* TextLabel5;
    QLineEdit* dirData;
    QToolButton* browseButton;
    QWidget* advancedWidget;
    QLabel* PixmapLabel1;
    QLabel* TextLabel1;
    QLabel* TextLabel4;
    QLabel* TextLabel6;
    QLabel* TextLabel7;
    QLabel* TextLabel8;
    QLabel* minLabel;
    QLabel* maxLabel;
    QLineEdit* password;
    QLineEdit* passwordAgain;
    QLabel* passwordConsistencyLabel;
    QPushButton* okButton;
    QPushButton* cancelButton;
    QPushButton* advancedButton;
    QCheckBox* encryptHeaders;
    QCheckBox* multipleVolume;
    QSpinBox* volumeSpinBox;
    QComboBox* volumeUnitCombo;
    QCheckBox* setCompressionLevel;
    QSlider*   compressionSlider;
    KHistoryComboBox *commandLineSwitches;

public slots:
    virtual void browse();
    virtual bool extraProperties( QMap<QString,QString> & );

    void expand();
    void checkConsistency();

protected:
    Q3HBoxLayout* hbox;
    Q3HBoxLayout* hbox_2;
    Q3HBoxLayout* hbox_3;
    Q3HBoxLayout* hbox_4;
    Q3GridLayout* hbox_5;
    Q3HBoxLayout* hbox_6;
    Q3HBoxLayout* hbox_7;
    Q3GridLayout* grid;

private:
    bool expanded;
};

#endif // PACKGUIBASE_H
