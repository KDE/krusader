/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krservices.h"

// QtCore
#include <QDir>
#include <QSet>
#include <QTextStream>
#include <QtGlobal>

#include <KProtocolManager>
#include <KSharedConfig>
#include <utility>

#include "defaults.h"
#include "krglobal.h"

QString KrServices::GLOBAL_MESSAGE_PATTERN = "%{time hh:mm:ss.zzz}-%{type} %{category} %{function}@%{line} # %{message}";

bool KrServices::cmdExist(const QString &cmdName)
{
    // Reminder: If that function is modified, it's important to research if the
    // changes must also be applied to `KrServices::fullPathName()`
    // and `kio_krarcProtocol::fullPathName()`

    KConfigGroup dependGrp(krConfig, "Dependencies");
    QString supposedName = dependGrp.readEntry(cmdName, QString());
    if (QFileInfo::exists(supposedName))
        return true;

    if ((supposedName = QStandardPaths::findExecutable(cmdName)).isEmpty())
        return false;

    // Because an executable file has been found, its path is remembered
    // in order to avoid some future searches
    dependGrp.writeEntry(cmdName, supposedName);

    return true;
}

QString KrServices::fullPathName(const QString &name, QString confName)
{
    // Reminder: If that function is modified, it's important to research if the
    // changes must also be applied to `kio_krarcProtocol::fullPathName()`
    // and `KrServices::cmdExist()`

    if (confName.isNull())
        confName = name;

    KConfigGroup dependGrp(krConfig, "Dependencies");
    QString supposedName = dependGrp.readEntry(confName, QString());
    if (QFileInfo::exists(supposedName))
        return supposedName;

    if ((supposedName = QStandardPaths::findExecutable(name)).isEmpty())
        return QString();

    // Because an executable file has been found, its path is remembered
    // in order to avoid some future searches
    dependGrp.writeEntry(confName, supposedName);

    return supposedName;
}

QString KrServices::chooseFullPathName(QStringList names, const QString &confName)
{
    for (const QString &name : std::as_const(names)) {
        QString foundTool = KrServices::fullPathName(name, confName);
        if (!foundTool.isEmpty()) {
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

bool KrServices::isoSupported(const QString &mimetype)
{
#ifdef KRARC_QUERY_ENABLED
    return KProtocolInfo::archiveMimetypes("iso").contains(mimetype);
#else
    return false;
#endif
}

bool KrServices::fileToStringList(QTextStream *stream, QStringList &target, bool keepEmptyLines)
{
    if (!stream)
        return false;
    QString line;
    while (!stream->atEnd()) {
        line = stream->readLine().trimmed();
        if (keepEmptyLines || !line.isEmpty())
            target.append(line);
    }
    return true;
}

bool KrServices::fileToStringList(QFile *file, QStringList &target, bool keepEmptyLines)
{
    QTextStream stream(file);
    return fileToStringList(&stream, target, keepEmptyLines);
}

QString KrServices::quote(const QString &name)
{
    if (!name.contains('\''))
        return '\'' + name + '\'';
    if (!name.contains('"') && !name.contains('$'))
        return '\"' + name + '\"';
    return escape(name);
}

QStringList KrServices::quote(const QStringList &names)
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
    for (QList<QUrl>::ConstIterator it = list.constBegin(); it != list.constEnd(); ++it) {
        result.append(it->toString());
    }
    return result;
}

// Adds one tool to the list in the supportedTools method
void supportedTool(QStringList &tools, const QString &toolType, QStringList names, QString confName)
{
    QString foundTool = KrServices::chooseFullPathName(std::move(names), std::move(confName));
    if (!foundTool.isEmpty()) {
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
QStringList KrServices::supportedTools()
{
    QStringList tools;

    // first, a diff program: kdiff
    supportedTool(tools,
                  "DIFF",
                  QStringList() << "kdiff3"
                                << "kompare"
                                << "xxdiff",
                  "diff utility");

    // a mailer: kmail or thunderbird
    supportedTool(tools,
                  "MAIL",
                  QStringList() << "thunderbird"
                                << "kmail",
                  "mailer");

    // rename tool: krename
    supportedTool(tools, "RENAME", QStringList() << "krename", "krename");

    // checksum utility
    supportedTool(tools, "MD5", QStringList() << "md5sum", "checksum utility");

    return tools;
}

QString KrServices::escape(QString name)
{
    const QString evilstuff = "\\\"'`()[]{}!?;$&<>| \t\r\n"; // stuff that should get escaped

    for (auto i : evilstuff)
        name.replace(i, ('\\' + i));

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

#ifdef Q_OS_WIN
    if (path.startsWith(DIR_SEPARATOR)) {
        int p = 1;
        while (p < path.length() && path[p] == DIR_SEPARATOR_CHAR)
            p++;
        /* /C:/Folder */
        if (p + 2 <= path.length() && path[p].isLetter() && path[p + 1] == ':') {
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
    s_defaultMessageHandler = qInstallMessageHandler(nullptr);
    qInstallMessageHandler(&krMessageHandler);
}

void KrServices::krMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // filter debug if not enabled
    if (type != QtDebugMsg || s_withDebugMessages) {
        s_defaultMessageHandler(type, context, msg);
    }
}
