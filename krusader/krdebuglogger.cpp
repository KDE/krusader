/***************************************************************************
                           krdebuglogger.cpp
                           ------------------
    copyright            : (C) 2016 by Rafi Yanai & Shie Erlich
    email                : krusader@users.sf.net
    web site             : http://krusader.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "krdebuglogger.h"

int KrDebugLogger::indentation = 1;
const int KrDebugLogger::indentationIncrease = 3;
const QString KrDebugLogger::logFile = QDir::tempPath() + "/krdebug";

KrDebugLogger::KrDebugLogger(const QString &argFunction, int line) : function(argFunction)
{
    QFile file;
    QTextStream stream;
    prepareWriting(file, stream);
    stream << QString("┏"); // Indicates that a function has been started
    stream << function << "(" << line << ")" << endl;
    indentation += indentationIncrease;
}

KrDebugLogger::~KrDebugLogger()
{
    indentation -= indentationIncrease;
    QFile file;
    QTextStream stream;
    prepareWriting(file, stream);
    stream << QString("┗"); // Indicates that a function is going to finish
    stream << function << endl;
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
