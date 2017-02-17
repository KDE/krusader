/*****************************************************************************
 * Copyright (C) 2005 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2007-2008 Csaba Karai <cskarai@freemail.hu>                 *
 * Copyright (C) 2008 Jonas Bähr <jonas.baehr@web.de>                        *
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

#include "checksumdlg.h"
#include "../krusader.h"
#include "../krglobal.h"
#include "../GUI/krlistwidget.h"
#include "../GUI/krtreewidget.h"

#include <unistd.h>

// QtCore
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QMap>
#include <QTemporaryFile>
#include <QTextStream>
// QtGui
#include <QPixmap>
// QtWidgets
#include <QApplication>
#include <QDialogButtonBox>
#include <QLayout>
#include <QLabel>
#include <QCheckBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QGridLayout>

#include <KCoreAddons/KProcess>
#include <KCompletion/KLineEdit>
#include <KI18n/KLocalizedString>
#include <KWidgetsAddons/KCursor>
#include <KWidgetsAddons/KMessageBox>
#include <KIconThemes/KIconLoader>
#include <KCompletion/KComboBox>

#include "../krservices.h"

class CS_Tool; // forward
typedef void CREATE_FUNC(KProcess& proc, CS_Tool *self, const QStringList& files, bool recursive);
typedef void VERIFY_FUNC(KProcess& proc, CS_Tool *self, const QStringList& files,
                         const QString checksumFile, bool recursive);

typedef QStringList GET_FAILED_FUNC(const QStringList& stdOut, const QStringList& stdErr);

class CS_Tool
{
public:
    enum Type {
        MD5 = 0, SHA1, SHA256, TIGER, WHIRLPOOL,
        SHA224, SHA384, SHA512,
        NumOfTypes
    };

    Type type;
    QString binary;
    bool recursive;
    CREATE_FUNC *create;
    VERIFY_FUNC *verify;
    GET_FAILED_FUNC *failed;
};

class CS_ToolByType
{
public:
    QList<CS_Tool *> tools, r_tools; // normal and recursive tools
};

// handles md5sum and sha1sum
void sumCreateFunc(KProcess& proc, CS_Tool *self, const QStringList& files, bool recursive)
{
    Q_UNUSED(recursive)
    proc << KrServices::fullPathName(self->binary);
    Q_ASSERT(!recursive);
    proc << files;
}

void sumVerifyFunc(KProcess& proc, CS_Tool *self, const QStringList& /* files */,
                   const QString checksumFile, bool recursive)
{
    // TODO there is a huge bug here: it is expected that all selected files are checked, but in
    // fact we ignore the file list and only check what is in the checksum file.
    Q_UNUSED(recursive)
    proc << KrServices::fullPathName(self->binary);
    Q_ASSERT(!recursive);
    proc << "-c" << checksumFile;
}

QStringList sumFailedFunc(const QStringList& stdOut, const QStringList& stdErr)
{
    // md5sum and sha1sum print "...: FAILED" for failed files and display
    // the number of failures to stderr. so if stderr is empty, we'll assume all is ok
    QStringList result;
    if (stdErr.size() == 0)
        return result;
    result += stdErr;
    // grep for the ":FAILED" substring
    const QString tmp = QString(": FAILED").toLocal8Bit();
    for (int i = 0; i < stdOut.size(); ++i) {
        if (stdOut[i].indexOf(tmp) != -1)
            result += stdOut[i];
    }

    return result;
}

// handles *deep binaries
void deepCreateFunc(KProcess& proc, CS_Tool *self, const QStringList& files, bool recursive)
{
    proc << KrServices::fullPathName(self->binary);
    if (recursive)
        proc << "-r";
    proc << "-l" << files;
}

void deepVerifyFunc(KProcess& proc, CS_Tool *self, const QStringList& files,
                    const QString checksumFile, bool recursive)
{
    proc << KrServices::fullPathName(self->binary);
    if (recursive)
        proc << "-r";
    proc << "-x" << checksumFile << files;
}

QStringList deepFailedFunc(const QStringList& stdOut, const QStringList&/* stdErr */)
{
    // *deep dumps (via -x) all failed hashes to stdout
    return stdOut;
}

// important: this table should be ordered like so that all md5 tools should be
// one after another, and then all sha1 and so on and so forth. The tools must be grouped,
// since the code in getTools() counts on it!
CS_Tool cs_tools[] = {
    // type              binary            recursive    create_func       verify_func      failed_func
    {CS_Tool::MD5,       "md5sum",         false,       sumCreateFunc,    sumVerifyFunc,   sumFailedFunc},
    {CS_Tool::MD5,       "md5deep",        true,        deepCreateFunc,   deepVerifyFunc,  deepFailedFunc},
    {CS_Tool::SHA1,      "sha1sum",        false,       sumCreateFunc,    sumVerifyFunc,   sumFailedFunc},
    {CS_Tool::SHA1,      "sha1deep",       true,        deepCreateFunc,   deepVerifyFunc,  deepFailedFunc},
    {CS_Tool::SHA224,    "sha224sum",      false,       sumCreateFunc,    sumVerifyFunc,   sumFailedFunc},
    {CS_Tool::SHA256,    "sha256sum",      false,       sumCreateFunc,    sumVerifyFunc,   sumFailedFunc},
    {CS_Tool::SHA256,    "sha256deep",     true,        deepCreateFunc,   deepVerifyFunc,  deepFailedFunc},
    {CS_Tool::SHA384,    "sha384sum",      false,       sumCreateFunc,    sumVerifyFunc,   sumFailedFunc},
    {CS_Tool::SHA512,    "sha512sum",      false,       sumCreateFunc,    sumVerifyFunc,   sumFailedFunc},
    {CS_Tool::TIGER,     "tigerdeep",      true,        deepCreateFunc,   deepVerifyFunc,  deepFailedFunc},
    {CS_Tool::WHIRLPOOL, "whirlpooldeep",  true,        deepCreateFunc,   deepVerifyFunc,  deepFailedFunc},
};

QMap<QString, CS_Tool::Type> cs_textToType;
QMap<CS_Tool::Type, QString> cs_typeToText;

void initChecksumModule()
{
    // prepare the dictionaries - pity it has to be manually
    cs_textToType["md5"] = CS_Tool::MD5;
    cs_textToType["sha1"] = CS_Tool::SHA1;
    cs_textToType["sha256"] = CS_Tool::SHA256;
    cs_textToType["sha224"] = CS_Tool::SHA224;
    cs_textToType["sha384"] = CS_Tool::SHA384;
    cs_textToType["sha512"] = CS_Tool::SHA512;
    cs_textToType["tiger"] = CS_Tool::TIGER;
    cs_textToType["whirlpool"] = CS_Tool::WHIRLPOOL;

    cs_typeToText[CS_Tool::MD5] = "md5";
    cs_typeToText[CS_Tool::SHA1] = "sha1";
    cs_typeToText[CS_Tool::SHA256] = "sha256";
    cs_typeToText[CS_Tool::SHA224] = "sha224";
    cs_typeToText[CS_Tool::SHA384] = "sha384";
    cs_typeToText[CS_Tool::SHA512] = "sha512";
    cs_typeToText[CS_Tool::TIGER] = "tiger";
    cs_typeToText[CS_Tool::WHIRLPOOL] = "whirlpool";

    // build the checksumFilter (for usage in KRQuery)
    for (const QString text: cs_textToType.keys())
        Checksum::checksumTypesFilter += ("*." + text + ' ');
}

// --------------------------------------------------

// returns a list of tools which can work with recursive or non-recursive mode and are installed
// note: only 1 tool from each type is suggested
static QList<CS_Tool *> getTools(bool folders)
{
    QList<CS_Tool *> result;
    uint i;
    for (i = 0; i < sizeof(cs_tools) / sizeof(CS_Tool); ++i) {
        if (!result.isEmpty() && result.last()->type == cs_tools[i].type)
            continue; // 1 from each type please
        if (folders && !cs_tools[i].recursive)
            continue;
        if (KrServices::cmdExist(cs_tools[i].binary))
            result.append(&cs_tools[i]);
    }

    if (result.count() == 0) { // nothing was suggested?!
        QString error = i18n("<qt>No supported checksum tool was found. Please check the "
                             "<b>Dependencies</b> page in Krusader's settings.</qt>");
        if (folders)
            error += i18n("<qt><b>Note</b>: you have selected folders, and probably have no "
                          "recursive checksum tool installed. Krusader currently supports "
                          "<i>md5deep, sha1deep, sha256deep and tigerdeep</i></qt>");
        KMessageBox::error(0, error);
    }

    return result;
}

// ------------- CreateChecksumDlg

Checksum::CreateDialog::CreateDialog(const QStringList& files, bool containFolders, const QString& path)
    : QDialog(krApp)
{
    const QList<CS_Tool *> tools = getTools(containFolders);
    if (tools.count() == 0)
        return;

    setWindowModality(Qt::WindowModal);
    setWindowTitle(i18n("Create Checksum"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QWidget *widget = new QWidget(this);
    QGridLayout *layout = new QGridLayout(widget);

    int row = 0;

    // title (icon+text)
    QHBoxLayout *hlayout = new QHBoxLayout;
    QLabel *p = new QLabel(widget);
    p->setPixmap(krLoader->loadIcon("document-edit-sign", KIconLoader::Desktop, 32));
    hlayout->addWidget(p);
    QLabel *l1 = new QLabel(widget);

    l1->setText(containFolders ?
                    i18n("About to calculate checksum for the following files and folders:") :
                    i18n("About to calculate checksum for the following files:"));

    hlayout->addWidget(l1);
    layout->addLayout(hlayout, row, 0, 1, 2, Qt::AlignLeft);
    ++row;

    // file list
    KrListWidget *lb = new KrListWidget(widget);
    lb->addItems(files);
    layout->addWidget(lb, row, 0, 1, 2);
    ++row;

    // checksum method
    QHBoxLayout *hlayout2 = new QHBoxLayout;
    QLabel *l2 = new QLabel(i18n("Select the checksum method:"), widget);
    hlayout2->addWidget(l2);
    KComboBox *method = new KComboBox(widget);
    // -- fill the combo with available methods
    for (int i = 0; i < tools.count(); ++i)
        method->addItem(cs_typeToText[tools.at(i)->type], i);
    method->setFocus();
    hlayout2->addWidget(method);
    layout->addLayout(hlayout2, row, 0, 1, 2, Qt::AlignLeft);
    ++row;
    mainLayout->addWidget(widget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    if (exec() != Accepted)
        return;
    // else implied: run the process
    QTemporaryFile tmpOut(QDir::tempPath() + QLatin1String("/krusader_XXXXXX.stdout"));
    tmpOut.open(); // necessary to create the filename
    QTemporaryFile tmpErr(QDir::tempPath() + QLatin1String("/krusader_XXXXXX.stderr"));
    tmpErr.open(); // necessary to create the filename
    KProcess proc;
    CS_Tool *mytool = tools.at(method->currentIndex());
    mytool->create(proc, mytool, files, containFolders);
    proc.setOutputChannelMode(KProcess::SeparateChannels); // without this the next 2 lines have no effect!
    proc.setStandardOutputFile(tmpOut.fileName());
    proc.setStandardErrorFile(tmpErr.fileName());
    proc.setWorkingDirectory(path);

    krApp->startWaiting(i18n("Calculating checksums..."), 0, true);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    proc.start();
    // TODO make use of asynchronous process starting. waitForStarted(int msec = 30000) is blocking
    // it would be better to connect to started(), error() and finished()
    if (proc.waitForStarted())
        while (proc.state() == QProcess::Running) {
            usleep(500);
            qApp->processEvents();
            if (krApp->wasWaitingCancelled()) { // user cancelled
                proc.kill();
                QApplication::restoreOverrideCursor();
                return;
            }
        };
    krApp->stopWait();
    QApplication::restoreOverrideCursor();
    if (proc.exitStatus() != QProcess::NormalExit) {
        KMessageBox::error(0, i18n("<qt>There was an error while running <b>%1</b>.</qt>", mytool->binary));
        return;
    }

    // suggest a filename
    QString suggestedFilename = path + '/';
    if (files.count() > 1)
        suggestedFilename += ("checksum." + cs_typeToText[mytool->type]);
    else
        suggestedFilename += (files[0] + '.' + cs_typeToText[mytool->type]);
    // send both stdout and stderr
    QStringList stdOut, stdErr;
    if (!KrServices::fileToStringList(&tmpOut, stdOut) ||
        !KrServices::fileToStringList(&tmpErr, stdErr)) {
        KMessageBox::error(krApp, i18n("Error reading stdout or stderr"));
        return;
    }

    ResultsDialog dlg(stdOut, stdErr, suggestedFilename);
}

QString Checksum::checksumTypesFilter;

// ------------- MatchChecksumDlg

Checksum::MatchDialog::MatchDialog(const QStringList& files, bool containFolders,
                                   const QString& path, const QString& checksumFile)
    : QDialog(krApp)
{
    const QList<CS_Tool *> tools = getTools(containFolders);
    if (tools.count() == 0)
        return;

    setWindowTitle(i18n("Verify Checksum"));
    setWindowModality(Qt::WindowModal);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QWidget *widget = new QWidget(this);
    QGridLayout *layout = new QGridLayout(widget);

    int row = 0;

    // title (icon+text)
    QHBoxLayout *hlayout = new QHBoxLayout;
    QLabel *p = new QLabel(widget);
    p->setPixmap(krLoader->loadIcon("document-edit-decrypt-verify", KIconLoader::Desktop, 32));
    hlayout->addWidget(p);
    QLabel *l1 = new QLabel(widget);

    l1->setText(containFolders ?
                    i18n("About to verify checksum for the following files and folders:") :
                    i18n("About to verify checksum for the following files:"));

    hlayout->addWidget(l1);
    layout->addLayout(hlayout, row, 0, 1, 2, Qt::AlignLeft);
    ++row;

    // file list
    KrListWidget *lb = new KrListWidget(widget);
    lb->addItems(files);
    layout->addWidget(lb, row, 0, 1, 2);
    ++row;

    // checksum file
    QHBoxLayout *hlayout2 = new QHBoxLayout;
    QLabel *l2 = new QLabel(i18n("Checksum file:"), widget);
    hlayout2->addWidget(l2);
    KUrlRequester *checksumFileReq = new KUrlRequester(widget);
    checksumFileReq->setUrl(QUrl::fromLocalFile(path));
    if (!checksumFile.isEmpty())
        checksumFileReq->setUrl(QUrl::fromLocalFile(checksumFile));
    checksumFileReq->setFocus();
    hlayout2->addWidget(checksumFileReq);
    layout->addLayout(hlayout2, row, 0, 1, 2, Qt::AlignLeft);
    mainLayout->addWidget(widget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    if (exec() != Accepted)
        return;
    QString file = checksumFileReq->url().toDisplayString(QUrl::PreferLocalFile);
    QString extension;
    if (!verifyChecksumFile(file, extension)) {
        KMessageBox::error(0, i18n("<qt>Error reading checksum file <i>%1</i>.<br />Please specify "
                                   "a valid checksum file.</qt>",
                                   file));
        return;
    }

    // do we have a tool for that extension?
    CS_Tool *mytool = 0;
    for (CS_Tool *tool : tools) {
        if (cs_typeToText[tool->type] == extension.toLower()) {
            mytool = tool;
            break;
        }
    }
    if (!mytool) {
        KMessageBox::error(
            0, i18n("<qt>Krusader cannot find a checksum tool that handles %1 on your system. "
                    "Please check the <b>Dependencies</b> page in Krusader's settings.</qt>",
                    extension));
        return;
    }

    // else implied: run the process
    QTemporaryFile tmpOut(QDir::tempPath() + QLatin1String("/krusader_XXXXXX.stdout"));
    tmpOut.open(); // necessary to create the filename
    QTemporaryFile tmpErr(QDir::tempPath() + QLatin1String("/krusader_XXXXXX.stderr"));
    tmpErr.open(); // necessary to create the filename
    KProcess proc;
    mytool->verify(proc, mytool, files, file, containFolders);
    proc.setOutputChannelMode(KProcess::SeparateChannels); // without this the next 2 lines have no effect!
    proc.setStandardOutputFile(tmpOut.fileName());
    proc.setStandardErrorFile(tmpErr.fileName());
    proc.setWorkingDirectory(path);

    krApp->startWaiting(i18n("Verifying checksums..."), 0, true);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    proc.start();
    // TODO make use of asynchronous process starting. waitForStarted(int msec = 30000) is blocking
    // it would be better to connect to started(), error() and finished()
    if (proc.waitForStarted())
        while (proc.state() == QProcess::Running) {
            usleep(500);
            qApp->processEvents();
            if (krApp->wasWaitingCancelled()) { // user cancelled
                proc.kill();
                QApplication::restoreOverrideCursor();
                return;
            }
        };
    if (proc.exitStatus() != QProcess::NormalExit) {
        KMessageBox::error(0, i18n("<qt>There was an error while running <b>%1</b>.</qt>", mytool->binary));
        return;
    }
    QApplication::restoreOverrideCursor();
    krApp->stopWait();
    // send both stdout and stderr
    QStringList stdOut, stdErr;
    if (!KrServices::fileToStringList(&tmpOut, stdOut) ||
        !KrServices::fileToStringList(&tmpErr, stdErr)) {
        KMessageBox::error(krApp, i18n("Error reading stdout or stderr"));
        return;
    }
    VerifyDialog dlg(mytool->failed(stdOut, stdErr));
}

bool Checksum::MatchDialog::verifyChecksumFile(QString path, QString& extension)
{
    QFileInfo f(path);
    if (!f.exists() || f.isDir())
        return false;
    // find the extension
    extension = path.mid(path.lastIndexOf(".") + 1);

    // TODO: do we know the extension? if not, ask the user for one

    return true;
}

// ------------- VerifyResultDlg
Checksum::VerifyDialog::VerifyDialog(const QStringList& failed)
    : QDialog(krApp)
{
    setWindowTitle(i18n("Verify Checksum"));
    setWindowModality(Qt::WindowModal);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QWidget *widget = new QWidget(this);
    QGridLayout *layout = new QGridLayout(widget);

    bool errors = failed.size() > 0;
    int row = 0;

    // create the icon and title
    QHBoxLayout *hlayout = new QHBoxLayout;
    QLabel p(widget);
    p.setPixmap(krLoader->loadIcon(errors ? "dialog-error" : "dialog-information", KIconLoader::Desktop, 32));
    hlayout->addWidget(&p);

    QLabel *l1 = new QLabel((errors ? i18n("Errors were detected while verifying the checksums") :
                                      i18n("Checksums were verified successfully")), widget);
    hlayout->addWidget(l1);
    layout->addLayout(hlayout, row, 0, 1, 2, Qt::AlignLeft);
    ++row;

    if (errors) {
        QLabel *l3 = new QLabel(i18n("The following files have failed:"), widget);
        layout->addWidget(l3, row, 0, 1, 2);
        ++row;
        KrListWidget *lb2 = new KrListWidget(widget);
        lb2->addItems(failed);
        layout->addWidget(lb2, row, 0, 1, 2);
        ++row;
    }

    mainLayout->addWidget(widget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    mainLayout->addWidget(buttonBox);
    buttonBox->button(QDialogButtonBox::Close)->setDefault(true);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    exec();
}

// ------------- ChecksumResultsDlg

Checksum::ResultsDialog::ResultsDialog(const QStringList &stdOut, const QStringList &stdErr,
                                       const QString& suggestedFilename)
    : QDialog(krApp), _onePerFile(0), _checksumFileSelector(0), _data(stdOut), _suggestedFilename(suggestedFilename)
{
    // md5 tools display errors into stderr, so we'll use that to determine the result of the job
    bool errors = stdErr.size() > 0;
    bool successes = stdOut.size() > 0;

    setWindowTitle(i18n("Create Checksum"));
    setWindowModality(Qt::WindowModal);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    QWidget *widget = new QWidget(this);
    QGridLayout *layout = new QGridLayout(widget);

    int row = 0;

    // create the icon and title
    QHBoxLayout *hlayout = new QHBoxLayout;
    QLabel p(widget);
    p.setPixmap(krLoader->loadIcon(errors || !successes ? "dialog-error" : "dialog-information",
                                   KIconLoader::Desktop, 32));
    hlayout->addWidget(&p);

    QLabel *l1 = new QLabel((errors || !successes ? i18n("Errors were detected while creating the checksums") :
                                                    i18n("Checksums were created successfully")), widget);
    hlayout->addWidget(l1);
    layout->addLayout(hlayout, row, 0, 1, 2, Qt::AlignLeft);
    ++row;

    if (successes) {
        if (errors) {
            QLabel *l2 = new QLabel(i18n("Here are the calculated checksums:"), widget);
            layout->addWidget(l2, row, 0, 1, 2);
            ++row;
        }
        KrTreeWidget *lv = new KrTreeWidget(widget);

        QStringList columns;
        columns << i18n("Hash") << i18n("File");
        lv->setAllColumnsShowFocus(true);
        lv->setHeaderLabels(columns);

        for (const QString line : stdOut) {
            const int space = line.indexOf(' ');
            QTreeWidgetItem *item = new QTreeWidgetItem(lv);
            item->setText(0, line.left(space));
            item->setText(1, line.mid(space + 2));
        }
        lv->sortItems(1, Qt::AscendingOrder);

        layout->addWidget(lv, row, 0, 1, 2);
        ++row;
    }

    if (errors) {
        QFrame *line1 = new QFrame(widget);
        line1->setGeometry(QRect(60, 210, 501, 20));
        line1->setFrameShape(QFrame::HLine);
        line1->setFrameShadow(QFrame::Sunken);
        layout->addWidget(line1, row, 0, 1, 2);
        ++row;

        QLabel *l3 = new QLabel(i18n("Here are the errors received:"), widget);
        layout->addWidget(l3, row, 0, 1, 2);
        ++row;
        KrListWidget *lb = new KrListWidget(widget);
        lb->addItems(stdErr);
        layout->addWidget(lb, row, 0, 1, 2);
        ++row;
    }

    // save checksum to disk, if any hashes are found

    if (successes) {
        QHBoxLayout *hlayout2 = new QHBoxLayout;
        QLabel *label = new QLabel(i18n("Save checksum to file:"), widget);
        hlayout2->addWidget(label);

        _checksumFileSelector = new KUrlRequester(QUrl::fromLocalFile(suggestedFilename), widget);
        hlayout2->addWidget(_checksumFileSelector, Qt::AlignLeft);
        layout->addLayout(hlayout2, row, 0, 1, 2, Qt::AlignLeft);
        ++row;
        _checksumFileSelector->setFocus();
    }

    if (stdOut.size() > 1) {
        _onePerFile = new QCheckBox(i18n("Checksum file for each source file"), widget);
        _onePerFile->setChecked(false);
        connect(_onePerFile, SIGNAL(toggled(bool)), _checksumFileSelector, SLOT(setDisabled(bool)));
        layout->addWidget(_onePerFile, row, 0, 1, 2, Qt::AlignLeft);
        ++row;
    }

    mainLayout->addWidget(widget);

    QDialogButtonBox *buttonBox;
    if (successes) {
        buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
        okButton->setDefault(true);
        okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    } else {
        buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        buttonBox->button(QDialogButtonBox::Close)->setDefault(true);
    }
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    exec();
}

void Checksum::ResultsDialog::accept()
{
    if (_onePerFile && _onePerFile->isChecked()) {
        Q_ASSERT(_data.size() > 1);
        if (savePerFile())
            QDialog::accept();
    } else if (!_checksumFileSelector->url().isEmpty()) {
        if (saveChecksum(_data, _checksumFileSelector->url().toDisplayString(QUrl::PreferLocalFile)))
            QDialog::accept();
    }
}

bool Checksum::ResultsDialog::saveChecksum(const QStringList& data, QString filename)
{
    if (QFile::exists(filename) &&
        KMessageBox::warningContinueCancel(
            this, i18n("File %1 already exists.\nAre you sure you want to overwrite it?", filename),
            i18n("Warning"), KStandardGuiItem::overwrite()) != KMessageBox::Continue) {
        // find a better name to save to
        filename = QFileDialog::getSaveFileName(0, i18n("Select a file to save to"), QString(),
                                                QStringLiteral("*"));
        if (filename.simplified().isEmpty())
            return false;
    }

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        for (const QString line : data)
            stream << line << "\n";
        file.close();
    }

    if (file.error() != QFile::NoError) {
        KMessageBox::detailedError(this, i18n("Error saving file %1", filename), file.errorString());
        return false;
    }

    return true;
}

bool Checksum::ResultsDialog::savePerFile()
{
    const QString type = _suggestedFilename.mid(_suggestedFilename.lastIndexOf('.'));

    krApp->startWaiting(i18n("Saving checksum files..."), 0);
    for (const QString line : _data) {
        const QString filename = line.mid(line.indexOf(' ') + 2) + type;
        if (!saveChecksum(QStringList() << line, filename)) {
            KMessageBox::error(this, i18n("Errors occurred while saving multiple checksums. Stopping"));
            krApp->stopWait();
            return false;
        }
    }
    krApp->stopWait();

    return true;
}

void Checksum::startCreation(const QStringList &files, bool containFolders, const QString &path)
{
    Checksum::CreateDialog(files, containFolders, path);
}

void Checksum::startMatch(const QStringList &files, bool containFolders, const QString &path,
                          const QString &checksumFile)
{
    Checksum::MatchDialog(files, containFolders, path, checksumFile);
}
