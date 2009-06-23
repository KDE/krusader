/*****************************************************************************
 * Copyright (C) 2004 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2004 Rafi Yanai <yanai@users.sourceforge.net>               *
 * Copyright (C) 2006 Jonas Bähr <jonas.baehr@web.de>                        *
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

#ifndef KRACTION_H
#define KRACTION_H

#include <QtCore/QByteArray>

#include <KAction>
#include <KProcess>
#include <KDialog>
#include <KTextEdit>

#include "kractionbase.h"

class UserActionProperties;
class QDomDocument;
class QDomElement;
class KActionCollection;

/**
 * This subclass of KAction extends it with an individual executor and
 * a struct UserActionProperties.
 * It is used to integrate useractions into KDE's KAction-System
 */
class KrAction: public KAction, public KrActionBase
{
    Q_OBJECT
public:
    KrAction(KActionCollection *parent, QString name = QString());
    ~KrAction();

    /**
     * This chekcs if the KrAction is for a specific file / location available
     * @param currentURL Check for this file
     * @return true if the KrAction if available
     */
    bool isAvailable(const KUrl& currentURL);

    const QString& iconName() const {
        return _iconName;
    } // TODO: added for kde4 porting (functionality is missing)
    void setIconName(const QString& name) {
        _iconName = name;
    }

    bool xmlRead(const QDomElement& element);
    QDomElement xmlDump(QDomDocument& doc) const;

    void setName(const char*) { /* empty reimplementation to prevent a name-change */ };
    const QString & getName()   {
        return _name;
    }

    QString category() const {
        return _category;
    };
    void setCategory(const QString& category) {
        _category = category;
    };

    QString command() const {
        return _command;
    };
    void setCommand(const QString& command) {
        _command = command;
    };

    QString user() const {
        return _user;
    };
    void setUser(const QString& user) {
        _user = user;
    };

    QString startpath() const {
        return _startpath;
    };
    void setStartpath(const QString& startpath) {
        _startpath = startpath;
    };

    ExecType execType() const {
        return _execType;
    };
    void setExecType(ExecType execType) {
        _execType = execType;
    };

    bool acceptURLs() const {
        return _acceptURLs;
    };
    void setAcceptURLs(const bool& acceptURLs) {
        _acceptURLs = acceptURLs;
    };

    bool confirmExecution() const {
        return _confirmExecution;
    };
    void setConfirmExecution(const bool& confirmExecution) {
        _confirmExecution = confirmExecution;
    };

    QStringList showonlyProtocol() const {
        return _showonlyProtocol;
    };
    void setShowonlyProtocol(const QStringList& showonlyProtocol) {
        _showonlyProtocol = showonlyProtocol;
    };

    QStringList showonlyPath() const {
        return _showonlyPath;
    };
    void setShowonlyPath(const QStringList& showonlyPath) {
        _showonlyPath = showonlyPath;
    };

    QStringList showonlyMime() const {
        return _showonlyMime;
    };
    void setShowonlyMime(const QStringList& showonlyMime) {
        _showonlyMime = showonlyMime;
    };

    QStringList showonlyFile() const {
        return _showonlyFile;
    };
    void setShowonlyFile(const QStringList& showonlyFile) {
        _showonlyFile = showonlyFile;
    };

    bool doSubstitution() const {
        return true;
    }

    QString text() const {
        return KAction::text();
    }

public slots:
    void exec() {
        KrActionBase::exec();
    }


private:
    void readCommand(const QDomElement& element);
    QDomElement dumpCommand(QDomDocument& doc) const;

    void readAvailability(const QDomElement& element);
    QDomElement dumpAvailability(QDomDocument& doc) const;

    QString _iconName;
    QString _category;
    QString _command;
    QString _user;
    QString _startpath;
    ExecType _execType;
    bool _acceptURLs;
    bool _confirmExecution;
    QStringList _showonlyProtocol;
    QStringList _showonlyPath;
    QStringList _showonlyMime;
    QStringList _showonlyFile;

    QString  _name;
};

class QFont;
/**
 * This displays the output of a process
 */
class KrActionProcDlg: public KDialog
{
    Q_OBJECT
public:
    KrActionProcDlg(QString caption, bool enableStderr = false, QWidget *parent = 0);

public slots:
    void addStderr(const QString& str);
    void addStdout(const QString& str);

protected slots:
    void toggleFixedFont(bool state);
    void slotUser1(); ///< This is used to save the buffer to disc

private:
    KTextEdit *_stdout;
    KTextEdit *_stderr;
    KTextEdit *_currentTextEdit;
    QFont normalFont;
    QFont fixedFont;
private slots:
    void currentTextEditChanged();
};

/**
 * This executes a command of a UserAction
 */
// TODO jonas: call a list of commands separately (I began it but it doesn't work)
class KrActionProc: public QObject
{
    Q_OBJECT
public:

    KrActionProc(KrActionBase* action);
    virtual ~KrActionProc();
    void start(QString cmdLine);
    void start(QStringList cmdLineList);

protected slots:
    void kill() {
        _proc->kill();
    }
    void processExited(int exitCode, QProcess::ExitStatus exitStatus);
    void addStderr();
    void addStdout();

private:
    KrActionBase* _action;
    KProcess *_proc;
    QString _stdout;
    QString _stderr;
    KrActionProcDlg *_output;
};

#endif //KRACTION_H
