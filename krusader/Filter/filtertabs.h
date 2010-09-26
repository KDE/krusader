/***************************************************************************
                         filtertab.h  -  description
                             -------------------
    copyright            : (C) 2005 + by Csaba Karai
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

#ifndef FILTERTABS_H
#define FILTERTABS_H

#include <QtCore/QList>

#include <KTabWidget>

#include "filterbase.h"

class FilterTabs : public QObject
{
    Q_OBJECT

public:

    enum {
        HasRemoteContentSearch  =   0x0800,
        HasProfileHandler       =   0x1000,
        HasRecurseOptions       =   0x2000,
        HasSearchIn             =   0x4000,
        HasDontSearchIn         =   0x8000,

        Default                 =   0xe000
    };

    static FilterTabs * addTo(KTabWidget *tabWidget, int props = FilterTabs::Default,
                              QStringList extraOptions = QStringList());
    static KRQuery      getQuery(QWidget *parent = 0);

    FilterBase *get(QString name);
    bool isExtraOptionChecked(QString name);
    void checkExtraOption(QString name, bool check);

public slots:
    void  loadFromProfile(QString);
    void  saveToProfile(QString);
    bool  fillQuery(KRQuery *query);
    void  close(bool accept = true) {
        emit closeRequest(accept);
    }

signals:
    void  closeRequest(bool accept = true);

private:
    FilterTabs(int properties, KTabWidget *tabWidget, QObject *parent, QStringList extraOptions);
    void  acceptQuery();

    QList<FilterBase *> filterList;
    QList<int>      pageNumbers;

    KTabWidget * tabWidget;
};

#endif /* FILTERTABS_H */
