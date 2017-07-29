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

#include "krservices.h"

// QtCore
#include <QDir>
#include <QSet>
#include <QTextStream>
#include <QtGlobal>

#include <KConfigCore/KSharedConfig>
#include <KIOCore/KProtocolManager>

#include "krglobal.h"
#include "defaults.h"

QMap<QString, QString>* KrServices::slaveMap = 0;
QSet<QString> KrServices::krarcArchiveMimetypes = KrServices::generateKrarcArchiveMimetypes();
#ifdef KRARC_QUERY_ENABLED
QSet<QString> KrServices::isoArchiveMimetypes = QSet<QString>::fromList(KProtocolInfo::archiveMimetypes("iso"));
#else
QSet<QString> KrServices::isoArchiveMimetypes;
#endif

QString KrServices::GLOBAL_MESSAGE_PATTERN = "%{time hh:mm:ss.zzz}-%{type} %{category} %{function}@%{line} # %{message}";

QSet<QString> KrServices::generateKrarcArchiveMimetypes()
{
    // Hard-code these proven mimetypes openable by krarc protocol.
    // They cannot be listed in krarc.protocol itself
    // because it would baffle other file managers (like Dolphin).
    QSet<QString> mimes;
    mimes += QString("application/x-deb");
    mimes += QString("application/x-debian-package");
    mimes += QString("application/vnd.debian.binary-package");
    mimes += QString("application/x-java-archive");
    mimes += QString("application/x-rpm");
    mimes += QString("application/x-source-rpm");
    mimes += QString("application/vnd.oasis.opendocument.chart");
    mimes += QString("application/vnd.oasis.opendocument.database");
    mimes += QString("application/vnd.oasis.opendocument.formula");
    mimes += QString("application/vnd.oasis.opendocument.graphics");
    mimes += QString("application/vnd.oasis.opendocument.presentation");
    mimes += QString("application/vnd.oasis.opendocument.spreadsheet");
    mimes += QString("application/vnd.oasis.opendocument.text");
    mimes += QString("application/vnd.openxmlformats-officedocument.presentationml.presentation");
    mimes += QString("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
    mimes += QString("application/vnd.openxmlformats-officedocument.wordprocessingml.document");
    mimes += QString("application/x-cbz");
    mimes += QString("application/x-cbr");
    mimes += QString("application/epub+zip");
    mimes += QString("application/x-webarchive");
    mimes += QString("application/x-plasma");
    mimes += QString("application/vnd.rar");

    #ifdef KRARC_QUERY_ENABLED
    mimes += QSet<QString>::fromList(KProtocolInfo::archiveMimetypes("krarc"));
    #endif

    return mimes;
}

bool KrServices::cmdExist(QString cmdName)
{
    KConfigGroup group(krConfig, "Dependencies");
    if (QFile(group.readEntry(cmdName, QString())).exists())
        return true;

    return !QStandardPaths::findExecutable(cmdName).isEmpty();
}

QString KrServices::fullPathName(QString name, QString confName)
{
    QString supposedName;

    if (confName.isNull())
        confName = name;

    KConfigGroup config(krConfig, "Dependencies");
    if (QFile(supposedName = config.readEntry(confName, QString())).exists())
        return supposedName;

    if ((supposedName = QStandardPaths::findExecutable(name)).isEmpty())
        return "";

    config.writeEntry(confName, supposedName);
    return supposedName;
}

QString KrServices::chooseFullPathName(QStringList names, QString confName)
{
    foreach(const QString &name, names) {
        QString foundTool = KrServices::fullPathName(name, confName);
        if (! foundTool.isEmpty()) {
            return foundTool;
        }
    }

    return "";
}

bool KrServices::isExecutable(const QString &path)
{
    QFileInfo info(path);
    return info.isFile() && info.isExecutable();
}

QString KrServices::registeredProtocol(QString mimetype)
{
    if (slaveMap == 0) {
        slaveMap = new QMap<QString, QString>();

        KConfigGroup group(krConfig, "Protocols");
        QStringList protList = group.readEntry("Handled Protocols", QStringList());
        for (QStringList::Iterator it = protList.begin(); it != protList.end(); ++it) {
            QStringList mimes = group.readEntry(QString("Mimes For %1").arg(*it), QStringList());
            for (QStringList::Iterator it2 = mimes.begin(); it2 != mimes.end(); ++it2)
                (*slaveMap)[*it2] = *it;
        }
    }
    QString protocol = (*slaveMap)[mimetype];
    if (protocol.isEmpty()) {
        if (krarcArchiveMimetypes.contains(mimetype)) {
            return "krarc";
        }
        protocol = KProtocolManager::protocolForArchiveMimetype(mimetype);
    }
    return protocol;
}

bool KrServices::isoSupported(QString mimetype)
{
    return isoArchiveMimetypes.contains(mimetype);
}

void KrServices::clearProtocolCache()
{
    if (slaveMap)
        delete slaveMap;
    slaveMap = 0;
}

bool KrServices::fileToStringList(QTextStream *stream, QStringList& target, bool keepEmptyLines)
{
    if (!stream) return false;
    QString line;
    while (!stream->atEnd()) {
        line = stream->readLine().trimmed();
        if (keepEmptyLines || !line.isEmpty()) target.append(line);
    }
    return true;
}

bool KrServices::fileToStringList(QFile *file, QStringList& target, bool keepEmptyLines)
{
    QTextStream stream(file);
    return fileToStringList(&stream, target, keepEmptyLines);
}

QString KrServices::quote(QString name)
{
    if (!name.contains('\''))
        return '\'' + name + '\'';
    if (!name.contains('"') && !name.contains('$'))
        return '\"' + name + '\"';
    return escape(name);
}

QStringList KrServices::quote(const QStringList& names)
{
    QStringList result;
    for (int i = 0; i < names.size(); ++i)
        result.append(quote(names[i]));
    return result;
}

QList<QUrl> KrServices::toUrlList(const QStringList &list)
{
    QList<QUrl> result;
    for (QStringList::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
        result.append(QUrl::fromUserInput(*it, QDir::currentPath(), QUrl::AssumeLocalFile));
    }
    return result;
}

QStringList KrServices::toStringList(const QList<QUrl> &list)
{
    QStringList result;
    for(QList<QUrl>::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
        result.append(it->toString());
    }
    return result;
}

// Adds one tool to the list in the supportedTools method
void supportedTool(QStringList &tools, QString toolType,
                   QStringList names, QString confName) {
    QString foundTool = KrServices::chooseFullPathName(names, confName);
    if (! foundTool.isEmpty()) {
        tools.append(toolType);
        tools.append(foundTool);
    }
}

// return a list in the format of TOOLS,PATH. for example
// DIFF,kdiff,TERMINAL,konsole,...
//
// currently supported tools: DIFF, MAIL, RENAME
//
// to use it: QStringList lst = supportedTools();
//            int i = lst.indexOf("DIFF");
//            if (i!=-1) pathToDiff=lst[i+1];
QStringList KrServices::supportedTools() {
    QStringList tools;

    // first, a diff program: kdiff
    supportedTool(tools, "DIFF",
                  QStringList() << "kdiff3" << "kompare" << "xxdiff",
                  "diff utility");

    // a mailer: kmail or thunderbird
    supportedTool(tools, "MAIL",
                  QStringList() << "thunderbird" << "kmail",
                  "mailer");

    // rename tool: krename
    supportedTool(tools, "RENAME",
                  QStringList() << "krename",
                  "krename");

    // checksum utility
    supportedTool(tools, "MD5",
                  QStringList() << "md5sum",
                  "checksum utility");

    return tools;
}


QString KrServices::escape(QString name)
{
    const QString evilstuff = "\\\"'`()[]{}!?;$&<>| \t\r\n";  // stuff that should get escaped

    for (int i = 0; i < evilstuff.length(); ++i)
        name.replace(evilstuff[ i ], ('\\' + evilstuff[ i ]));

    return name;
}

QString KrServices::escapeFileUrl(QString urlString)
{
    // Avoid that if a path contains a '#' then what follows the '#' be interpreted as the fragment identifier of
    // the URL and not a part of the file path; for more information https://bugs.kde.org/show_bug.cgi?id=270150 can be seen
    return urlString.replace('#', "%23").replace('?', "%3F");
}

QUrl KrServices::escapeFileUrl(const QUrl &url)
{
    return QUrl(KrServices::escapeFileUrl(url.toString()));
}

QString KrServices::urlToLocalPath(const QUrl &url)
{
    QUrl fileUrl = QUrl(url);
    // QUrl::toLocalFile() does not work if the protocol is "file" e.g. when opening an archive
    fileUrl.setScheme("file");
    QString path = fileUrl.toLocalFile();
    REPLACE_DIR_SEP2(path);

#ifdef Q_WS_WIN
    if (path.startsWith(DIR_SEPARATOR)) {
        int p = 1;
        while (p < path.length() && path[ p ] == DIR_SEPARATOR_CHAR)
            p++;
        /* /C:/Folder */
        if (p + 2 <= path.length() && path[ p ].isLetter() && path[ p + 1 ] == ':') {
            path = path.mid(p);
        }
    }
#endif
    return path;
}

static bool s_withDebugMessages;
static QtMessageHandler s_defaultMessageHandler;

void KrServices::setGlobalKrMessageHandler(bool withDebugMessages)
{
    s_withDebugMessages = withDebugMessages;
    s_defaultMessageHandler = qInstallMessageHandler(0);
    qInstallMessageHandler(&krMessageHandler);
}

void KrServices::krMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // filter debug if not enabled
    if (type != QtDebugMsg || s_withDebugMessages) {
        s_defaultMessageHandler(type, context, msg);
    }
}
