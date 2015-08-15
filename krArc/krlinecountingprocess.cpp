/***************************************************************************
                           krlinecountingprocess.cpp
                           -------------------------
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

#include "krlinecountingprocess.h"

KrLinecountingProcess::KrLinecountingProcess() : KProcess()
{
    setOutputChannelMode(KProcess::SeparateChannels); // without this output redirection has no effect!
    connect(this, SIGNAL(readyReadStandardError()), SLOT(receivedError()));
    connect(this, SIGNAL(readyReadStandardOutput()), SLOT(receivedOutput()));
    mergedOutput = true;
}

void KrLinecountingProcess::setMerge(bool value)
{
    mergedOutput = value;
}

QString KrLinecountingProcess::getErrorMsg()
{
    if (errorData.trimmed().isEmpty())
        return QString::fromLocal8Bit(outputData);
    else
        return QString::fromLocal8Bit(errorData);
}

void KrLinecountingProcess::receivedError()
{
    QByteArray newData(this->readAllStandardError());
    emit newErrorLines(newData.count('\n'));
    errorData += newData;
    if (errorData.length() > 500)
        errorData = errorData.right(500);
    if (mergedOutput)
        receivedOutput(newData);
}

void KrLinecountingProcess::receivedOutput(QByteArray newData)
{
    if (newData.isEmpty())
        newData = this->readAllStandardOutput();
    emit newOutputLines(newData.count('\n'));
    emit newOutputData(this, newData);
    outputData += newData;
    if (outputData.length() > 500)
        outputData = outputData.right(500);
}

