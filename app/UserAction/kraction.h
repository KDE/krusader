/*
    SPDX-FileCopyrightText: 2004 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006 Jonas Bähr <jonas.baehr@web.de>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRACTION_H
#define KRACTION_H

// QtCore
#include <QByteArray>
#include <QUrl>
// QtGui
#include <QFont>
// QtWidgets
#include <QAction>
#include <QDialog>

#include <KProcess>
#include <KTextEdit>

#include "kractionbase.h"

class QDomDocument;
class QDomElement;
class KActionCollection;

/**
 * This subclass of QAction extends it with an individual executor and
 * a struct UserActionProperties.
 * It is used to integrate useractions into KDE's QAction-System
 */
class KrAction : public QAction, public KrActionBase
{
    Q_OBJECT
public:
    explicit KrAction(KActionCollection *parent, const QString &name = QString());
    ~KrAction() override;

    /**
     * This chekcs if the KrAction is for a specific file / location available
     * @param currentURL Check for this file
     * @return true if the KrAction if available
     */
    bool isAvailable(const QUrl &currentURL);

    const QString &iconName() const
    {
        return _iconName;
    } // TODO: added for kde4 porting (functionality is missing)
    void setIconName(const QString &name)
    {
        _iconName = name;
    }

    bool xmlRead(const QDomElement &element);
    QDomElement xmlDump(QDomDocument &doc) const;

    QString category() const
    {
        return _category;
    };
    void setCategory(const QString &category)
    {
        _category = category;
    };

    QString command() const override
    {
        return _command;
    };
    void setCommand(const QString &command)
    {
        _command = command;
    };

    QString user() const override
    {
        return _user;
    };
    void setUser(const QString &user)
    {
        _user = user;
    };

    QString startpath() const override
    {
        return _startpath;
    };
    void setStartpath(const QString &startpath)
    {
        _startpath = startpath;
    };

    ExecType execType() const override
    {
        return _execType;
    };
    void setExecType(ExecType execType)
    {
        _execType = execType;
    };

    bool acceptURLs() const override
    {
        return _acceptURLs;
    };
    void setAcceptURLs(const bool &acceptURLs)
    {
        _acceptURLs = acceptURLs;
    };

    bool confirmExecution() const override
    {
        return _confirmExecution;
    };
    void setConfirmExecution(const bool &confirmExecution)
    {
        _confirmExecution = confirmExecution;
    };

    QStringList showonlyProtocol() const
    {
        return _showonlyProtocol;
    };
    void setShowonlyProtocol(const QStringList &showonlyProtocol)
    {
        _showonlyProtocol = showonlyProtocol;
    };

    QStringList showonlyPath() const
    {
        return _showonlyPath;
    };
    void setShowonlyPath(const QStringList &showonlyPath)
    {
        _showonlyPath = showonlyPath;
    };

    QStringList showonlyMime() const
    {
        return _showonlyMime;
    };
    void setShowonlyMime(const QStringList &showonlyMime)
    {
        _showonlyMime = showonlyMime;
    };

    QStringList showonlyFile() const
    {
        return _showonlyFile;
    };
    void setShowonlyFile(const QStringList &showonlyFile)
    {
        _showonlyFile = showonlyFile;
    };

    bool doSubstitution() const override
    {
        return true;
    }

    QString text() const override
    {
        return QAction::text();
    }

public slots:
    void exec()
    {
        KrActionBase::exec();
    }

private:
    void readCommand(const QDomElement &element);
    QDomElement dumpCommand(QDomDocument &doc) const;

    void readAvailability(const QDomElement &element);
    QDomElement dumpAvailability(QDomDocument &doc) const;

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
    KActionCollection *_actionCollection;
};

/**
 * This displays the output of a process
 */
class KrActionProcDlg : public QDialog
{
    Q_OBJECT
public:
    explicit KrActionProcDlg(const QString &caption, bool enableStderr = false, QWidget *parent = nullptr);

public slots:
    void addStderr(const QString &str);
    void addStdout(const QString &str);
    void slotProcessFinished();

protected slots:
    void toggleFixedFont(bool state);
    void slotSaveAs();

signals:
    void killClicked();

private:
    KTextEdit *_stdout;
    KTextEdit *_stderr;
    KTextEdit *_currentTextEdit;
    QFont normalFont;
    QFont fixedFont;
    QPushButton *closeButton;
    QPushButton *killButton;
private slots:
    void currentTextEditChanged();
};

/**
 * This executes a command of a UserAction
 */
// TODO jonas: call a list of commands separately (I began it but it doesn't work)
class KrActionProc : public QObject
{
    Q_OBJECT
public:
    explicit KrActionProc(KrActionBase *action);
    ~KrActionProc() override;
    void start(const QString &cmdLine);
    void start(QStringList cmdLineList);

protected slots:
    void kill()
    {
        _proc->kill();
    }
    void processExited(int exitCode, QProcess::ExitStatus exitStatus);
    void addStderr();
    void addStdout();

private:
    KrActionBase *_action;
    KProcess *_proc;
    QString _stdout;
    QString _stderr;
    KrActionProcDlg *_output;
};

#endif // KRACTION_H
