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

#include "../krglobal.h"
#include "../kractions.h"
#include "../kicons.h"
#include "../Dialogs/krsqueezedtextlabel.h"
#include "../defaults.h"
#include "../krslots.h"
#include "../KViewer/kimagefilepreview.h"
#include "../KViewer/panelviewer.h"
#include "../KViewer/diskusageviewer.h"
#include "krpanel.h"
#include "listpanel.h"
#include "panelfunc.h"
#include "krview.h"
#include "krviewitem.h"

#include <QtGui/QButtonGroup>
#include <QtGui/QToolButton>
#include <QDropEvent>
#include <QGridLayout>
#include <QFrame>
#include <QMenu>
#include <QtGui/QCursor>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtCore/QDir>
#include <QHeaderView>
#include <QSplitter>

#include <klocale.h>
#include <klineedit.h>
#include <kio/jobclasses.h>
#include <kcolorscheme.h>
#include <kdirlister.h>
#include <kdirmodel.h>
#include <kdirsortfilterproxymodel.h>
#include <kfileitemdelegate.h>
#include <kcombobox.h>

class KrDirModel : public KDirModel
{
public:
    KrDirModel(QWidget *parent, KrFileTreeView *ftv) : KDirModel(parent), fileTreeView(ftv) {}

protected:
    virtual bool dropMimeData(const QMimeData * data, Qt::DropAction /* action */, int /* row */, int /* column */, const QModelIndex & parent) {
        KFileItem item = itemForIndex(parent);
        if (item.isNull())
            return false;

        KUrl::List list = KUrl::List::fromMimeData(data);
        if (list.isEmpty())
            return false;

        fileTreeView->dropMimeData(list, item.url(), parent);
        return true;
    }

    virtual Qt::ItemFlags flags(const QModelIndex & index) const {
        Qt::ItemFlags itflags = KDirModel::flags(index);
        if (index.column() != KDirModel::Name)
            itflags &= ~Qt::ItemIsDropEnabled;
        return itflags;
    }
private:
    KrFileTreeView * fileTreeView;
};

KrFileTreeView::KrFileTreeView(QWidget *parent)
        : QTreeView(parent)
{
    mSourceModel = new KrDirModel(this, this);
    mProxyModel = new KDirSortFilterProxyModel(this);
    mProxyModel->setSourceModel(mSourceModel);

    mSourceModel->setDropsAllowed(KDirModel::DropOnDirectory);

    setModel(mProxyModel);
    setItemDelegate(new KFileItemDelegate(this));

    mSourceModel->dirLister()->openUrl(KUrl(QDir::root().absolutePath()), KDirLister::Keep);

    connect(this, SIGNAL(activated(const QModelIndex&)),
            this, SLOT(slotActivated(const QModelIndex&)));
    connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
            this, SLOT(slotCurrentChanged(const QModelIndex&, const QModelIndex&)));

    connect(mSourceModel, SIGNAL(expand(const QModelIndex&)),
            this, SLOT(slotExpanded(const QModelIndex&)));

    QFontMetrics fontMetrics(viewport()->font());
    header()->resizeSection(KDirModel::Name, fontMetrics.width("WWWWWWWWWWWWWWW"));
}

KUrl KrFileTreeView::urlForProxyIndex(const QModelIndex &index) const
{
    const KFileItem item = mSourceModel->itemForIndex(mProxyModel->mapToSource(index));

    return !item.isNull() ? item.url() : KUrl();
}

void KrFileTreeView::slotActivated(const QModelIndex &index)
{
    const KUrl url = urlForProxyIndex(index);
    if (url.isValid())
        emit activated(url);
}

void KrFileTreeView::dropMimeData(const KUrl::List & lst, const KUrl & url, const QModelIndex & ind)
{
    QModelIndex ndx = mProxyModel->mapFromSource(ind);
    QRect rect = visualRect(ndx);
    QPoint pnt = viewport()->mapToGlobal(QPoint(rect.x(), rect.y() + rect.height() / 2));

    QMenu popup(this);
    QAction * act;

    act = popup.addAction(i18n("Copy Here"));
    act->setData(QVariant(1));
    act = popup.addAction(i18n("Move Here"));
    act->setData(QVariant(2));
    act = popup.addAction(i18n("Link Here"));
    act->setData(QVariant(3));
    act = popup.addAction(i18n("Cancel"));
    act->setData(QVariant(4));

    int result = -1;
    QAction * res = popup.exec(pnt);
    if (res && res->data().canConvert<int> ())
        result = res->data().toInt();

    KIO::CopyJob *job;
    switch (result) {
    case 1 :
        job = KIO::copy(lst, url);
        break;
    case 2 :
        job = KIO::move(lst, url);
        break;
    case 3 :
        job = KIO::link(lst, url);
        break;
    default :         // user pressed outside the menu
        return ;          // or cancel was pressed;
    }
}

void KrFileTreeView::slotCurrentChanged(const QModelIndex &currentIndex, const QModelIndex&)
{
    const KUrl url = urlForProxyIndex(currentIndex);
    if (url.isValid())
        emit currentChanged(url);
}

void KrFileTreeView::slotExpanded(const QModelIndex &baseIndex)
{
    QModelIndex index = mProxyModel->mapFromSource(baseIndex);

    selectionModel()->clearSelection();
    selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
    scrollTo(index);
}


KUrl KrFileTreeView::currentUrl() const
{
    return urlForProxyIndex(currentIndex());
}

KUrl KrFileTreeView::selectedUrl() const
{
    if (!selectionModel()->hasSelection())
        return KUrl();

    const QItemSelection selection = selectionModel()->selection();
    const QModelIndex firstIndex = selection.indexes().first();

    return urlForProxyIndex(firstIndex);
}

KUrl::List KrFileTreeView::selectedUrls() const
{
    KUrl::List urls;

    if (!selectionModel()->hasSelection())
        return urls;

    const QModelIndexList indexes = selectionModel()->selection().indexes();
    foreach(const QModelIndex &index, indexes) {
        const KUrl url = urlForProxyIndex(index);
        if (url.isValid())
            urls.append(url);
    }

    return urls;
}

KUrl KrFileTreeView::rootUrl() const
{
    return mSourceModel->dirLister()->url();
}

void KrFileTreeView::setDirOnlyMode(bool enabled)
{
    mSourceModel->dirLister()->setDirOnlyMode(enabled);
    mSourceModel->dirLister()->openUrl(mSourceModel->dirLister()->url());
}

void KrFileTreeView::setShowHiddenFiles(bool enabled)
{
    mSourceModel->dirLister()->setShowingDotFiles(enabled);
    mSourceModel->dirLister()->openUrl(mSourceModel->dirLister()->url());
}

void KrFileTreeView::setCurrentUrl(const KUrl &url)
{
    QModelIndex baseIndex = mSourceModel->indexForUrl(url);

    if (!baseIndex.isValid()) {
        mSourceModel->expandToUrl(url);
        return;
    }

    QModelIndex proxyIndex = mProxyModel->mapFromSource(baseIndex);
    selectionModel()->clearSelection();
    selectionModel()->setCurrentIndex(proxyIndex, QItemSelectionModel::SelectCurrent);
    scrollTo(proxyIndex);
}

void KrFileTreeView::setRootUrl(const KUrl &url)
{
    mSourceModel->dirLister()->openUrl(url);
}

PanelPopup::PanelPopup(QSplitter *parent, bool left) : QWidget(parent),
        _left(left), _hidden(true), stack(0), viewer(0), pjob(0), splitterSizes()
{
    splitter = parent;
    QGridLayout * layout = new QGridLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // loading the splitter sizes
    KConfigGroup pg(krConfig, "Private");
    if (left)
        splitterSizes = pg.readEntry("Left PanelPopup Splitter Sizes", QList<int>());
    else
        splitterSizes = pg.readEntry("Right PanelPopup Splitter Sizes", QList<int>());

    if (splitterSizes.count() == 2 && splitterSizes[ 0 ] == 0 && splitterSizes[ 1 ] == 0)
        splitterSizes.clear();

    if (splitterSizes.count() < 2) {
        splitterSizes.clear();
        splitterSizes.push_back(100);
        splitterSizes.push_back(100);
    }

    // create the label+buttons setup
    dataLine = new KrSqueezedTextLabel(this);
    dataLine->setText("blah blah");
    connect(dataLine, SIGNAL(clicked()), this, SLOT(setFocus()));
    KConfigGroup lg(krConfig, "Look&Feel");
    dataLine->setFont(lg.readEntry("Filelist Font", *_FilelistFont));
    // --- hack: setup colors to be the same as an inactive panel
    dataLine->setBackgroundRole(QPalette::Window);
    QPalette q(dataLine->palette());
    q.setColor(QPalette::WindowText, KColorScheme(QPalette::Active, KColorScheme::View).foreground().color());
    q.setColor(QPalette::Window, KColorScheme(QPalette::Active, KColorScheme::View).background().color());
    dataLine->setPalette(q);
    dataLine->setFrameStyle(QFrame::Box | QFrame::Raised);
    dataLine->setLineWidth(1);    // a nice 3D touch :-)
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
    previewBtn->setIcon(krLoader->loadIcon("thumbnail-show", KIconLoader::Toolbar, 16));
    previewBtn->setFixedSize(20, 20);
    previewBtn->setCheckable(true);
    btns->addButton(previewBtn, Preview);

    quickBtn = new QToolButton(this);
    quickBtn->setToolTip(i18n("Quick Panel: quick way to perform actions"));
    quickBtn->setIcon(krLoader->loadIcon("fork", KIconLoader::Toolbar, 16));
    quickBtn->setFixedSize(20, 20);
    quickBtn->setCheckable(true);
    btns->addButton(quickBtn, QuickPanel);

    viewerBtn = new QToolButton(this);
    viewerBtn->setToolTip(i18n("View Panel: view the current file"));
    viewerBtn->setIcon(krLoader->loadIcon("zoom-original", KIconLoader::Toolbar, 16));
    viewerBtn->setFixedSize(20, 20);
    viewerBtn->setCheckable(true);
    btns->addButton(viewerBtn, View);

    duBtn = new QToolButton(this);
    duBtn->setToolTip(i18n("Disk Usage Panel: view the usage of a directory"));
    duBtn->setIcon(krLoader->loadIcon("kr_diskusage", KIconLoader::Toolbar, 16));
    duBtn->setFixedSize(20, 20);
    duBtn->setCheckable(true);
    btns->addButton(duBtn, DskUsage);

    layout->addWidget(dataLine, 0, 0);
    layout->addWidget(treeBtn, 0, 1);
    layout->addWidget(previewBtn, 0, 2);
    layout->addWidget(quickBtn, 0, 3);
    layout->addWidget(viewerBtn, 0, 4);
    layout->addWidget(duBtn, 0, 5);

    // create a widget stack on which to put the parts
    stack = new QStackedWidget(this);

    // create the tree part ----------
    tree = new KrFileTreeView(stack);
    tree->setAcceptDrops(true);
    tree->setDragDropMode(QTreeView::DropOnly);
    tree->setDropIndicatorShown(true);

    tree->setProperty("KrusaderWidgetId", QVariant(Tree));
    stack->addWidget(tree);
    tree->setDirOnlyMode(true);
    connect(tree, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(treeSelection()));
    connect(tree, SIGNAL(activated(const KUrl &)), this, SLOT(treeSelection()));

    // create the quickview part ------
    viewer = new KrusaderImageFilePreview(stack);
    viewer->setProperty("KrusaderWidgetId", QVariant(Preview));
    stack->addWidget(viewer);

    // create the panelview

    panelviewer = new PanelViewer(stack);
    panelviewer->setProperty("KrusaderWidgetId", QVariant(View));
    stack->addWidget(panelviewer);
    connect(panelviewer, SIGNAL(openUrlRequest(const KUrl &)), this, SLOT(handleOpenUrlRequest(const KUrl &)));

    // create the disk usage view

    diskusage = new DiskUsageViewer(stack);
    diskusage->setStatusLabel(dataLine, i18n("Disk Usage: "));
    diskusage->setProperty("KrusaderWidgetId", QVariant(DskUsage));
    stack->addWidget(diskusage);
    connect(diskusage, SIGNAL(openUrlRequest(const KUrl &)), this, SLOT(handleOpenUrlRequest(const KUrl &)));

    // create the quick-panel part ----

    quickPanel = new QWidget(stack);
    QGridLayout *qlayout = new QGridLayout(quickPanel);
    // --- quick select
    QLabel *selectLabel = new QLabel(i18n("Quick Select"), quickPanel);
    quickSelectCombo = new KComboBox(quickPanel);
    quickSelectCombo->setEditable(true);
    QStringList lst = pg.readEntry("Predefined Selections", QStringList());
    if (lst.count() > 0)
        quickSelectCombo->addItems(lst);
    quickSelectCombo->lineEdit()->setText("*");
    quickSelectCombo->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));

    connect(quickSelectCombo->lineEdit(), SIGNAL(returnPressed()),
            this, SLOT(quickSelect()));

    QToolButton *qselectBtn = new QToolButton(quickPanel);
    qselectBtn->setIcon(krLoader->loadIcon("kr_selectall", KIconLoader::Toolbar, 16));
    qselectBtn->setFixedSize(20, 20);
    qselectBtn->setToolTip(i18n("Apply the selection"));
    connect(qselectBtn, SIGNAL(clicked()), this, SLOT(quickSelect()));

    QToolButton *qstoreBtn = new QToolButton(quickPanel);
    qstoreBtn->setIcon(krLoader->loadIcon("document-save", KIconLoader::Toolbar, 16));
    qstoreBtn->setFixedSize(20, 20);
    qstoreBtn->setToolTip(i18n("Store the current selection"));
    connect(qstoreBtn, SIGNAL(clicked()), this, SLOT(quickSelectStore()));

    QToolButton *qsettingsBtn = new QToolButton(quickPanel);
    qsettingsBtn->setIcon(krLoader->loadIcon("configure", KIconLoader::Toolbar, 16));
    qsettingsBtn->setFixedSize(20, 20);
    qsettingsBtn->setToolTip(i18n("Select group dialog"));
    connect(qsettingsBtn, SIGNAL(clicked()), krSelect, SLOT(trigger()));

    qlayout->addWidget(selectLabel, 0, 0);
    qlayout->addWidget(quickSelectCombo, 0, 1);
    qlayout->addWidget(qselectBtn, 0, 2);
    qlayout->addWidget(qstoreBtn, 0, 3);
    qlayout->addWidget(qsettingsBtn, 0, 4);

    quickPanel->setProperty("KrusaderWidgetId", QVariant(QuickPanel));
    stack->addWidget(quickPanel);

    // -------- finish the layout (General one)
    layout->addWidget(stack, 1, 0, 1, 6);

    // set the wanted widget
    // ugly: are we left or right?
    int id;
    KConfigGroup sg(krConfig, "Startup");
    if (left) {
        id = sg.readEntry("Left Panel Popup", _LeftPanelPopup);
    } else {
        id = sg.readEntry("Right Panel Popup", _RightPanelPopup);
    }
    QAbstractButton * curr = btns->button(id);
    if (curr)
        curr->click();

    hide(); // for not to open the 3rd hand tool at start (selecting the last used tab)
    tabSelected(id);
}

PanelPopup::~PanelPopup() {}

void PanelPopup::show()
{
    QWidget::show();
    if (_hidden)
        splitter->setSizes(splitterSizes);
    _hidden = false;
    tabSelected(currentPage());
}


void PanelPopup::hide()
{
    if (!_hidden)
        splitterSizes = splitter->sizes();
    QWidget::hide();
    _hidden = true;
    if (currentPage() == View) panelviewer->closeUrl();
    if (currentPage() == DskUsage) diskusage->closeUrl();
}

void PanelPopup::focusInEvent(QFocusEvent*)
{
    switch (currentPage()) {
    case Preview:
        if (!isHidden())
            viewer->setFocus();
        break;
    case View:
        if (!isHidden() && panelviewer->part() && panelviewer->part()->widget())
            panelviewer->part()->widget()->setFocus();
        break;
    case DskUsage:
        if (!isHidden() && diskusage->getWidget() && diskusage->getWidget()->currentWidget())
            diskusage->getWidget()->currentWidget()->setFocus();
        break;
    case Tree:
        if (!isHidden())
            tree->setFocus();
        break;
    case QuickPanel:
        if (!isHidden())
            quickSelectCombo->setFocus();
        break;
    }
}

void PanelPopup::saveSizes()
{
    KConfigGroup group(krConfig, "Private");

    if (!isHidden())
        splitterSizes = splitter->sizes();

    if (_left)
        group.writeEntry("Left PanelPopup Splitter Sizes", splitterSizes);
    else
        group.writeEntry("Right PanelPopup Splitter Sizes", splitterSizes);
}

void PanelPopup::handleOpenUrlRequest(const KUrl &url)
{
    KMimeType::Ptr mime = KMimeType::findByUrl(url.url());
    if (mime && mime->name() == "inode/directory") ACTIVE_PANEL->func->openUrl(url);
}


void PanelPopup::tabSelected(int id)
{
    KUrl url;
    const vfile *vf = 0;
    if (ACTIVE_PANEL && ACTIVE_PANEL->func->files() && ACTIVE_PANEL->view)
        vf = ACTIVE_PANEL->func->files()->vfs_search(ACTIVE_PANEL->view->getCurrentItem());
    if(vf)
        url = vf->vfile_getUrl();

    // if tab is tree, set something logical in the data line
    switch (id) {
    case Tree:
        stack->setCurrentWidget(tree);
        dataLine->setText(i18n("Tree:"));
        if (!isHidden())
            tree->setFocus();
        if (ACTIVE_PANEL)
            tree->setCurrentUrl(ACTIVE_PANEL->func->files()->vfs_getOrigin());
        break;
    case Preview:
        stack->setCurrentWidget(viewer);
        dataLine->setText(i18n("Preview:"));
        update(vf);
        break;
    case QuickPanel:
        stack->setCurrentWidget(quickPanel);
        dataLine->setText(i18n("Quick Select:"));
        if (!isHidden())
            quickSelectCombo->setFocus();
        break;
    case View:
        stack->setCurrentWidget(panelviewer);
        dataLine->setText(i18n("View:"));
        update(vf);
        if (!isHidden() && panelviewer->part() && panelviewer->part()->widget())
            panelviewer->part()->widget()->setFocus();
        break;
    case DskUsage:
        stack->setCurrentWidget(diskusage);
        dataLine->setText(i18n("Disk Usage:"));
        update(vf);
        if (!isHidden() && diskusage->getWidget() && diskusage->getWidget()->currentWidget())
            diskusage->getWidget()->currentWidget()->setFocus();
        break;
    }
    if (id != View) panelviewer->closeUrl();
}

// decide which part to update, if at all
void PanelPopup::update(const vfile *vf)
{
    if (isHidden())
        return;

    KUrl url;
    if(vf)
       url = vf->vfile_getUrl();

    switch (currentPage()) {
    case Preview:
        viewer->showPreview(url);
        dataLine->setText(i18n("Preview: %1", url.fileName()));
        break;
    case View:
        if(vf && !vf->vfile_isDir() && vf->vfile_isReadable())
            panelviewer->openUrl(vf->vfile_getUrl());
        else
            panelviewer->closeUrl();
        dataLine->setText(i18n("View: %1", url.fileName()));
        break;
    case DskUsage: {
        if(vf && !vf->vfile_isDir())
            url = url.upUrl();
        dataLine->setText(i18n("Disk Usage: %1", url.fileName()));
        diskusage->openUrl(url);
    }
    break;
    case Tree:  // nothing to do
        break;
    }
}

// ------------------- tree

void PanelPopup::treeSelection()
{
    emit selection(tree->currentUrl());
    //emit hideMe();
}

// ------------------- quick panel

void PanelPopup::quickSelect()
{
    quickSelect(quickSelectCombo->currentText());
}

void PanelPopup::quickSelect(const QString &mask)
{
    ACTIVE_PANEL->gui->select(KRQuery(mask), true);
}

void PanelPopup::quickSelectStore()
{
    KConfigGroup group(krConfig, "Private");
    QStringList lst = group.readEntry("Predefined Selections", QStringList());
    if (lst.indexOf(quickSelectCombo->currentText()) == -1)
        lst.append(quickSelectCombo->currentText());
    group.writeEntry("Predefined Selections", lst);
}

#include "panelpopup.moc"
