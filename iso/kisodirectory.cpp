/***************************************************************************
                          kisodirectory.cpp  -  description
                             -------------------
    begin                : Wed Oct 30 2002
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

#include "kisodirectory.h"

KIsoDirectory::KIsoDirectory( KArchive* archive, const QString& name, int access,
    int date, int adate, int cdate, const QString& user, const QString& group,
    const QString& symlink) :
        KArchiveDirectory(archive, name, access, date, user, group, symlink) {
                               

    m_adate=adate;
    m_cdate=cdate;
}

KIsoDirectory::~KIsoDirectory(){
}
