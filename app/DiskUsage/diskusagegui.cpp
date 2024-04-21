/*
    SPDX-FileCopyrightText: 2004 Csaba Karai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "diskusagegui.h"

// QtCore
#include <QTimer>
// QtGui
#include <QResizeEvent>
// QtWidgets
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>

#include <KLocalizedString>
#include <KSharedConfig>

#include "../Dialogs/krdialogs.h"
#include "../FileSystem/filesystem.h"
#include "../compat.h"
#include "../icon.h"
#include "../krglobal.h"

DiskUsageGUI::DiskUsageGUI(const QUrl &openDir)
    : QDialog(nullptr)
    , exitAtFailure(true)
{
    setWindowTitle(i18n("Krusader::Disk Usage"));
    setAttribute(Qt::WA_DeleteOnClose);

    baseDirectory = openDir;

    auto *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    auto *duGrid = new QGridLayout();
    duGrid->setSpacing(6);
    duGrid->setContentsMargins(11, 11, 11, 11);

    QWidget *duTools = new QWidget(this);
    auto *duHBox = new QHBoxLayout(duTools);
    duHBox->setContentsMargins(0, 0, 0, 0);
    duTools->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    btnNewSearch = new QToolButton(duTools);
    btnNewSearch->setIcon(Icon("document-open"));
    duHBox->addWidget(btnNewSearch);
    btnNewSearch->setToolTip(i18n("Start new disk usage search"));

    btnRefresh = new QToolButton(duTools);
    btnRefresh->setIcon(Icon("view-refresh"));
    duHBox->addWidget(btnRefresh);
    btnRefresh->setToolTip(i18n("Refresh"));

    btnDirUp = new QToolButton(duTools);
    btnDirUp->setIcon(Icon("go-up"));
    duHBox->addWidget(btnDirUp);
    btnDirUp->setToolTip(i18n("Parent folder"));

    QWidget *separatorWidget = new QWidget(duTools);
    separatorWidget->setMinimumWidth(10);
    duHBox->addWidget(separatorWidget);

    btnLines = new QToolButton(duTools);
    btnLines->setIcon(Icon("view-list-details"));
    btnLines->setCheckable(true);
    duHBox->addWidget(btnLines);
    btnLines->setToolTip(i18n("Line view"));

    btnDetailed = new QToolButton(duTools);
    btnDetailed->setIcon(Icon("view-list-tree"));
    btnDetailed->setCheckable(true);
    duHBox->addWidget(btnDetailed);
    btnDetailed->setToolTip(i18n("Detailed view"));

    btnFilelight = new QToolButton(duTools);
    btnFilelight->setIcon(Icon("kr_diskusage"));
    btnFilelight->setCheckable(true);
    duHBox->addWidget(btnFilelight);
    btnFilelight->setToolTip(i18n("Filelight view"));

    QWidget *spacerWidget = new QWidget(duTools);
    duHBox->addWidget(spacerWidget);
    auto *hboxlayout = new QHBoxLayout(spacerWidget);
    auto *spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed);
    hboxlayout->addItem(spacer);

    duGrid->addWidget(duTools, 0, 0);

    diskUsage = new DiskUsage("DiskUsage", this);
    duGrid->addWidget(diskUsage, 1, 0);

    status = new KSqueezedTextLabel(this);
    duGrid->addWidget(status, 2, 0);

    mainLayout->addLayout(duGrid);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::rejected, this, &DiskUsageGUI::close);
    connect(diskUsage, &DiskUsage::status, this, &DiskUsageGUI::slotStatus);
    connect(diskUsage, &DiskUsage::viewChanged, this, &DiskUsageGUI::slotViewChanged);
    connect(diskUsage, &DiskUsage::newSearch, this, &DiskUsageGUI::askDir);
    connect(diskUsage, &DiskUsage::loadFinished, this, &DiskUsageGUI::slotLoadFinished);
    connect(btnNewSearch, &QToolButton::clicked, this, &DiskUsageGUI::askDir);
    connect(btnRefresh, &QToolButton::clicked, this, &DiskUsageGUI::slotLoadUsageInfo);
    connect(btnDirUp, &QToolButton::clicked, diskUsage, &DiskUsage::dirUp);
    connect(btnLines, &QToolButton::clicked, this, &DiskUsageGUI::slotSelectLinesView);
    connect(btnDetailed, &QToolButton::clicked, this, &DiskUsageGUI::slotSelectListView);
    connect(btnFilelight, &QToolButton::clicked, this, &DiskUsageGUI::slotSelectFilelightView);

    KConfigGroup group(krConfig, "DiskUsage");

    int view = group.readEntry("View", VIEW_LINES);
    if (view < VIEW_LINES || view > VIEW_FILELIGHT)
        view = VIEW_LINES;

    diskUsage->setView(view);

    sizeX = group.readEntry("Window Width", QFontMetrics(font()).horizontalAdvance("W") * 70);
    sizeY = group.readEntry("Window Height", QFontMetrics(font()).height() * 25);
    resize(sizeX, sizeY);

    if (group.readEntry("Window Maximized", false)) {
        setWindowState(windowState() | Qt::WindowMaximized);
    }
}

void DiskUsageGUI::askDirAndShow()
{
    if (askDir()) {
        show();
    }
}

void DiskUsageGUI::slotLoadFinished(bool result)
{
    if (exitAtFailure && !result) {
        close();
    } else {
        exitAtFailure = false;
    }
}

void DiskUsageGUI::enableButtons(bool isOn)
{
    btnNewSearch->setEnabled(isOn);
    btnRefresh->setEnabled(isOn);
    btnDirUp->setEnabled(isOn);
    btnLines->setEnabled(isOn);
    btnDetailed->setEnabled(isOn);
    btnFilelight->setEnabled(isOn);
}

void DiskUsageGUI::resizeEvent(QResizeEvent *e)
{
    if (!isMaximized()) {
        sizeX = e->size().width();
        sizeY = e->size().height();
    }
    QDialog::resizeEvent(e);
}

void DiskUsageGUI::closeEvent(QCloseEvent *event)
{
    KConfigGroup group(krConfig, "DiskUsage");
    group.writeEntry("Window Width", sizeX);
    group.writeEntry("Window Height", sizeY);
    group.writeEntry("Window Maximized", isMaximized());
    group.writeEntry("View", diskUsage->getActiveView());

    event->accept();
}

void DiskUsageGUI::slotLoadUsageInfo()
{
    diskUsage->load(baseDirectory);
}

void DiskUsageGUI::slotStatus(const QString &stat)
{
    status->setText(stat);
}

void DiskUsageGUI::slotViewChanged(int view)
{
    if (view == VIEW_LOADER) {
        enableButtons(false);
        return;
    }
    enableButtons(true);

    btnLines->setChecked(false);
    btnDetailed->setChecked(false);
    btnFilelight->setChecked(false);

    switch (view) {
    case VIEW_LINES:
        btnLines->setChecked(true);
        break;
    case VIEW_DETAILED:
        btnDetailed->setChecked(true);
        break;
    case VIEW_FILELIGHT:
        btnFilelight->setChecked(true);
        break;
    case VIEW_LOADER:
        break;
    }
}

bool DiskUsageGUI::askDir()
{
    // ask the user for the copy destX
    const QUrl newDir = KChooseDir::getDir(i18n("Viewing the usage of folder:"), baseDirectory, baseDirectory);

    if (newDir.isEmpty())
        return false;

    baseDirectory = newDir;

    QTimer::singleShot(0, this, &DiskUsageGUI::slotLoadUsageInfo);
    return true;
}
