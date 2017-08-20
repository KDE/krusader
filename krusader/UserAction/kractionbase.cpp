/*****************************************************************************
 * Copyright (C) 2006 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2006 Rafi Yanai <yanai@users.sourceforge.net>               *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#include "kractionbase.h"

// QtWidgets
#include <QInputDialog>

#include <KI18n/KLocalizedString>
#include <KWidgetsAddons/KMessageBox>

#include "kraction.h"
#include "../krglobal.h"

KrActionBase::~KrActionBase()
{
}

void KrActionBase::exec()
{
    KrActionProc *proc = 0;

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
    //TODO: query expander for status and may skip the rest of the function

    // stop here if the commandline is empty
    if (commandList.count() == 1 && commandList[0].trimmed().isEmpty())
        return;

    if (confirmExecution()) {
        for (QStringList::iterator it = commandList.begin(); it != commandList.end(); ++it) {
            bool exec = true;
            *it = QInputDialog::getText(krMainWindow, i18n("Confirm Execution"), i18n("Command being executed:"),
                      QLineEdit::Normal, *it, &exec);
            if (exec) {
                proc = actionProcFactoryMethod();
                proc->start(*it);
            }
        } //for
    } // if ( _properties->confirmExecution() )
    else {
        proc = actionProcFactoryMethod();
        proc->start(commandList);
    }

}

void KrActionBase::handleError(const Error& err)
{
    // once qtHandler is instantiated, it keeps on showing all qDebug() messages
    //QErrorMessage::qtHandler()->showMessage(err.what());
    const QString errorMessage = err.description();
    if (!errorMessage.isEmpty()) {
        KMessageBox::error(krMainWindow, errorMessage);
    }
}

KrActionProc* KrActionBase::actionProcFactoryMethod()
{
    return new KrActionProc(this);
}
