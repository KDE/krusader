/*
    SPDX-FileCopyrightText: 2005 Dirk Eschler <deschler@users.sourceforge.net>
    SPDX-FileCopyrightText: 2005-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krresulttable.h"

#include <iostream>

// QtCore
#include <QList>
// QtGui
#include <QFontDatabase>
#include <QGuiApplication>
// QtWidgets
#include <QGridLayout>
#include <QLabel>

#include <KLocalizedString>
#include <KIO/JobUiDelegate>
#include <KIO/OpenUrlJob>
#include <kio_version.h>

#include "../Archive/krarchandler.h"
#include "../krservices.h"

using namespace std;

#define PS(x) (_supported.contains(x))

KrResultTable::KrResultTable(QWidget *parent)
    : QWidget(parent)
    , _numRows(1)
{
}

KrResultTable::~KrResultTable() = default;

QGridLayout *KrResultTable::initTable()
{
    _grid = new QGridLayout(this);
    _grid->setColumnStretch(static_cast<int>(_numColumns - 1), 1); // stretch last column
    _grid->setContentsMargins(0, 0, 0, 0);
    _grid->setSpacing(0);

    // +++ Build and add table header +++
    int column = 0;
    for (auto &_tableHeader : _tableHeaders) {
        _label = new QLabel(_tableHeader, this);
        _label->setContentsMargins(5, 5, 5, 5);
        _grid->addWidget(_label, 0, column);

        // Set font
        QFont defFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
        defFont.setPointSize(defFont.pointSize() - 1);
        defFont.setBold(true);
        _label->setFont(defFont);

        ++column;
    }

    return _grid;
}

void KrResultTable::adjustRow(QGridLayout *grid)
{
    int i = 0;
    QLayoutItem *child;
    int col = 0;

    while ((child = grid->itemAt(i)) != nullptr) {
        // Add some space between columns
        child->widget()->setMinimumWidth(child->widget()->sizeHint().width() + 15);

        // Paint uneven rows in alternate color
        if (((col / _numColumns) % 2)) {
            child->widget()->setAutoFillBackground(true);
            QPalette p = QGuiApplication::palette();
            QPalette pal = child->widget()->palette();
            pal.setColor(child->widget()->backgroundRole(), p.color(QPalette::Active, QPalette::AlternateBase));
            child->widget()->setPalette(pal);
        }

        ++i;
        ++col;
    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

KrArchiverResultTable::KrArchiverResultTable(QWidget *parent)
    : KrResultTable(parent)
{
    _supported = KrArcHandler::supportedPackers(); // get list of available packers

    Archiver *tar = new Archiver("tar", "https://www.gnu.org/", PS("tar"), true, true);
    Archiver *gzip = new Archiver("gzip", "https://www.gnu.org/", PS("gzip"), true, true);
    Archiver *bzip2 = new Archiver("bzip2", "https://www.gnu.org/", PS("bzip2"), true, true);
    Archiver *lzma = new Archiver("lzma", "https://tukaani.org/lzma/", PS("lzma"), true, true);
    Archiver *xz = new Archiver("xz", "https://tukaani.org/xz/", PS("xz"), true, true);
    Archiver *lha = new Archiver("lha", "https://www.gnu.org/", PS("lha"), true, true);
    Archiver *zip = new Archiver("zip", "http://www.info-zip.org", PS("zip"), true, false);
    Archiver *unzip = new Archiver("unzip", "http://www.info-zip.org", PS("unzip"), false, true);
    Archiver *arj = new Archiver("arj", "http://www.arjsoftware.com", PS("arj"), true, true);
    Archiver *unarj = new Archiver("unarj", "http://www.arjsoftware.com", PS("unarj"), false, true);
    Archiver *unace = new Archiver("unace", "https://web.archive.org/web/20170714193504/http://winace.com/", PS("unace"), false, true);
    Archiver *rar = new Archiver("rar", "https://www.rarlab.com/", PS("rar"), true, true);
    Archiver *unrar = new Archiver("unrar", "https://www.rarlab.com/", PS("unrar"), false, true);
    Archiver *rpm = new Archiver("rpm", "https://www.gnu.org/", PS("rpm"), false, true);
    Archiver *dpkg = new Archiver("dpkg", "https://www.dpkg.org/", PS("dpkg"), false, true);
    Archiver *_7z = new Archiver("7z", "https://www.7-zip.org/", PS("7z"), true, true);

    // Special case: arj can unpack, but unarj is preferred
    if (PS("arj") && PS("unarj"))
        arj->setIsUnpacker(false);
    if (PS("arj") && !PS("unarj"))
        unarj->setNote(i18n("unarj not found, but arj found, which will be used for unpacking"));
    // Special case: rar can unpack, but unrar is preferred
    if (PS("rar") && PS("unrar"))
        rar->setIsUnpacker(false);
    // Special case: rpm needs cpio for unpacking
    if (PS("rpm") && !PS("cpio"))
        rpm->setNote(i18n("rpm found, but cpio not found which is required for unpacking"));

    _tableHeaders.append(i18n("Name"));
    _tableHeaders.append(i18n("Found"));
    _tableHeaders.append(i18n("Packing"));
    _tableHeaders.append(i18n("Unpacking"));
    _tableHeaders.append(i18n("Note"));
    _numColumns = _tableHeaders.size();

    _grid = initTable();

    addRow(tar, _grid);
    addRow(gzip, _grid);
    addRow(bzip2, _grid);
    addRow(lzma, _grid);
    addRow(xz, _grid);
    addRow(lha, _grid);
    addRow(zip, _grid);
    addRow(unzip, _grid);
    addRow(arj, _grid);
    addRow(unarj, _grid);
    addRow(unace, _grid);
    addRow(rar, _grid);
    addRow(unrar, _grid);
    addRow(rpm, _grid);
    addRow(dpkg, _grid);
    addRow(_7z, _grid);

    delete tar;
    delete gzip;
    delete bzip2;
    delete lzma;
    delete xz;
    delete lha;
    delete zip;
    delete unzip;
    delete arj;
    delete unarj;
    delete unace;
    delete rar;
    delete unrar;
    delete rpm;
    delete dpkg;
    delete _7z;
}

KrArchiverResultTable::~KrArchiverResultTable() = default;

bool KrArchiverResultTable::addRow(SearchObject *search, QGridLayout *grid)
{
    auto *arch = dynamic_cast<Archiver *>(search);

    // Name column
    KUrlLabel *urlLabel = new KUrlLabel(arch->getWebsite(), arch->getSearchName(), this);
    urlLabel->setContentsMargins(5, 5, 5, 5);
    urlLabel->setAlignment(Qt::AlignTop);
    grid->addWidget(urlLabel, _numRows, 0);
    connect(urlLabel, QOverload<>::of(&KUrlLabel::leftClickedUrl), this, [this, urlLabel]() {
        KrArchiverResultTable::website(urlLabel->url());
    });
    _label = urlLabel;

    // Found column
    _label = new QLabel(arch->getPath(), this);
    _label->setContentsMargins(5, 5, 5, 5);
    grid->addWidget(_label, _numRows, 1);

    // Packing column
    _label = new QLabel(this);
    _label->setContentsMargins(5, 5, 5, 5);
    _label->setAlignment(Qt::AlignTop);
    if (arch->getIsPacker() && arch->getFound()) {
        _label->setText(i18n("enabled"));
        QPalette pal = _label->palette();
        pal.setColor(_label->foregroundRole(), "darkgreen");
        _label->setPalette(pal);
    } else if (arch->getIsPacker() && !arch->getFound()) {
        _label->setText(i18n("disabled"));
        QPalette pal = _label->palette();
        pal.setColor(_label->foregroundRole(), "red");
        _label->setPalette(pal);
    } else
        _label->setText("");
    grid->addWidget(_label, _numRows, 2);

    // Unpacking column
    _label = new QLabel(this);
    _label->setContentsMargins(5, 5, 5, 5);
    _label->setAlignment(Qt::AlignTop);
    if (arch->getIsUnpacker() && arch->getFound()) {
        _label->setText(i18n("enabled"));
        QPalette pal = _label->palette();
        pal.setColor(_label->foregroundRole(), "darkgreen");
        _label->setPalette(pal);
    } else if (arch->getIsUnpacker() && !arch->getFound()) {
        _label->setText(i18n("disabled"));
        QPalette pal = _label->palette();
        pal.setColor(_label->foregroundRole(), "red");
        _label->setPalette(pal);
    } else
        _label->setText("");
    grid->addWidget(_label, _numRows, 3);

    // Note column
    _label = new QLabel(arch->getNote(), this);
    _label->setContentsMargins(5, 5, 5, 5);
    _label->setAlignment(Qt::AlignTop);
    _label->setWordWrap(true); // wrap words
    grid->addWidget(_label, _numRows, 4);

    // Apply shared design elements
    adjustRow(_grid);

    // Ensure the last column takes more space
    _label->setMinimumWidth(300);

    ++_numRows;
    return true;
}

void KrArchiverResultTable::website(const QString &url)
{
    auto *job = new KIO::OpenUrlJob(QUrl(url), this);
    job->start();
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

KrToolResultTable::KrToolResultTable(QWidget *parent)
    : KrResultTable(parent)
{
    _supported = KrServices::supportedTools(); // get list of available tools

    QList<Application *> vecDiff, vecMail, vecRename, vecChecksum;
    Application *kdiff3 = new Application("kdiff3", "https://www.kde.org/applications/development/kdiff3/", KrServices::cmdExist("kdiff3"));
    Application *kompare = new Application("kompare", "https://www.kde.org/applications/development/kompare/", KrServices::cmdExist("kompare"));
    Application *xxdiff = new Application("xxdiff", "http://furius.ca/xxdiff/", KrServices::cmdExist("xxdiff"));
    Application *thunderbird = new Application("thunderbird", "https://www.thunderbird.net/", KrServices::cmdExist("thunderbird"));
    Application *kmail = new Application("kmail", "https://kontact.kde.org/components/kmail.html", KrServices::cmdExist("kmail"));
    Application *krename = new Application("krename", "https://www.kde.org/applications/utilities/krename/", KrServices::cmdExist("krename"));
    Application *md5sum = new Application("md5sum", "https://www.gnu.org/software/textutils/textutils.html", KrServices::cmdExist("md5sum"));

    vecDiff.push_back(kdiff3);
    vecDiff.push_back(kompare);
    vecDiff.push_back(xxdiff);
    vecMail.push_back(thunderbird);
    vecMail.push_back(kmail);
    vecRename.push_back(krename);
    vecChecksum.push_back(md5sum);

    ApplicationGroup *diff = new ApplicationGroup(i18n("diff utility"), PS("DIFF"), vecDiff);
    ApplicationGroup *mail = new ApplicationGroup(i18n("email client"), PS("MAIL"), vecMail);
    ApplicationGroup *rename = new ApplicationGroup(i18n("batch renamer"), PS("RENAME"), vecRename);
    ApplicationGroup *checksum = new ApplicationGroup(i18n("checksum utility"), PS("MD5"), vecChecksum);

    _tableHeaders.append(i18n("Group"));
    _tableHeaders.append(i18n("Tool"));
    _tableHeaders.append(i18n("Found"));
    _tableHeaders.append(i18n("Status"));
    _numColumns = _tableHeaders.size();

    _grid = initTable();

    addRow(diff, _grid);
    addRow(mail, _grid);
    addRow(rename, _grid);
    addRow(checksum, _grid);

    delete thunderbird;
    delete kmail;
    delete kompare;
    delete kdiff3;
    delete xxdiff;
    delete krename;
    delete md5sum;

    delete diff;
    delete mail;
    delete rename;
    delete checksum;
}

KrToolResultTable::~KrToolResultTable() = default;

bool KrToolResultTable::addRow(SearchObject *search, QGridLayout *grid)
{
    auto *appGroup = dynamic_cast<ApplicationGroup *>(search);
    QList<Application *> _apps = appGroup->getAppVec();

    // Name column
    _label = new QLabel(appGroup->getSearchName(), this);
    _label->setContentsMargins(5, 5, 5, 5);
    _label->setAlignment(Qt::AlignTop);
    grid->addWidget(_label, _numRows, 0);

    // Tool column
    QWidget *toolBoxWidget = new QWidget(this);
    auto *toolBox = new QVBoxLayout(toolBoxWidget);

    for (auto &_app : _apps) {
        auto *l = new KUrlLabel(_app->getWebsite(), _app->getAppName(), toolBoxWidget);
        toolBox->addWidget(l);

        l->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        l->setContentsMargins(5, 5, 5, 5);
        connect(l, QOverload<>::of(&KUrlLabel::leftClickedUrl), this, [this, l]() {
            KrToolResultTable::website(l->url());
        });
    }
    grid->addWidget(toolBoxWidget, _numRows, 1);

    // Found column
    QWidget *vboxWidget = new QWidget(this);
    auto *vbox = new QVBoxLayout(vboxWidget);

    for (auto &_app : _apps) {
        _label = new QLabel(_app->getPath(), vboxWidget);
        _label->setContentsMargins(5, 5, 5, 5);
        _label->setAlignment(Qt::AlignTop);
        vbox->addWidget(_label);
    }
    grid->addWidget(vboxWidget, _numRows, 2);

    // Status column
    _label = new QLabel(this);
    _label->setContentsMargins(5, 5, 5, 5);
    _label->setAlignment(Qt::AlignTop);
    if (appGroup->getFoundGroup()) {
        _label->setText(i18n("enabled"));
        QPalette pal = _label->palette();
        pal.setColor(_label->foregroundRole(), "darkgreen");
        _label->setPalette(pal);
    } else {
        _label->setText(i18n("disabled"));
        QPalette pal = _label->palette();
        pal.setColor(_label->foregroundRole(), "red");
        _label->setPalette(pal);
    }
    grid->addWidget(_label, _numRows, 3);

    // Apply shared design elements
    adjustRow(_grid);

    // Ensure the last column takes more space
    _label->setMinimumWidth(300);

    ++_numRows;
    return true;
}

void KrToolResultTable::website(const QString &url)
{
    auto *job = new KIO::OpenUrlJob(QUrl(url), this);
    job->start();
}
