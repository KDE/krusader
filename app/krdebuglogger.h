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

#include "compat.h"
#include <unistd.h>

//! Manages a system aimed to show debug messages
class KrDebugLogger
{
public:
    explicit KrDebugLogger();
    ~KrDebugLogger() = default;

    //! Builds a QString that contains the corresponding indentation and other information that has to be written
    /*!
        \param functionName  The name of the function where this method is called.
        \param line          The line where this method is called.
        \param indentationSymbol  In the case of a function: This QString is used to indicate the user if the function is starting or ending.
        \return The corresponding indentation (a group of spaces) and other information that has to be written in the same line.
    */
    QString indentedCodePoint(const QString &functionName, int line = 0, const QString &indentationSymbol = "") const;
    //! Decreases the indentation that is going to be used next time
    void decreaseIndentation();
    //! Increases the indentation that is going to be used next time
    void increaseIndentation();

private:
    //! The indentation that is presently used, it represents how many spaces are going to be used to indent
    int indentation = 0;
    //! The quantity of spaces that are going be added to the indentation when increasing it
    const int indentationIncrease = 4;
    //! Indicates if debug messages are going to be less detailed, which will be useful e.g. when comparing traces
    bool briefMode = false;
};

// ---------------------------------------------------------------------------------------

//! A class to manage the automatic indentation of debug messages, and their writing when a function starts or ends
class KrDebugFunctionLogger
{
public:
    //! This constructor is used inside the KRFUNC macro. For more details: the description of the KRFUNC macro can be seen
    explicit KrDebugFunctionLogger(const QString &functionName, int line);
    //! For more details: the description of the KRFUNC macro can be seen
    ~KrDebugFunctionLogger();

private:
    //! The name of a function which is going to be written about
    QString functionName;
};

// ---------------------------------------------------------------------------------------

//! An object that manages debug messages in a convenient way
extern KrDebugLogger krDebugLogger;

//! Writes a function name, etc. when entering the function and automatically before exiting from it
/*!
    Inside a function this macro is aimed to be used only once, in its first line.
    Each time that the code of this macro is executed: an object is created in the stack, its
    constructor shows information (useful to know e.g. that the function has started), and it's
    caused that when the object is destroyed (because the function is finished) the destructor shows
    other information (useful to know e.g. that the function has finished)
*/
#define KRFUNC \
    KrDebugFunctionLogger functionLogger(__FUNCTION__, __LINE__);

#define KRDEBUG(...) \
    qDebug().nospace().noquote() << krDebugLogger.indentedCodePoint(__FUNCTION__, __LINE__) << ": " << __VA_ARGS__;

#endif // KRDEBUGLOGGER_H
