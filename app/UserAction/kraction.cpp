/*
    SPDX-FileCopyrightText: 2004 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2006 Jonas Bähr <jonas.baehr@web.de>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kraction.h"

// QtCore
#include <QDebug>
#include <QEvent>
#include <QFile>
#include <QMimeDatabase>
#include <QMimeType>
#include <QRegExp>
#include <QTextStream>
// QtGui
#include <QKeyEvent>
// QtWidgets
#include <QAction>
#include <QBoxLayout>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QSplitter>
// QtXml
#include <QDomElement>

#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <KParts/ReadOnlyPart>
#include <KSharedConfig>
#include <KShell>

#include "../GUI/terminaldock.h"
#include "../defaults.h"
#include "../icon.h"
#include "../krglobal.h"
#include "../krservices.h"
#include "../krusaderview.h"
#include "expander.h"
#include "useraction.h"

// KrActionProcDlg
KrActionProcDlg::KrActionProcDlg(const QString &caption, bool enableStderr, QWidget *parent)
    : QDialog(parent)
    , _stdout(nullptr)
    , _stderr(nullptr)
    , _currentTextEdit(nullptr)
{
    setWindowTitle(caption);
    setWindowModality(Qt::NonModal);

    auto *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    // do we need to separate stderr and stdout?
    if (enableStderr) {
        auto *splitt = new QSplitter(Qt::Horizontal, this);
        mainLayout->addWidget(splitt);
        // create stdout
        QWidget *stdoutWidget = new QWidget(splitt);
        auto *stdoutBox = new QVBoxLayout(stdoutWidget);

        stdoutBox->addWidget(new QLabel(i18n("Standard Output (stdout)"), stdoutWidget));
        _stdout = new KTextEdit(stdoutWidget);
        _stdout->setReadOnly(true);
        stdoutBox->addWidget(_stdout);
        // create stderr
        QWidget *stderrWidget = new QWidget(splitt);
        auto *stderrBox = new QVBoxLayout(stderrWidget);

        stderrBox->addWidget(new QLabel(i18n("Standard Error (stderr)"), stderrWidget));
        _stderr = new KTextEdit(stderrWidget);
        _stderr->setReadOnly(true);
        stderrBox->addWidget(_stderr);
    } else {
        // create stdout
        mainLayout->addWidget(new QLabel(i18n("Output")));
        _stdout = new KTextEdit;
        _stdout->setReadOnly(true);
        mainLayout->addWidget(_stdout);
    }

    _currentTextEdit = _stdout;
    connect(_stdout, &KTextEdit::textChanged, this, &KrActionProcDlg::currentTextEditChanged);
    if (_stderr)
        connect(_stderr, &KTextEdit::textChanged, this, &KrActionProcDlg::currentTextEditChanged);

    KConfigGroup group(krConfig, "UserActions");
    normalFont = group.readEntry("Normal Font", _UserActions_NormalFont);
    fixedFont = group.readEntry("Fixed Font", _UserActions_FixedFont);
    bool startupState = group.readEntry("Use Fixed Font", _UserActions_UseFixedFont);
    toggleFixedFont(startupState);

    auto *hbox = new QHBoxLayout;
    QCheckBox *useFixedFont = new QCheckBox(i18n("Use font with fixed width"));
    useFixedFont->setChecked(startupState);
    hbox->addWidget(useFixedFont);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    hbox->addWidget(buttonBox);

    mainLayout->addLayout(hbox);

    closeButton = buttonBox->button(QDialogButtonBox::Close);
    closeButton->setEnabled(false);

    auto *saveAsButton = new QPushButton;
    KGuiItem::assign(saveAsButton, KStandardGuiItem::saveAs());
    buttonBox->addButton(saveAsButton, QDialogButtonBox::ActionRole);

    killButton = new QPushButton(i18n("Kill"));
    killButton->setToolTip(i18n("Kill the running process"));
    killButton->setDefault(true);
    buttonBox->addButton(killButton, QDialogButtonBox::ActionRole);

    connect(killButton, &QPushButton::clicked, this, &KrActionProcDlg::killClicked);
    connect(saveAsButton, &QPushButton::clicked, this, &KrActionProcDlg::slotSaveAs);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &KrActionProcDlg::reject);
    connect(useFixedFont, &QCheckBox::toggled, this, &KrActionProcDlg::toggleFixedFont);

    resize(sizeHint() * 2);
}

void KrActionProcDlg::addStderr(const QString &str)
{
    if (_stderr)
        _stderr->append(str);
    else {
        _stdout->setFontItalic(true);
        _stdout->append(str);
        _stdout->setFontItalic(false);
    }
}

void KrActionProcDlg::addStdout(const QString &str)
{
    _stdout->append(str);
}

void KrActionProcDlg::toggleFixedFont(bool state)
{
    if (state) {
        _stdout->setFont(fixedFont);
        if (_stderr)
            _stderr->setFont(fixedFont);
    } else {
        _stdout->setFont(normalFont);
        if (_stderr)
            _stderr->setFont(normalFont);
    }
}

void KrActionProcDlg::slotSaveAs()
{
    QString filename = QFileDialog::getSaveFileName(this, QString(), QString(), i18n("*.txt|Text files\n*|All files"));
    if (filename.isEmpty())
        return;
    QFile file(filename);
    int answer = KMessageBox::PrimaryAction;
    if (file.exists())
        answer = KMessageBox::questionTwoActionsCancel(this, // parent
                                                 i18n("This file already exists.\nDo you want to overwrite it or append the output?"), // text
                                                 i18n("Overwrite or append?"), // caption
                                                 KStandardGuiItem::overwrite(), // label for primaryAction-Button
                                                 KGuiItem(i18n("Append")) // label for secondaryAction-Button
        );
    if (answer == KMessageBox::Cancel)
        return;
    bool open;
    if (answer == KMessageBox::SecondaryAction) // this means to append
        open = file.open(QIODevice::WriteOnly | QIODevice::Append);
    else
        open = file.open(QIODevice::WriteOnly);

    if (!open) {
        KMessageBox::error(this, i18n("Cannot open %1 for writing.\nNothing exported.", filename), i18n("Export failed"));
        return;
    }

    QTextStream stream(&file);
    stream << _currentTextEdit->toPlainText();
    file.close();
}

void KrActionProcDlg::slotProcessFinished()
{
    closeButton->setEnabled(true);
    killButton->setEnabled(false);
}

void KrActionProcDlg::currentTextEditChanged()
{
    if (_stderr && _stderr->hasFocus())
        _currentTextEdit = _stderr;
    else
        _currentTextEdit = _stdout;
}

// KrActionProc
KrActionProc::KrActionProc(KrActionBase *action)
    : _action(action)
    , _proc(nullptr)
    , _output(nullptr)
{
}

KrActionProc::~KrActionProc()
{
    delete _proc;
}

void KrActionProc::start(const QString &cmdLine)
{
    QStringList list;
    list << cmdLine;
    start(list);
}

void KrActionProc::start(QStringList cmdLineList)
{
    QString cmd; // this is the command which is really executed (with  maybe kdesu, maybe konsole, ...)
    // in case no specific working directory has been requested, execute in a relatively safe place
    QString workingDir = QDir::tempPath();

    if (!_action->startpath().isEmpty())
        workingDir = _action->startpath();

    if (!_action->user().isEmpty()) {
        if (!KrServices::isExecutable(KDESU_PATH)) {
            KMessageBox::error(nullptr,
                               i18n("Cannot run user action, %1 not found or not executable. "
                                    "Please verify that kde-cli-tools are installed.",
                                    QString(KDESU_PATH)));
            return;
        }
    }

    if (_action->execType() == KrAction::RunInTE && (!MAIN_VIEW->terminalDock()->initialise())) {
        KMessageBox::error(nullptr, i18n("Embedded terminal emulator does not work, using output collection instead."));
    }

    for (QStringList::Iterator it = cmdLineList.begin(); it != cmdLineList.end(); ++it) {
        if (!cmd.isEmpty())
            cmd += " ; "; // TODO make this separator configurable (users may want && or ||)
        // TODO: read header fom config or action-properties and place it on top of each command
        if (cmdLineList.count() > 1)
            cmd += "echo --------------------------------------- ; ";
        cmd += *it;
    }

    // make sure the command gets executed in the right directory
    cmd = "(cd " + KrServices::quote(workingDir) + " && (" + cmd + "))";

    if (_action->execType() == KrAction::RunInTE && MAIN_VIEW->terminalDock()->isInitialised()) { // send the commandline contents to the terminal emulator
        if (!_action->user().isEmpty()) {
            // "-t" is necessary that kdesu displays the terminal-output of the command
            cmd = KrServices::quote(KDESU_PATH) + " -t -u " + _action->user() + " -c " + KrServices::quote(cmd);
        }
        MAIN_VIEW->terminalDock()->sendInput(cmd + '\n');
        deleteLater();
    } else { // will start a new process
        _proc = new KProcess(this);
        _proc->clearProgram(); // this clears the arglist too
        _proc->setWorkingDirectory(workingDir);
        connect(_proc, QOverload<int, QProcess::ExitStatus>::of(&KProcess::finished), this, &KrActionProc::processExited);

        if (_action->execType() == KrAction::Normal || _action->execType() == KrAction::Terminal) { // not collect output
            if (_action->execType() == KrAction::Terminal) { // run in terminal
                KConfigGroup group(krConfig, "UserActions");
                QString term = group.readEntry("Terminal", _UserActions_Terminal);
                QStringList termArgs = KShell::splitArgs(term, KShell::TildeExpand);
                if (termArgs.isEmpty()) {
                    KMessageBox::error(nullptr, i18nc("Arg is a string containing the bad quoting.", "Bad quoting in terminal command:\n%1", term));
                    deleteLater();
                    return;
                }
                for (int i = 0; i != termArgs.size(); i++) {
                    if (termArgs[i] == "%t")
                        termArgs[i] = cmdLineList.join(" ; ");
                    else if (termArgs[i] == "%d")
                        termArgs[i] = workingDir;
                }
                termArgs << "sh"
                         << "-c" << cmd;
                cmd = KrServices::quote(termArgs).join(" ");
            }
            if (!_action->user().isEmpty()) {
                cmd = KrServices::quote(KDESU_PATH) + " -u " + _action->user() + " -c " + KrServices::quote(cmd);
            }
        } else { // collect output
            bool separateStderr = false;
            if (_action->execType() == KrAction::CollectOutputSeparateStderr)
                separateStderr = true;
            _output = new KrActionProcDlg(_action->text(), separateStderr);
            // connect the output to the dialog
            _proc->setOutputChannelMode(KProcess::SeparateChannels);
            connect(_proc, &KProcess::readyReadStandardError, this, &KrActionProc::addStderr);
            connect(_proc, &KProcess::readyReadStandardOutput, this, &KrActionProc::addStdout);
            connect(_output, &KrActionProcDlg::killClicked, this, &KrActionProc::kill);
            _output->show();

            if (!_action->user().isEmpty()) {
                // "-t" is necessary that kdesu displays the terminal-output of the command
                cmd = KrServices::quote(KDESU_PATH) + " -t -u " + _action->user() + " -c " + KrServices::quote(cmd);
            }
        }
        // printf("cmd: %s\n", cmd.toAscii().data());
        _proc->setShellCommand(cmd);
        _proc->start();
    }
}

void KrActionProc::processExited(int /*exitCode*/, QProcess::ExitStatus /*exitStatus*/)
{
    // enable the 'close' button on the dialog (if active), disable 'kill' button
    if (_output) {
        // TODO tell the user the program exit code
        _output->slotProcessFinished();
    }
    delete this; // banzai!!
}

void KrActionProc::addStderr()
{
    if (_output) {
        _output->addStderr(QString::fromLocal8Bit(_proc->readAllStandardError().data()));
    }
}

void KrActionProc::addStdout()
{
    if (_output) {
        _output->addStdout(QString::fromLocal8Bit(_proc->readAllStandardOutput().data()));
    }
}

// KrAction
KrAction::KrAction(KActionCollection *parent, const QString &name)
    : QAction((QObject *)parent)
{
    _actionCollection = parent;
    setObjectName(name);
    parent->addAction(name, this);

    connect(this, &KrAction::triggered, this, &KrAction::exec);
}

KrAction::~KrAction()
{
    const auto widgets = associatedObjects();
    for (QObject *w : widgets)
        qobject_cast<QWidget *>(w)->removeAction(this);
    krUserAction->removeKrAction(this); // Importent! Else Krusader will crash when writing the actions to file
}

bool KrAction::isAvailable(const QUrl &currentURL)
{
    bool available = true; // show per default (FIXME: make the default an attribute of <availability>)

    // check protocol
    if (!_showonlyProtocol.empty()) {
        available = false;
        for (auto &it : _showonlyProtocol) {
            // qDebug() << "KrAction::isAvailable currentProtocol: " << currentURL.scheme() << " =?= " << *it;
            if (currentURL.scheme() == it) { // FIXME remove trailing slashes at the xml-parsing (faster because done only once)
                available = true;
                break;
            }
        }
    } // check protocol: done

    // check the Path-list:
    if (available && !_showonlyPath.empty()) {
        available = false;
        for (auto &it : _showonlyPath) {
            if (it.right(1) == "*") {
                if (currentURL.path().indexOf(it.left(it.length() - 1)) == 0) {
                    available = true;
                    break;
                }
            } else if (currentURL.adjusted(QUrl::RemoveFilename | QUrl::StripTrailingSlash).path()
                       == it) { // FIXME remove trailing slashes at the xml-parsing (faster because done only once)
                available = true;
                break;
            }
        }
    } // check the Path-list: done

    // check mime-type
    if (available && !_showonlyMime.empty()) {
        available = false;
        QMimeDatabase db;
        QMimeType mime = db.mimeTypeForUrl(currentURL);
        if (mime.isValid()) {
            for (auto &it : _showonlyMime) {
                if (it.contains("/")) {
                    if (mime.inherits(it)) { // don't use ==; use 'inherits()' instead, which is aware of inheritance (ie: text/x-makefile is also text/plain)
                        available = true;
                        break;
                    }
                } else {
                    if (mime.name().indexOf(it) == 0) { // 0 is the beginning, -1 is not found
                        available = true;
                        break;
                    }
                }
            } // for
        }
    } // check the mime-type: done

    // check filename
    if (available && !_showonlyFile.empty()) {
        available = false;
        for (auto &it : _showonlyFile) {
            QRegExp regex = QRegExp(it, Qt::CaseInsensitive, QRegExp::Wildcard); // case-sensitive = false; wildcards = true
            if (regex.exactMatch(currentURL.fileName())) {
                available = true;
                break;
            }
        }
    } // check the filename: done

    return available;
}

bool KrAction::xmlRead(const QDomElement &element)
{
    /*
       This has to be done elsewhere!!

       if ( element.tagName() != "action" )
          return false;

       Also the name has to be checked before the action is created!

       setName( element.attribute( "name" ).toLatin1() );
    */
    QString attr;

    attr = element.attribute("enabled", "true"); // default: "true"
    if (attr == "false") {
        setEnabled(false);
        setVisible(false);
    }

    for (QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
        QDomElement e = node.toElement();
        if (e.isNull())
            continue; // this should skip nodes which are not elements ( i.e. comments, <!-- -->, or text nodes)

        if (e.tagName() == "title")
            setText(i18n(e.text().toUtf8().constData()));
        else if (e.tagName() == "tooltip")
            setToolTip(i18n(e.text().toUtf8().constData()));
        else if (e.tagName() == "icon")
            setIcon(Icon(_iconName = e.text()));
        else if (e.tagName() == "category")
            setCategory(i18n(e.text().toUtf8().constData()));
        else if (e.tagName() == "description")
            setWhatsThis(i18n(e.text().toUtf8().constData()));
        else if (e.tagName() == "command")
            readCommand(e);
        else if (e.tagName() == "startpath")
            setStartpath(e.text());
        else if (e.tagName() == "availability")
            readAvailability(e);
        else if (e.tagName() == "defaultshortcut")
            _actionCollection->setDefaultShortcut(this, QKeySequence(e.text()));
        else

            // unknown but not empty
            if (!e.tagName().isEmpty())
                qWarning() << "KrAction::xmlRead() - unrecognized tag found: <action name=\"" << objectName() << "\"><" << e.tagName() << ">";

    } // for ( QDomNode node = action->firstChild(); !node.isNull(); node = node.nextSibling() )

    return true;
} // KrAction::xmlRead

QDomElement KrAction::xmlDump(QDomDocument &doc) const
{
    QDomElement actionElement = doc.createElement("action");
    actionElement.setAttribute("name", objectName());

    if (!isVisible()) {
        actionElement.setAttribute("enabled", "false");
    }

#define TEXT_ELEMENT(TAGNAME, TEXT)                                                                                                                            \
    {                                                                                                                                                          \
        QDomElement e = doc.createElement(TAGNAME);                                                                                                            \
        e.appendChild(doc.createTextNode(TEXT));                                                                                                               \
        actionElement.appendChild(e);                                                                                                                          \
    }

    TEXT_ELEMENT("title", text())

    if (!toolTip().isEmpty())
        TEXT_ELEMENT("tooltip", toolTip())

    if (!_iconName.isEmpty())
        TEXT_ELEMENT("icon", _iconName)

    if (!category().isEmpty())
        TEXT_ELEMENT("category", category())

    if (!whatsThis().isEmpty())
        TEXT_ELEMENT("description", whatsThis())

    actionElement.appendChild(dumpCommand(doc));

    if (!startpath().isEmpty())
        TEXT_ELEMENT("startpath", startpath())

    QDomElement availabilityElement = dumpAvailability(doc);

    if (availabilityElement.hasChildNodes())
        actionElement.appendChild(availabilityElement);

    if (!shortcut().isEmpty())
        TEXT_ELEMENT("defaultshortcut", shortcut().toString()) //.toString() would return a localised string which can't be read again

    return actionElement;
} // KrAction::xmlDump

void KrAction::readCommand(const QDomElement &element)
{
    QString attr;

    attr = element.attribute("executionmode", "normal"); // default: "normal"
    if (attr == "normal")
        setExecType(Normal);
    else if (attr == "terminal")
        setExecType(Terminal);
    else if (attr == "collect_output")
        setExecType(CollectOutput);
    else if (attr == "collect_output_separate_stderr")
        setExecType(CollectOutputSeparateStderr);
    else if (attr == "embedded_terminal")
        setExecType(RunInTE);
    else
        qWarning() << "KrAction::readCommand() - unrecognized attribute value found: <action name=\"" << objectName() << "\"><command executionmode=\"" << attr
                   << "\"";

    attr = element.attribute("accept", "local"); // default: "local"
    if (attr == "local")
        setAcceptURLs(false);
    else if (attr == "url")
        setAcceptURLs(true);
    else
        qWarning() << "KrAction::readCommand() - unrecognized attribute value found: <action name=\"" << objectName() << "\"><command accept=\"" << attr
                   << "\"";

    attr = element.attribute("confirmexecution", "false"); // default: "false"
    if (attr == "true")
        setConfirmExecution(true);
    else
        setConfirmExecution(false);

    setUser(element.attribute("run_as"));

    setCommand(element.text());

} // KrAction::readCommand

QDomElement KrAction::dumpCommand(QDomDocument &doc) const
{
    QDomElement commandElement = doc.createElement("command");

    switch (execType()) {
    case Terminal:
        commandElement.setAttribute("executionmode", "terminal");
        break;
    case CollectOutput:
        commandElement.setAttribute("executionmode", "collect_output");
        break;
    case CollectOutputSeparateStderr:
        commandElement.setAttribute("executionmode", "collect_output_separate_stderr");
        break;
    case RunInTE:
        commandElement.setAttribute("executionmode", "embedded_terminal");
        break;
    default:
        // don't write the default to file
        break;
    }

    if (acceptURLs())
        commandElement.setAttribute("accept", "url");

    if (confirmExecution())
        commandElement.setAttribute("confirmexecution", "true");

    if (!user().isEmpty())
        commandElement.setAttribute("run_as", user());

    commandElement.appendChild(doc.createTextNode(command()));

    return commandElement;
} // KrAction::dumpCommand

void KrAction::readAvailability(const QDomElement &element)
{
    for (QDomNode node = element.firstChild(); !node.isNull(); node = node.nextSibling()) {
        QDomElement e = node.toElement();
        if (e.isNull())
            continue; // this should skip nodes which are not elements ( i.e. comments, <!-- -->, or text nodes)

        QStringList *showlist = nullptr;

        if (e.tagName() == "protocol")
            showlist = &_showonlyProtocol;
        else if (e.tagName() == "path")
            showlist = &_showonlyPath;
        else if (e.tagName() == "mimetype")
            showlist = &_showonlyMime;
        else if (e.tagName() == "filename")
            showlist = &_showonlyFile;
        else {
            qWarning() << "KrAction::readAvailability() - unrecognized element found: <action name=\"" << objectName() << "\"><availability><" << e.tagName()
                       << ">";
            showlist = nullptr;
        }

        if (showlist) {
            for (QDomNode subnode = e.firstChild(); !subnode.isNull(); subnode = subnode.nextSibling()) {
                QDomElement subelement = subnode.toElement();
                if (subelement.tagName() == "show")
                    showlist->append(subelement.text());
            } // for
        } // if ( showlist )

    } // for
} // KrAction::readAvailability

QDomElement KrAction::dumpAvailability(QDomDocument &doc) const
{
    QDomElement availabilityElement = doc.createElement("availability");

#define LIST_ELEMENT(TAGNAME, LIST)                                                                                                                            \
    {                                                                                                                                                          \
        QDomElement e = doc.createElement(TAGNAME);                                                                                                            \
        for (QStringList::const_iterator it = LIST.constBegin(); it != LIST.constEnd(); ++it) {                                                                \
            QDomElement show = doc.createElement("show");                                                                                                      \
            show.appendChild(doc.createTextNode(*it));                                                                                                         \
            e.appendChild(show);                                                                                                                               \
        }                                                                                                                                                      \
        availabilityElement.appendChild(e);                                                                                                                    \
    }

    if (!_showonlyProtocol.isEmpty())
        LIST_ELEMENT("protocol", _showonlyProtocol)

    if (!_showonlyPath.isEmpty())
        LIST_ELEMENT("path", _showonlyPath)

    if (!_showonlyMime.isEmpty())
        LIST_ELEMENT("mimetype", _showonlyMime)

    if (!_showonlyFile.isEmpty())
        LIST_ELEMENT("filename", _showonlyFile)

    return availabilityElement;
} // KrAction::dumpAvailability
