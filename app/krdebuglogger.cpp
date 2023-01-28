/*
    SPDX-FileCopyrightText: 2016 Rafi Yanai <krusader@users.sf.net>
    SPDX-FileCopyrightText: 2016 Shie Erlich <krusader@users.sf.net>
    SPDX-FileCopyrightText: 2016-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krdebuglogger.h"
#include "compat.h"

#include <QStringBuilder>

KrDebugLogger krDebugLogger;

KrDebugLogger::KrDebugLogger()
{
    // Sets the level of detail/verbosity
    const QByteArray krDebugBrief = qgetenv("KRDEBUG_BRIEF").toLower();
    briefMode = (krDebugBrief == "true" || krDebugBrief == "yes" || krDebugBrief == "on" || krDebugBrief == "1");
}

QString KrDebugLogger::indentedCodePoint(const QString &functionName, int line, const QString &indentationSymbol) const
{
    QString result = QString(indentation, ' ') %  // Applies the indentation level to make logs clearer
                     indentationSymbol % functionName;  // Uses QStringBuilder to concatenate
    if (!briefMode)
        result = QString("Pid:%1 ").arg(getpid()) %
                result %
                (line != 0 ? QString("(%1)").arg(line) : "");
    return result;
}

void KrDebugLogger::decreaseIndentation()
{
    indentation -= indentationIncrease;
}

void KrDebugLogger::increaseIndentation()
{
    indentation += indentationIncrease;
}

// ---------------------------------------------------------------------------------------
// Member functions of the KrDebugFunctionLogger class
// ---------------------------------------------------------------------------------------

KrDebugFunctionLogger::KrDebugFunctionLogger(const QString &functionName, int line) :
    functionName(functionName)
{
    // Shows that a function has been started
    qDebug().nospace().noquote() << krDebugLogger.indentedCodePoint(functionName, line, "┏");

    krDebugLogger.increaseIndentation();
}

KrDebugFunctionLogger::~KrDebugFunctionLogger()
{
    krDebugLogger.decreaseIndentation();
    // Shows that a function is going to finish
    qDebug().nospace().noquote() << krDebugLogger.indentedCodePoint(functionName, 0, "┗");
}
