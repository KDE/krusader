/*
    SPDX-FileCopyrightText: 2001 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krlinecountingprocess.h"

KrLinecountingProcess::KrLinecountingProcess()
{
    setOutputChannelMode(KProcess::SeparateChannels); // without this output redirection has no effect!
    connect(this, &KrLinecountingProcess::readyReadStandardError, this, &KrLinecountingProcess::receivedError);
    connect(this, &KrLinecountingProcess::readyReadStandardOutput, [=]() {
        receivedOutput();
    });
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
