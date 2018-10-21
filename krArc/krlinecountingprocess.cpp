/*****************************************************************************
 * Copyright (C) 2001 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2001 Rafi Yanai <krusader@users.sourceforge.net>            *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#include "krlinecountingprocess.h"

KrLinecountingProcess::KrLinecountingProcess() : KProcess()
{
    setOutputChannelMode(KProcess::SeparateChannels); // without this output redirection has no effect!
    connect(this, &KrLinecountingProcess::readyReadStandardError, this, &KrLinecountingProcess::receivedError);
    connect(this, &KrLinecountingProcess::readyReadStandardOutput, [=]() { receivedOutput(); });
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

