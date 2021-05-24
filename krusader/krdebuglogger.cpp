/*
    SPDX-FileCopyrightText: 2016 Rafi Yanai <krusader@users.sf.net>
    SPDX-FileCopyrightText: 2016 Shie Erlich <krusader@users.sf.net>
    SPDX-FileCopyrightText: 2016-2020 Krusader Krew [https://krusader.org]

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krdebuglogger.h"
#include "compat.h"

int KrDebugLogger::indentation = 1;
const int KrDebugLogger::indentationIncrease = 3;
const QString KrDebugLogger::logFile = QDir::tempPath() + "/krdebug";

KrDebugLogger::KrDebugLogger(const QString &argFunction, int line) : function(argFunction)
{
    QFile file;
    QTextStream stream;
    prepareWriting(file, stream);
    stream << QString("┏"); // Indicates that a function has been started
    stream << function << "(" << line << ")" << QT_ENDL;
    indentation += indentationIncrease;
}

KrDebugLogger::~KrDebugLogger()
{
    indentation -= indentationIncrease;
    QFile file;
    QTextStream stream;
    prepareWriting(file, stream);
    stream << QString("┗"); // Indicates that a function is going to finish
    stream << function << QT_ENDL;
}

//! Prepares some elements before a writing into the krarc debug log file
void KrDebugLogger::prepareWriting(QFile &file, QTextStream &stream)
{
    file.setFileName(logFile);
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    stream.setDevice(&file);
    stream << "Pid:" << (int)getpid();
    // Applies the indentation level to make logs clearer
    for (int x = 0; x < indentation; ++x)
        stream << " ";
}
