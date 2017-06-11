/*****************************************************************************
 * Copyright (C) 2003 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2003 Rafi Yanai <yanai@users.sourceforge.net>               *
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

#include "panelpopup.h"

#include "krfiletreeview.h"
#include "krpanel.h"
#include "panelfunc.h"
#include "viewactions.h"
#include "../defaults.h"
#include "../kicons.h"
#include "../Dialogs/krsqueezedtextlabel.h"
#include "../FileSystem/fileitem.h"
#include "../FileSystem/filesystem.h"
#include "../KViewer/diskusageviewer.h"
#include "../KViewer/panelviewer.h"
#include "PanelView/krview.h"
#include "PanelView/krviewitem.h"

// QtCore
#include <QMimeDatabase>
#include <QMimeType>
// QtWidgets
#include <QAbstractItemView>
#include <QGridLayout>

#include <KConfigCore/KSharedConfig>
#include <KI18n/KLocalizedString>
#include <KIconThemes/KIconLoader>

PanelPopup::PanelPopup(QWidget *parent) : QWidget(parent), stack(0), imageFilePreview(0), pjob(0)
{
    QGridLayout * layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // create the label+buttons setup
    dataLine = new KrSqueezedTextLabel(this);
    KConfigGroup lg(krConfig, "Look&Feel");
    dataLine->setFont(lg.readEntry("Filelist Font", _FilelistFont));
    // --- hack: setup colors to be the same as an inactive panel
    dataLine->setBackgroundRole(QPalette::Window);
    int sheight = QFontMetrics(dataLine->font()).height() + 4;
    dataLine->setMaximumHeight(sheight);

    btns = new QButtonGroup(this);
    btns->setExclusive(true);
    connect(btns, SIGNAL(buttonClicked(int)), this, SLOT(tabSelected(int)));

    treeBtn = new QToolButton(this);
    treeBtn->setToolTip(i18n("Tree Panel: a tree view of the local file system"));
    treeBtn->setIcon(krLoader->loadIcon("view-list-tree", KIconLoader::Toolbar, 16));
    treeBtn->setFixedSize(20, 20);
    treeBtn->setCheckable(true);
    btns->addButton(treeBtn, Tree);

    previewBtn = new QToolButton(this);
    previewBtn->setToolTip(i18n("Preview Panel: display a preview of the current file"));
    previewBtn->setIcon(krLoader->loadIcon("view-preview", KIconLoader::Toolbar, 16));
    previewBtn->setFixedSize(20, 20);
    previewBtn->setCheckable(true);
    btns->addButton(previewBtn, Preview);

    viewerBtn = new QToolButton(this);
    viewerBtn->setToolTip(i18n("View Panel: view the current file"));
    viewerBtn->setIcon(krLoader->loadIcon("zoom-original", KIconLoader::Toolbar, 16));
    viewerBtn->setFixedSize(20, 20);
    viewerBtn->setCheckable(true);
    btns->addButton(viewerBtn, View);

    duBtn = new QToolButton(this);
    duBtn->setToolTip(i18n("Disk Usage Panel: view the usage of a folder"));
    duBtn->setIcon(krLoader->loadIcon("kr_diskusage", KIconLoader::Toolbar, 16));
    duBtn->setFixedSize(20, 20);
    duBtn->setCheckable(true);
    btns->addButton(duBtn, DskUsage);

    layout->addWidget(dataLine, 0, 0);
    layout->addWidget(treeBtn, 0, 1);
    layout->addWidget(previewBtn, 0, 2);
    layout->addWidget(viewerBtn, 0, 3);
    layout->addWidget(duBtn, 0, 4);

    // create a widget stack on which to put the parts
    stack = new QStackedWidget(this);

    // create the tree part ----------
    tree = new KrFileTreeView(stack);
    tree->setProperty("KrusaderWidgetId", QVariant(Tree));
    stack->addWidget(tree);
    // NOTE: the F2 key press event is caught before it gets to the tree
    tree->setEditTriggers(QAbstractItemView::EditKeyPressed);
    // connecting signal to signal
    connect(tree, &KrFileTreeView::urlActivated, this, &PanelPopup::urlActivated);

    // create the quickview part ------
    imageFilePreview = new KImageFilePreview(stack);
    imageFilePreview->setProperty("KrusaderWidgetId", QVariant(Preview));
    stack->addWidget(imageFilePreview);

    // create the panelview
    fileViewer = new PanelViewer(stack);
    fileViewer->setProperty("KrusaderWidgetId", QVariant(View));
    // kparts demand too much width
    QSizePolicy sizePolicy = fileViewer->sizePolicy();
    sizePolicy.setHorizontalPolicy(QSizePolicy::Ignored);
    fileViewer->setSizePolicy(sizePolicy);
    stack->addWidget(fileViewer);
    connect(fileViewer, &PanelViewer::openUrlRequest, this, &PanelPopup::handleOpenUrlRequest);

    // create the disk usage view
    diskusage = new DiskUsageViewer(stack);
    diskusage->setStatusLabel(dataLine, i18n("Disk Usage:"));
    diskusage->setProperty("KrusaderWidgetId", QVariant(DskUsage));
    stack->addWidget(diskusage);
    connect(diskusage, &DiskUsageViewer::openUrlRequest, this, &PanelPopup::handleOpenUrlRequest);

    // -------- finish the layout (General one)
    layout->addWidget(stack, 1, 0, 1, 5);

    hide(); // for not to open the 3rd hand tool at start (selecting the last used tab)
    setCurrentPage(0);
}

PanelPopup::~PanelPopup() {}

void PanelPopup::saveSettings(KConfigGroup cfg) const
{
    tree->saveSettings(cfg);
}

void PanelPopup::restoreSettings(const KConfigGroup &cfg)
{
    tree->restoreSettings(cfg);
}

void PanelPopup::setCurrentPage(int id)
{
    QAbstractButton * curr = btns->button(id);
    if (curr) {
        curr->click();
    }
}

void PanelPopup::show()
{
    QWidget::show();
    tabSelected(currentPage());
}

void PanelPopup::hide()
{
    QWidget::hide();
    if (currentPage() == View) fileViewer->closeUrl();
    if (currentPage() == DskUsage) diskusage->closeUrl();
}

void PanelPopup::focusInEvent(QFocusEvent*)
{
    switch (currentPage()) {
    case Preview:
        if (!isHidden())
            imageFilePreview->setFocus();
        break;
    case View:
        if (!isHidden() && fileViewer->part() && fileViewer->part()->widget())
            fileViewer->part()->widget()->setFocus();
        break;
    case DskUsage:
        if (!isHidden() && diskusage->getWidget() && diskusage->getWidget()->currentWidget())
            diskusage->getWidget()->currentWidget()->setFocus();
        break;
    case Tree:
        if (!isHidden())
            tree->setFocus();
        break;
    }
}

void PanelPopup::handleOpenUrlRequest(const QUrl &url)
{
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForUrl(url);
    if (mime.isValid() && mime.name() == "inode/directory") ACTIVE_PANEL->func->openUrl(url);
}

void PanelPopup::tabSelected(int id)
{
    QUrl url;
    const FileItem *fileitem = 0;
    if (ACTIVE_PANEL && ACTIVE_PANEL->view)
        fileitem = ACTIVE_PANEL->func->files()->getFileItem(ACTIVE_PANEL->view->getCurrentItem());
    if(fileitem)
        url = fileitem->getUrl();

    // if tab is tree, set something logical in the data line
    switch (id) {
    case Tree:
        stack->setCurrentWidget(tree);
        dataLine->setText(i18n("Tree:"));
        if (!isHidden())
            tree->setFocus();
        if (ACTIVE_PANEL)
            tree->setCurrentUrl(ACTIVE_PANEL->virtualPath());
        break;
    case Preview:
        stack->setCurrentWidget(imageFilePreview);
        dataLine->setText(i18n("Preview:"));
        update(fileitem);
        break;
    case View:
        stack->setCurrentWidget(fileViewer);
        dataLine->setText(i18n("View:"));
        update(fileitem);
        if (!isHidden() && fileViewer->part() && fileViewer->part()->widget())
            fileViewer->part()->widget()->setFocus();
        break;
    case DskUsage:
        stack->setCurrentWidget(diskusage);
        dataLine->setText(i18n("Disk Usage:"));
        update(fileitem);
        if (!isHidden() && diskusage->getWidget() && diskusage->getWidget()->currentWidget())
            diskusage->getWidget()->currentWidget()->setFocus();
        break;
    }
    if (id != View) fileViewer->closeUrl();
}

// decide which part to update, if at all
void PanelPopup::update(const FileItem *fileitem)
{
    if (isHidden())
        return;

    QUrl url;
    if(fileitem)
       url = fileitem->getUrl();

    switch (currentPage()) {
    case Preview:
        imageFilePreview->showPreview(url);
        dataLine->setText(i18n("Preview: %1", url.fileName()));
        break;
    case View:
        if(fileitem && !fileitem->isDir() && fileitem->isReadable())
            fileViewer->openUrl(fileitem->getUrl());
        else
            fileViewer->closeUrl();
        dataLine->setText(i18n("View: %1", url.fileName()));
        break;
    case DskUsage: {
        if(fileitem && !fileitem->isDir())
            url = KIO::upUrl(url);
        dataLine->setText(i18n("Disk Usage: %1", url.fileName()));
        diskusage->openUrl(url);
    }
    break;
    case Tree:  // nothing to do
        break;
    }
}

void PanelPopup::onPanelPathChange(const QUrl &url)
{
    switch (currentPage()) {
    case Tree:
        if (url.isLocalFile()) {
            tree->setCurrentUrl(url); // synchronize panel path with tree path
        }
        break;
    }
}
