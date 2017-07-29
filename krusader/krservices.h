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

// QtCore
#include <QString>
#include <QStringList>
#include <QMap>
#include <QUrl>

class QTextStream;
class QFile;

/**
 * Global static utility functions.
 */
class KrServices
{
public:
    static bool         cmdExist(QString cmdName);
    static QString      chooseFullPathName(QStringList names, QString confName);
    static QString      fullPathName(QString name, QString confName = QString());
    static bool         isExecutable(const QString &path);
    static QString      registeredProtocol(QString mimetype);
    static bool         isoSupported(QString mimetype);
    static QString      urlToLocalPath(const QUrl &url);
    static void         clearProtocolCache();
    static bool         fileToStringList(QTextStream *stream, QStringList& target, bool keepEmptyLines = false);
    static bool         fileToStringList(QFile *file, QStringList& target, bool keepEmptyLines = false);
    static QString      quote(QString name);
    static QStringList  quote(const QStringList& names);
    static QList<QUrl>  toUrlList(const QStringList &list);
    static QStringList  toStringList(const QList<QUrl> &list);
    static QStringList  supportedTools(); // find supported tools
    static QString      escapeFileUrl(QString urlString);
    static QUrl         escapeFileUrl(const QUrl &url);
    /**
     * Sets the global logging message handler for qDebug(), qWarning()... messages to a custom one
     * with the ability to filter debug messages.
     */
    static void setGlobalKrMessageHandler(bool withDebugMessages);
    static QString GLOBAL_MESSAGE_PATTERN;

protected:
    static QString    escape(QString name);

private:
    KrServices() {}
    ~KrServices() {}
    static QSet<QString> generateKrarcArchiveMimetypes();
    static QMap<QString, QString>* slaveMap;
    static QSet<QString> krarcArchiveMimetypes;
    static QSet<QString> isoArchiveMimetypes;
    static void krMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
};

// TODO: make KrServices a namespace and move it there

#endif
