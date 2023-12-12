/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRSERVICES_H
#define KRSERVICES_H

// QtCore
#include <QMap>
#include <QString>
#include <QStringList>
#include <QUrl>

class QTextStream;
class QFile;

/**
 * Global static utility functions.
 */
class KrServices
{
public:
    static bool cmdExist(const QString &cmdName);
    static QString chooseFullPathName(QStringList names, const QString &confName);
    static QString fullPathName(const QString &name, QString confName = QString());
    static bool isExecutable(const QString &path);
    static bool isoSupported(const QString &mimetype);
    static QString urlToLocalPath(const QUrl &url);
    static bool fileToStringList(QTextStream *stream, QStringList &target, bool keepEmptyLines = false);
    static bool fileToStringList(QFile *file, QStringList &target, bool keepEmptyLines = false);
    static QString quote(const QString &name);
    static QStringList quote(const QStringList &names);
    static QList<QUrl> toUrlList(const QStringList &list);
    static QStringList toStringList(const QList<QUrl> &list);
    static QStringList supportedTools(); // find supported tools
    static QString escapeFileUrl(QString urlString);
    static QUrl escapeFileUrl(const QUrl &url);
    /**
     * Sets the global logging message handler for qDebug(), qWarning()... messages to a custom one
     * with the ability to filter debug messages.
     */
    static void setGlobalKrMessageHandler(bool withDebugMessages);
    static QString GLOBAL_MESSAGE_PATTERN;

protected:
    static QString escape(QString name);

private:
    KrServices()
    {
    }
    ~KrServices()
    {
    }
    static void krMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
};

// TODO: make KrServices a namespace and move it there

#endif
