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

#include "kisodebug.h"

int KIsoDebug::indentation = 1;
const int KIsoDebug::indentationIncrease = 3;
const QString KIsoDebug::logFile = QDir::tempPath() + "/kisodebug";

//! This constructor is used inside the KRFUNC macro. For more details: the description of the KRFUNC macro can be seen
KIsoDebug::KIsoDebug(const QString &argFunction, int line) : function(argFunction)
{
    QFile file;
    QTextStream stream;
    prepareWriting(file, stream);
    stream << QString("┏"); // Indicates that a function has been started
    stream << function << "(" << line << ")" << endl;
    indentation += indentationIncrease;
}

//! For more information: the description of the KRFUNC macro can be seen
KIsoDebug::~KIsoDebug()
{
    indentation -= indentationIncrease;
    QFile file;
    QTextStream stream;
    prepareWriting(file, stream);
    stream << QString("┗"); // Indicates that a function is going to finish
    stream << function << endl;
}

//! Prepares some elements before a writing into the kiso debug log file
void KIsoDebug::prepareWriting(QFile &file, QTextStream &stream)
{
    file.setFileName(logFile);
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    stream.setDevice(&file);
    stream << "Pid:" << (int)getpid();
    // Applies the indentation level to make logs clearer
    for (int x = 0; x < indentation; ++x)
        stream << " ";
}
