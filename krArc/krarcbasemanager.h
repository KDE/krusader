/***************************************************************************
                           krarcbasemanager.h
                           ------------------
    copyright            : (C) 2003 by Rafi Yanai & Shie Erlich
    email                : krusader@users.sf.net
    web site             : http://krusader.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KRARCBASEMANAGER_H
#define KRARCBASEMANAGER_H

#include <QtCore/QFile>

/*!
 * \brief An abstract base class for managing archives.
 */
class KrArcBaseManager
{
private:
    //! Information about a type of archive and the bytes that are used to detect it.
    struct AutoDetectParams {
        QString type;
        int location;
        QByteArray detectionString;
    };

    static AutoDetectParams autoDetectParams[]; //! Information used to detect if a file is an archive
    static int autoDetectElems; //!< The size of autoDetectParams[]

public:
    KrArcBaseManager() {}
    QString detectArchive(bool &, QString, bool = true, bool = false);
    virtual void checkIf7zIsEncrypted(bool &, QString) = 0;
    virtual ~KrArcBaseManager() {}
};

#endif // KRARCBASEMANAGER_H

