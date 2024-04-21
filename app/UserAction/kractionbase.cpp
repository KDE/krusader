/*
    SPDX-FileCopyrightText: 2006 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kractionbase.h"

// QtWidgets
#include <QInputDialog>

#include <KLocalizedString>
#include <KMessageBox>

#include "../krglobal.h"
#include "kraction.h"

KrActionBase::~KrActionBase() = default;

void KrActionBase::exec()
{
    KrActionProc *proc = nullptr;

    // replace %% and prepare string
    QStringList commandList;
    if (doSubstitution()) {
        Expander exp;
        exp.expand(command(), acceptURLs());
        if (exp.error()) {
            handleError(exp.error());
            return;
        }
        commandList = exp.result();
    } else
        commandList << command();
    // TODO: query expander for status and may skip the rest of the function

    // stop here if the commandline is empty
    if (commandList.count() == 1 && commandList[0].trimmed().isEmpty())
        return;

    if (confirmExecution()) {
        for (auto &it : commandList) {
            bool exec = true;
            it = QInputDialog::getText(krMainWindow, i18n("Confirm Execution"), i18n("Command being executed:"), QLineEdit::Normal, it, &exec);
            if (exec) {
                proc = actionProcFactoryMethod();
                proc->start(it);
            }
        } // for
    } // if ( _properties->confirmExecution() )
    else {
        proc = actionProcFactoryMethod();
        proc->start(commandList);
    }
}

void KrActionBase::handleError(const Error &err)
{
    // once qtHandler is instantiated, it keeps on showing all qDebug() messages
    // QErrorMessage::qtHandler()->showMessage(err.what());
    const QString &errorMessage = err.description();
    if (!errorMessage.isEmpty()) {
        KMessageBox::error(krMainWindow, errorMessage);
    }
}

KrActionProc *KrActionBase::actionProcFactoryMethod()
{
    return new KrActionProc(this);
}
