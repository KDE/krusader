/***************************************************************************
                          qfilehack.h  -  description
                             -------------------
    begin                : Tue Oct 29 2002
    copyright            : (C) 2002 by Szombathelyi György
    email                : gyurco@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QFILEHACK_H
#define QFILEHACK_H

#include <qfile.h>
#include <qstring.h>

/**
  *@author Szombathelyi György
  * Qt thinks if a file is not S_IFREG, you cannot seek in it. It's false (what about
  * block devices for example?
  */

class QFileHack : public QFile  {
public: 
    QFileHack();
    QFileHack( const QString & name );
    ~QFileHack();
    virtual bool open ( int m );
};

#endif
