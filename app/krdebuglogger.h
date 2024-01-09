/*
    SPDX-FileCopyrightText: 2016 Rafi Yanai <krusader@users.sf.net>
    SPDX-FileCopyrightText: 2016 Shie Erlich <krusader@users.sf.net>
    SPDX-FileCopyrightText: 2016-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRDEBUGLOGGER_H
#define KRDEBUGLOGGER_H

// QtCore
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTextStream>

#include "compat.h"
#include <unistd.h>

//! A class to manage some aspects of the writing of messages into the Krusader debug log file
class KrDebugLogger
{
private:
    QString function; //! The name of a function which is going to be written about
    static int indentation; //! The indentation that is presently used, it represents how many spaces are going to be used
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
#define KRFUNC KrDebugLogger functionLogger(__FUNCTION__, __LINE__);

#define KRDEBUG(...)                                                                                                                                           \
    do {                                                                                                                                                       \
        QFile file;                                                                                                                                            \
        QTextStream stream;                                                                                                                                    \
        KrDebugLogger::prepareWriting(file, stream);                                                                                                           \
        stream << __FUNCTION__ << "(" << __LINE__ << "): ";                                                                                                    \
        stream << __VA_ARGS__ << QT_ENDL; /* Like on https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html */                                                \
    } while (0);
#else
#define KRFUNC
#define KRDEBUG(...) qDebug() << __VA_ARGS__;
#endif

#endif // KRDEBUGLOGGER_H
