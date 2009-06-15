/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#ifndef KRSERVICES_H
#define KRSERVICES_H

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <qmap.h>
#include <kurl.h>

class QTextStream;
class QFile;

class KrServices
{
public:
    KrServices() {}
    ~KrServices() {}

    static bool         cmdExist(QString cmdName);
    static QString      chooseFullPathName(QStringList names, QString confName);
    static QString      detectFullPathName(QString name);
    static QString      fullPathName(QString name, QString confName = QString());
    static QStringList  separateArgs(QString args);
    static QString      registerdProtocol(QString mimetype);
    static QString      getPath(const KUrl &url, KUrl::AdjustPathOption trailing = KUrl::LeaveTrailingSlash);
    static void         clearProtocolCache();
    static bool         fileToStringList(QTextStream *stream, QStringList& target, bool keepEmptyLines = false);
    static bool         fileToStringList(QFile *file, QStringList& target, bool keepEmptyLines = false);
    static QString    quote(QString name);
    static QStringList  quote(const QStringList& names);

protected:
    static QString    escape(QString name);

private:
    static QMap<QString, QString>* slaveMap;

};

// TODO: make KrServices a namespace and move it there

#endif
