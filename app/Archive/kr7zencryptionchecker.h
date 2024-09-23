/*
    SPDX-FileCopyrightText: 2001 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KR7ZENCRYPTIONCHECKER_H
#define KR7ZENCRYPTIONCHECKER_H

#include <signal.h> // for kill
#include <unistd.h> // for setsid, see Kr7zEncryptionChecker::setupChildProcess

#include <KProcess>

/**
 * Used by ArcHandler.
 */
class Kr7zEncryptionChecker : public KProcess
{
    Q_OBJECT

public:
    Kr7zEncryptionChecker();

public slots:
    void receivedOutput();
    bool isEncrypted();

private:
    QString fileName;
    bool encrypted;
    QString lastData;
};

#endif // KR7ZENCRYPTIONCHECKER_H
