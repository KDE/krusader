/***************************************************************************
                           kr7zencryptionchecker.h
                           -----------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KR7ZENCRYPTIONCHECKER_H
#define KR7ZENCRYPTIONCHECKER_H

#include <unistd.h> // for setsid, see Kr7zEncryptionChecker::setupChildProcess
#include <signal.h> // for kill

#include <KCoreAddons/KProcess>

#include "../krservices.h"

class Kr7zEncryptionChecker : public KProcess
{
    Q_OBJECT

public:
    Kr7zEncryptionChecker();

protected:
    virtual void setupChildProcess() Q_DECL_OVERRIDE;

public slots:
    void receivedOutput();
    bool isEncrypted();

private:
    QString fileName;
    bool encrypted;
    QString lastData;
};

#endif // KR7ZENCRYPTIONCHECKER_H
