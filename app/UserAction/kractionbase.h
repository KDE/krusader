/*
    SPDX-FileCopyrightText: 2006 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRACTIONBASE_H
#define KRACTIONBASE_H

// QtCore
#include <QString>

#include "expander.h"

class KrActionProc;

class KrActionBase
{
public:
    KrActionBase()
    {
    }
    virtual ~KrActionBase();

    /** \brief Specifies the mode for executing the action */
    enum ExecType {
        Normal, ///< Run the command freely
        Terminal, ///< Run the command in new terminal window
        CollectOutput, ///< Collect output from this command
        CollectOutputSeparateStderr, ///< Like #CollectOutput, but display stderr output separately
        RunInTE ///< Run in built-in terminal emulator
    };

    /** \brief Command which runs this action
     *
     * The string of the command may contain placeholders
     * which are parsed by the #Expander class, unless #doSubstitution
     * returns false
     *
     * The command is run by the shell, which should be bash (see #Expander)
     *
     * @see Expander
     * @see doSubstitution
     *
     * @return The command to execute
     */
    virtual QString command() const = 0;
    /** \brief Execution type of the action
     *
     * @see #ExecType
     */
    virtual ExecType execType() const = 0;
    /** \brief Working directory of the command
     *
     * @return The working directory of the command. May be \a null,
     *   in which case the command is executed in current directory
     */
    virtual QString startpath() const = 0;
    /** \brief Username under which the command is run
     *
     * @return The username of the command. May be \a null,
     *   in which case the command is executed under the current user
     */
    virtual QString user() const = 0;
    /** \brief Name of the action
     *
     * @return The name of the action which will be shown to the user
     *   eg. any string will do
     */
    virtual QString text() const = 0;
    /** \brief Does the command accept URLs as filenames (like KDE apps)?
     *
     * @return \a true iff it does
     */
    virtual bool acceptURLs() const = 0;
    /** \brief Confirm execution of this action by the user?
     *
     * @return \a true iff execution should be confirmed
     */
    virtual bool confirmExecution() const = 0;
    /** \brief Can #command contain placeholders?
     *
     * @return \a true iff #command should be expanded by #Expander
     */
    virtual bool doSubstitution() const = 0;
    /** \brief A factory method for creating KrActionProc
     *
     * @return A new instance of KrActionProc
     */
    virtual KrActionProc *actionProcFactoryMethod();
    virtual void handleError(const Error &err);

    void exec();
};

#endif
