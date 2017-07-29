/***************************************************************************
                           krdebuglogger.h
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

#ifndef KRDEBUGLOGGER_H
#define KRDEBUGLOGGER_H

// QtCore
#include <QFile>
#include <QDir>
#include <QTextStream>

#include <unistd.h>

//! A class to manage some aspects of the writing of messages into the Krusader debug log file
class KrDebugLogger
{
private:
    QString function; //! The name of a function which is going to be written about
    static int indentation;  //! The indentation that is presently used, it represents how many spaces are going to be used
    const static int indentationIncrease; //! The quantity of spaces that are going be added to the indentation when increasing it
    static const QString logFile; //! The name of the log file

public:
    //! This constructor is used inside the KRFUNC macro. For more details: the description of the KRFUNC macro can be seen
    KrDebugLogger(const QString &argFunction, int line);
    //! For more information: the description of the KRFUNC macro can be seen
    ~KrDebugLogger();
    static void prepareWriting(QFile &, QTextStream &);
};

#ifdef QT_DEBUG

//! Writes a function name, etc. in the Krusader debug log when entering the function and automatically before exiting from it
#define KRFUNC \
        KrDebugLogger functionLogger(__FUNCTION__, __LINE__);

#define KRDEBUG(X...) do{   \
        QFile file;     \
        QTextStream stream;     \
        KrDebugLogger::prepareWriting(file, stream);       \
        stream << __FUNCTION__ << "(" <<__LINE__<< "): "; \
        stream << X << endl;      \
    } while(0);
#else
#define KRFUNC
#define KRDEBUG(X...) qDebug() << X
#endif

#endif // KRDEBUGLOGGER_H

