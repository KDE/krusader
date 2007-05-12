/***************************************************************************
                          qfilehack.cpp  -  description
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

#include "qfilehack.h"

QFileHack::QFileHack(){
}

QFileHack::QFileHack( const QString & name ) : QFile(name) {
}

QFileHack::~QFileHack(){
}

bool QFileHack::open ( int m ) {
    bool ret;

#ifdef __linux__
    m |= IO_Async; //On linux, set O_NONBLOCK, opens CD-ROMs faster
#endif
    ret=QFile::open(m);
    if (ret && isSequential() ) {
        setType(IO_Direct);
    }
    return ret;
}
