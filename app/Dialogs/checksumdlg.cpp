/*
    SPDX-FileCopyrightText: 2005 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2007-2008 Csaba Karai <cskarai@freemail.hu>
    SPDX-FileCopyrightText: 2008 Jonas Bähr <jonas.baehr@web.de>
    SPDX-FileCopyrightText: 2005-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "checksumdlg.h"

#include "../GUI/krlistwidget.h"
#include "../GUI/krtreewidget.h"
#include "../icon.h"
#include "../krglobal.h"
#include "../krservices.h"
#include "../krusader.h"

// QtCore
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QMap>
#include <QTextStream>
// QtWidgets
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QProgressBar>

#include <QtConcurrent/QtConcurrentRun> // krazy:exclude=includes

#include <KLocalizedString>
#include <KMessageBox>
#include <KUrlRequester>

void Checksum::startCreationWizard(const QString &path, const QStringList &files)
{
    if (files.isEmpty())
        return;

    QDialog *dialog = new CHECKSUM_::CreateWizard(path, files);
    dialog->show();
}

void Checksum::startVerifyWizard(const QString &path, const QString &checksumFile)
{
    QDialog *dialog = new CHECKSUM_::VerifyWizard(path, checksumFile);
    dialog->show();
}

namespace CHECKSUM_
{

bool stopListFiles;
// async operation invoked by QtConcurrent::run in creation wizard
QStringList listFiles(const QString &path, const QStringList &fileNames)
{
    const QDir baseDir(path);
    QStringList allFiles;
    for (const QString &fileName : fileNames) {
        if (stopListFiles)
            return QStringList();

        QDir subDir = QDir(baseDir.filePath(fileName));
        if (subDir.exists()) {
            subDir.setFilter(QDir::Files);
            QDirIterator it(subDir, QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
            while (it.hasNext()) {
                if (stopListFiles)
                    return QStringList();

                allFiles << baseDir.relativeFilePath(it.next());
            }
        } else {
            // assume this is a file
            allFiles << fileName;
        }
    }
    return allFiles;
}

// ------------- Checksum Process

ChecksumProcess::ChecksumProcess(QObject *parent, const QString &path)
    : KProcess(parent)
    , m_tmpOutFile(QDir::tempPath() + QLatin1String("/krusader_XXXXXX.stdout"))
    , m_tmpErrFile(QDir::tempPath() + QLatin1String("/krusader_XXXXXX.stderr"))
{
    m_tmpOutFile.open(); // necessary to create the filename
    m_tmpErrFile.open(); // necessary to create the filename

    setOutputChannelMode(KProcess::SeparateChannels); // without this the next 2 lines have no effect!
    setStandardOutputFile(m_tmpOutFile.fileName());
    setStandardErrorFile(m_tmpErrFile.fileName());
    setWorkingDirectory(path);
    connect(this, &ChecksumProcess::errorOccurred, this, &ChecksumProcess::slotError);
    connect(this, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &ChecksumProcess::slotFinished);
}

ChecksumProcess::~ChecksumProcess()
{
    disconnect(this, nullptr, this, nullptr); // QProcess emits finished() on destruction
    close();
}

void ChecksumProcess::slotError(QProcess::ProcessError error)
{
    if (error == QProcess::FailedToStart) {
        KMessageBox::error(nullptr, i18n("<qt>Could not start <b>%1</b>.</qt>", program().join(" ")));
    }
}

void ChecksumProcess::slotFinished(int, QProcess::ExitStatus exitStatus)
{
    if (exitStatus != QProcess::NormalExit) {
        KMessageBox::error(nullptr, i18n("<qt>There was an error while running <b>%1</b>.</qt>", program().join(" ")));
        return;
    }

    // parse result files
    if (!KrServices::fileToStringList(&m_tmpOutFile, m_outputLines) || !KrServices::fileToStringList(&m_tmpErrFile, m_errorLines)) {
        KMessageBox::error(nullptr, i18n("Error reading stdout or stderr"));
        return;
    }
    emit resultReady();
}

// ------------- Generic Checksum Wizard

ChecksumWizard::ChecksumWizard(const QString &path)
    : QWizard(krApp)
    , m_path(path)
    , m_process(nullptr)
{
    setAttribute(Qt::WA_DeleteOnClose);

    // init the dictionary - pity it has to be manually
    m_checksumTools.insert("md5", "md5sum");
    m_checksumTools.insert("sha1", "sha1sum");
    m_checksumTools.insert("sha256", "sha256sum");
    m_checksumTools.insert("sha224", "sha224sum");
    m_checksumTools.insert("sha384", "sha384sum");
    m_checksumTools.insert("sha512", "sha512sum");

    connect(this, &QWizard::currentIdChanged, this, &ChecksumWizard::slotCurrentIdChanged);
}

ChecksumWizard::~ChecksumWizard()
{
    if (m_process) {
        delete m_process;
    }
}

void ChecksumWizard::slotCurrentIdChanged(int id)
{
    if (id == m_introId) {
        onIntroPage();
    } else if (id == m_progressId) {
        if (m_process) {
            // we are coming from the result page;
            delete m_process;
            m_process = nullptr;
            restart();
        } else {
            button(QWizard::BackButton)->hide();
            button(QWizard::NextButton)->hide();
            onProgressPage();
        }
    } else if (id == m_resultId) {
        onResultPage();
    }
}

QWizardPage *ChecksumWizard::createProgressPage(const QString &title)
{
    auto *page = new QWizardPage;

    page->setTitle(title);
    page->setPixmap(QWizard::LogoPixmap, Icon("process-working").pixmap(32));
    page->setSubTitle(i18n("Please wait..."));

    auto *mainLayout = new QVBoxLayout;
    page->setLayout(mainLayout);

    // "busy" indicator
    auto *bar = new QProgressBar();
    bar->setRange(0, 0);

    mainLayout->addWidget(bar);

    return page;
}

bool ChecksumWizard::checkExists(const QString &type)
{
    if (!KrServices::cmdExist(m_checksumTools[type])) {
        KMessageBox::error(this,
                           i18n("<qt>Krusader cannot find a checksum tool that handles %1 on your system. "
                                "Please check the <b>Dependencies</b> page in Krusader's settings.</qt>",
                                type));
        return false;
    }
    return true;
}

void ChecksumWizard::runProcess(const QString &type, const QStringList &args)
{
    Q_ASSERT(m_process == nullptr);

    m_process = new ChecksumProcess(this, m_path);
    m_process->setProgram(KrServices::fullPathName(m_checksumTools[type]), args);
    // show next page (with results) (only) when process is done
    connect(m_process, &ChecksumProcess::resultReady, this, &QWizard::next);
    // run the process
    m_process->start();
}

void ChecksumWizard::addChecksumLine(KrTreeWidget *tree, const QString &line)
{
    auto *item = new QTreeWidgetItem(tree);
    const qsizetype hashLength = line.indexOf(' '); // delimiter is either "  " or " *"
    item->setText(0, line.left(hashLength));
    QString fileName = line.mid(hashLength + 2);
    if (fileName.endsWith('\n'))
        fileName.chop(1);
    item->setText(1, fileName);
}

// ------------- Create Wizard

CreateWizard::CreateWizard(const QString &path, const QStringList &_files)
    : ChecksumWizard(path)
    , m_fileNames(_files)
{
    m_introId = addPage(createIntroPage());
    m_progressId = addPage(createProgressPage(i18n("Creating Checksums")));
    m_resultId = addPage(createResultPage());

    setButton(QWizard::FinishButton, QDialogButtonBox(QDialogButtonBox::Save).button(QDialogButtonBox::Save));

    connect(&m_listFilesWatcher, &QFutureWatcher<QStringList>::resultReadyAt, this, &CreateWizard::createChecksums);
}

QWizardPage *CreateWizard::createIntroPage()
{
    auto *page = new QWizardPage;

    page->setTitle(i18n("Create Checksums"));
    page->setPixmap(QWizard::LogoPixmap, Icon("document-edit-sign").pixmap(32));
    page->setSubTitle(i18n("About to calculate checksum for the following files or folders:"));

    auto *mainLayout = new QVBoxLayout;
    page->setLayout(mainLayout);

    // file list
    auto *listWidget = new KrListWidget;
    listWidget->addItems(m_fileNames);
    mainLayout->addWidget(listWidget);

    // checksum method
    auto *hLayout = new QHBoxLayout;

    QLabel *methodLabel = new QLabel(i18n("Select the checksum method:"));
    hLayout->addWidget(methodLabel);

    m_methodBox = new KComboBox;
    // -- fill the combo with available methods
    for (const QString &type : m_checksumTools.keys())
        m_methodBox->addItem(type);
    m_methodBox->setFocus();
    hLayout->addWidget(m_methodBox);

    mainLayout->addLayout(hLayout);

    return page;
}

QWizardPage *CreateWizard::createResultPage()
{
    auto *page = new QWizardPage;

    page->setTitle(i18n("Checksum Results"));

    auto *mainLayout = new QVBoxLayout;
    page->setLayout(mainLayout);

    m_hashesTreeWidget = new KrTreeWidget(this);
    m_hashesTreeWidget->setAllColumnsShowFocus(true);
    m_hashesTreeWidget->setHeaderLabels(QStringList() << i18n("Hash") << i18n("File"));
    mainLayout->addWidget(m_hashesTreeWidget);

    m_errorLabel = new QLabel(i18n("Errors received:"));
    mainLayout->addWidget(m_errorLabel);

    m_errorListWidget = new KrListWidget;
    mainLayout->addWidget(m_errorListWidget);

    m_onePerFileBox = new QCheckBox(i18n("Save one checksum file for each source file"));
    m_onePerFileBox->setChecked(false);
    mainLayout->addWidget(m_onePerFileBox);

    return page;
}

void CreateWizard::onIntroPage()
{
    button(QWizard::NextButton)->show();
}

void CreateWizard::onProgressPage()
{
    // first, get all files (recurse in directories) - async
    stopListFiles = false; // QFuture cannot cancel QtConcurrent::run
    connect(this, &CreateWizard::finished, this, [=]() {
        stopListFiles = true;
    });
    QFuture<QStringList> listFuture = QtConcurrent::run(listFiles, m_path, m_fileNames);
    m_listFilesWatcher.setFuture(listFuture);
}

void CreateWizard::createChecksums()
{
    const QString type = m_methodBox->currentText();
    if (!checkExists(type)) {
        button(QWizard::BackButton)->show();
        return;
    }

    const QStringList &allFiles = m_listFilesWatcher.result();
    if (allFiles.isEmpty()) {
        KMessageBox::error(this, i18n("No files found"));
        button(QWizard::BackButton)->show();
        return;
    }

    runProcess(type, allFiles);

    // set suggested filename
    m_suggestedFilePath = QDir(m_path).filePath((m_fileNames.count() > 1 ? QString("checksum.") : (m_fileNames[0] + '.')) + type);
}

void CreateWizard::onResultPage()
{
    // hash tools display errors into stderr, so we'll use that to determine the result of the job
    const QStringList outputLines = m_process->stdOutput();
    const QStringList errorLines = m_process->errOutput();
    bool errors = !errorLines.isEmpty();
    bool successes = !outputLines.isEmpty();

    QWizardPage *page = currentPage();
    page->setPixmap(QWizard::LogoPixmap, Icon(errors || !successes ? "dialog-error" : "dialog-information").pixmap(32));
    page->setSubTitle(errors || !successes ? i18n("Errors were detected while creating the checksums") : i18n("Checksums were created successfully"));

    m_hashesTreeWidget->clear();
    m_hashesTreeWidget->setVisible(successes);
    if (successes) {
        for (const QString &line : outputLines)
            addChecksumLine(m_hashesTreeWidget, line);
        // m_hashesTreeWidget->sortItems(1, Qt::AscendingOrder);
    }

    m_errorLabel->setVisible(errors);
    m_errorListWidget->setVisible(errors);
    m_errorListWidget->clear();
    m_errorListWidget->addItems(errorLines);

    m_onePerFileBox->setEnabled(outputLines.size() > 1);

    button(QWizard::FinishButton)->setEnabled(successes);
}

bool CreateWizard::savePerFile()
{
    const QString type = m_suggestedFilePath.mid(m_suggestedFilePath.lastIndexOf('.'));

    krApp->startWaiting(i18n("Saving checksum files..."), 0);
    for (const QString &line : m_process->stdOutput()) {
        const QString filename = line.mid(line.indexOf(' ') + 2) + type;
        if (!saveChecksumFile(QStringList() << line, filename)) {
            KMessageBox::error(this, i18n("Errors occurred while saving multiple checksums. Stopping"));
            krApp->stopWait();
            return false;
        }
    }
    krApp->stopWait();

    return true;
}

bool CreateWizard::saveChecksumFile(const QStringList &data, const QString &filename)
{
    QString filePath = filename.isEmpty() ? m_suggestedFilePath : filename;
    if (filename.isEmpty() || QFile::exists(filePath)) {
        filePath = QFileDialog::getSaveFileName(this, QString(), filePath);
        if (filePath.isEmpty())
            return false; // user pressed cancel
    }

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        for (const QString &line : data)
            stream << line << "\n";
        file.close();
    }

    if (file.error() != QFile::NoError) {
        KMessageBox::detailedError(this, i18n("Error saving file %1", filePath), file.errorString());
        return false;
    }

    return true;
}

void CreateWizard::accept()
{
    const bool saved = m_onePerFileBox->isChecked() ? savePerFile() : saveChecksumFile(m_process->stdOutput());
    if (saved)
        QWizard::accept();
}

// ------------- Verify Wizard

VerifyWizard::VerifyWizard(const QString &path, const QString &inputFile)
    : ChecksumWizard(path)
{
    m_checksumFile = isSupported(inputFile) ? inputFile : path;

    m_introId = addPage(createIntroPage()); // m_checksumFile must already be set
    m_progressId = addPage(createProgressPage(i18n("Verifying Checksums")));
    m_resultId = addPage(createResultPage());
}

void VerifyWizard::slotChecksumPathChanged(const QString &path)
{
    m_hashesTreeWidget->clear();
    button(QWizard::NextButton)->setEnabled(false);

    if (!isSupported(path))
        return;

    m_checksumFile = path;

    // parse and display checksum file content; only for the user, parsed values are not used
    m_hashesTreeWidget->clear();
    QFile file(m_checksumFile);
    if (file.open(QFile::ReadOnly)) {
        QTextStream inStream(&file);
        while (!inStream.atEnd()) {
            addChecksumLine(m_hashesTreeWidget, file.readLine());
        }
    }
    file.close();

    button(QWizard::NextButton)->setEnabled(true);
}

QWizardPage *VerifyWizard::createIntroPage()
{
    auto *page = new QWizardPage;

    page->setTitle(i18n("Verify Checksum File"));
    page->setPixmap(QWizard::LogoPixmap, Icon("document-edit-verify").pixmap(32));
    page->setSubTitle(i18n("About to verify the following checksum file"));

    auto *mainLayout = new QVBoxLayout;
    page->setLayout(mainLayout);

    // checksum file
    auto *hLayout = new QHBoxLayout;
    QLabel *checksumFileLabel = new QLabel(i18n("Checksum file:"));
    hLayout->addWidget(checksumFileLabel);

    auto *checksumFileReq = new KUrlRequester;
    QString typesFilter;
    for (const QString &ext : m_checksumTools.keys())
        typesFilter += ("*." + ext + ' ');
    checksumFileReq->setNameFilter(typesFilter);
    checksumFileReq->setText(m_checksumFile);
    checksumFileReq->setFocus();
    connect(checksumFileReq, &KUrlRequester::textChanged, this, &VerifyWizard::slotChecksumPathChanged);
    hLayout->addWidget(checksumFileReq);

    mainLayout->addLayout(hLayout);

    // content of checksum file
    m_hashesTreeWidget = new KrTreeWidget(page);
    m_hashesTreeWidget->setAllColumnsShowFocus(true);
    m_hashesTreeWidget->setHeaderLabels(QStringList() << i18n("Hash") << i18n("File"));
    mainLayout->addWidget(m_hashesTreeWidget);

    return page;
}

QWizardPage *VerifyWizard::createResultPage()
{
    auto *page = new QWizardPage;

    page->setTitle(i18n("Verify Result"));

    auto *mainLayout = new QVBoxLayout;
    page->setLayout(mainLayout);

    m_outputLabel = new QLabel(i18n("Result output:"));
    mainLayout->addWidget(m_outputLabel);

    m_outputListWidget = new KrListWidget;
    mainLayout->addWidget(m_outputListWidget);

    return page;
}

void VerifyWizard::onIntroPage()
{
    // cannot do this in constructor: NextButton->hide() is overridden
    slotChecksumPathChanged(m_checksumFile);
}

void VerifyWizard::onProgressPage()
{
    // verify checksum file...
    const QString extension = QFileInfo(m_checksumFile).suffix();
    if (!checkExists(extension)) {
        button(QWizard::BackButton)->show();
        return;
    }

    runProcess(extension,
               QStringList() << "--strict"
                             << "-c" << m_checksumFile);
}

void VerifyWizard::onResultPage()
{
    // better not only trust error output
    const bool errors = m_process->exitCode() != 0 || !m_process->errOutput().isEmpty();

    QWizardPage *page = currentPage();
    page->setPixmap(QWizard::LogoPixmap, Icon(errors ? "dialog-error" : "dialog-information").pixmap(32));
    page->setSubTitle(errors ? i18n("Errors were detected while verifying the checksums") : i18n("Checksums were verified successfully"));

    // print everything, errors first
    m_outputListWidget->clear();
    m_outputListWidget->addItems(m_process->errOutput() + m_process->stdOutput());

    button(QWizard::FinishButton)->setEnabled(!errors);
}

bool VerifyWizard::isSupported(const QString &path)
{
    const QFileInfo fileInfo(path);
    return fileInfo.isFile() && m_checksumTools.contains(fileInfo.suffix());
}

} // NAMESPACE CHECKSUM_
