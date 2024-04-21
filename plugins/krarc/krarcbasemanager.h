/*
    SPDX-FileCopyrightText: 2003 Rafi Yanai <krusader@users.sf.net>
    SPDX-FileCopyrightText: 2003 Shie Erlich <krusader@users.sf.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRARCBASEMANAGER_H
#define KRARCBASEMANAGER_H

#include <KConfig>
#include <KConfigGroup>

// QtCore
#include <QFile>

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

protected:
    //! The maximum length of a short QString that represents the type of a file
    static const int maxLenType;
    //! The configuration file for Krusader
    KConfig krConf;
    //! The 'Dependencies' config group
    KConfigGroup dependGrp;

    //! Search for the full path to a program
    QString fullPathName(const QString &name);
    //! Find the path to a 7z (or 7za) executable
    QString find7zExecutable();

    static bool checkStatus(const QString &, int);

public:
    KrArcBaseManager();
    QString detectArchive(bool &, const QString &, bool = true, bool = false);
    virtual void checkIf7zIsEncrypted(bool &, QString) = 0;
    static QString getShortTypeFromMime(const QString &);
    virtual ~KrArcBaseManager()
    {
    }
};

#endif // KRARCBASEMANAGER_H
