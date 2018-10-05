/*****************************************************************************
 * Copyright (C) 2001 Shie Erlich <krusader@users.sourceforge.net>           *
 * Copyright (C) 2001 Rafi Yanai <krusader@users.sourceforge.net>            *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#include "krsearchdialog.h"

// QtCore
#include <QRegExp>
#include <QMimeData>
// QtGui
#include <QKeyEvent>
#include <QResizeEvent>
#include <QCursor>
#include <QClipboard>
#include <QDrag>
#include <QCloseEvent>
// QtWidgets
#include <QApplication>
#include <QGridLayout>
#include <QInputDialog>
#include <QHBoxLayout>
#include <QMenu>

#include <KConfigCore/KConfig>
#include <KI18n/KLocalizedString>
#include <KWidgetsAddons/KMessageBox>

#include "krsearchmod.h"
#include "../Dialogs/krdialogs.h"
#include "../Dialogs/krspecialwidgets.h"
#include "../Dialogs/krsqueezedtextlabel.h"
#include "../FileSystem/fileitem.h"
#include "../FileSystem/krquery.h"
#include "../FileSystem/virtualfilesystem.h"
#include "../Filter/filtertabs.h"
#include "../Filter/generalfilter.h"
#include "../KViewer/krviewer.h"
#include "../Panel/PanelView/krview.h"
#include "../Panel/PanelView/krviewfactory.h"
#include "../Panel/PanelView/krviewitem.h"
#include "../Panel/krpanel.h"
#include "../Panel/krsearchbar.h"
#include "../Panel/panelfunc.h"
#include "../defaults.h"
#include "../kractions.h"
#include "../krglobal.h"
#include "../filelisticon.h"
#include "../krservices.h"
#include "../krslots.h"
#include "../krusaderview.h"
#include "../panelmanager.h"

#define RESULTVIEW_TYPE 0

class SearchResultContainer : public DirListerInterface
{
public:
    explicit SearchResultContainer(QObject *parent) : DirListerInterface(parent) {}
    virtual ~SearchResultContainer() {
        clear();
    }

    virtual QList<FileItem *> fileItems() const Q_DECL_OVERRIDE {
        return _fileItems;
    }
    virtual unsigned long numFileItems() const Q_DECL_OVERRIDE {
        return _fileItems.count();
    }
    virtual bool isRoot() const Q_DECL_OVERRIDE {
        return true;
    }

    void clear() {
        emit cleared();
        foreach(FileItem *fileitem, _fileItems)
            delete fileitem;
        _fileItems.clear();
        _foundText.clear();
    }

    void addItem(const FileItem &file, const QString &foundText)
    {
        const QString path = file.getUrl().toDisplayString(QUrl::PreferLocalFile);
        FileItem *fileitem = FileItem::createCopy(file, path);
        _fileItems << fileitem;
        if(!foundText.isEmpty())
            _foundText[fileitem] = foundText;
        emit addedFileItem(fileitem);
    }

    QString foundText(const FileItem *fileitem) {
        return _foundText[fileitem];
    }

private:
    QList<FileItem *> _fileItems;
    QHash<const FileItem *, QString> _foundText;
};


KrSearchDialog *KrSearchDialog::SearchDialog = nullptr;

QString KrSearchDialog::lastSearchText = QString('*');
int KrSearchDialog::lastSearchType = 0;
bool KrSearchDialog::lastSearchForCase = false;
bool KrSearchDialog::lastContainsWholeWord = false;
bool KrSearchDialog::lastContainsWithCase = false;
bool KrSearchDialog::lastSearchInSubDirs = true;
bool KrSearchDialog::lastSearchInArchives = false;
bool KrSearchDialog::lastFollowSymLinks = false;
bool KrSearchDialog::lastContainsRegExp = false;


// class starts here /////////////////////////////////////////
KrSearchDialog::KrSearchDialog(QString profile, QWidget* parent)
        : QDialog(parent), query(nullptr), searcher(nullptr), isBusy(false), closed(false)
{
    KConfigGroup group(krConfig, "Search");

    setWindowTitle(i18n("Krusader::Search"));
    setWindowIcon(Icon("system-search"));

    auto* searchBaseLayout = new QGridLayout(this);
    searchBaseLayout->setSpacing(6);
    searchBaseLayout->setContentsMargins(11, 11, 11, 11);

    // creating the dialog buttons ( Search, Stop, Close )

    auto* buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(6);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);

    profileManager = new ProfileManager("SearcherProfile", this);
    buttonsLayout->addWidget(profileManager);

    searchTextToClipboard = new QCheckBox(this);
    searchTextToClipboard->setText(i18n("Query to clipboard"));
    searchTextToClipboard->setToolTip(i18n("Place search text to clipboard when a found file is opened."));
    searchTextToClipboard->setCheckState(static_cast<Qt::CheckState>(group.readEntry("QueryToClipboard", 0)));
    connect(searchTextToClipboard, &QCheckBox::stateChanged, this, [=](int state) {
        KConfigGroup group(krConfig, "Search");
        group.writeEntry("QueryToClipboard", state);
    });
    buttonsLayout->addWidget(searchTextToClipboard);

    auto* spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttonsLayout->addItem(spacer);

    mainSearchBtn = new QPushButton(this);
    mainSearchBtn->setText(i18n("Search"));
    mainSearchBtn->setIcon(Icon("edit-find"));
    mainSearchBtn->setDefault(true);
    buttonsLayout->addWidget(mainSearchBtn);

    mainStopBtn = new QPushButton(this);
    mainStopBtn->setEnabled(false);
    mainStopBtn->setText(i18n("Stop"));
    mainStopBtn->setIcon(Icon("process-stop"));
    buttonsLayout->addWidget(mainStopBtn);

    mainCloseBtn = new QPushButton(this);
    mainCloseBtn->setText(i18n("Close"));
    mainCloseBtn->setIcon(Icon("dialog-close"));
    buttonsLayout->addWidget(mainCloseBtn);

    searchBaseLayout->addLayout(buttonsLayout, 1, 0);

    // creating the searcher tabs

    searcherTabs = new QTabWidget(this);

    filterTabs = FilterTabs::addTo(searcherTabs, FilterTabs::Default);
    generalFilter = (GeneralFilter *)filterTabs->get("GeneralFilter");

    QWidget* resultTab = new QWidget(searcherTabs);
    auto* resultLayout = new QGridLayout(resultTab);
    resultLayout->setSpacing(6);
    resultLayout->setContentsMargins(6, 6, 6, 6);

    // creating the result tab

    // creating the result list view
    result = new SearchResultContainer(this);
    // the view
    resultView = KrViewFactory::createView(RESULTVIEW_TYPE, resultTab, krConfig);
    resultView->init(false);
    resultView->restoreSettings(KConfigGroup(&group, "ResultView"));
    resultView->setMainWindow(this);
    resultView->prepareForActive();
    resultView->refreshColors();
    resultView->setFiles(result);
    resultView->refresh();
    resultLayout->addWidget(resultView->widget(), 0, 0);
    // search bar - hidden by default
    searchBar = new KrSearchBar(resultView, this);
    searchBar->hide();
    resultLayout->addWidget(searchBar, 1, 0);

    // text found row
    foundTextFrame = new QFrame(resultTab);
    foundTextFrame->setFrameShape(QLabel::StyledPanel);
    foundTextFrame->setFrameShadow(QLabel::Sunken);

    auto* foundTextLayout = new QHBoxLayout();
    foundTextLayout->setSpacing(6);

    QLabel *textFoundLabel = new QLabel(i18n("Text found:"), resultTab);
    foundTextLayout->addWidget(textFoundLabel);
    foundTextLabel = new KrSqueezedTextLabel(resultTab);
    foundTextLayout->addWidget(foundTextLabel);

    foundTextFrame->setLayout(foundTextLayout);
    foundTextFrame->hide();

    resultLayout->addWidget(foundTextFrame, 2, 0);

    // result info row
    auto* resultLabelLayout = new QHBoxLayout();
    resultLabelLayout->setSpacing(6);
    resultLabelLayout->setContentsMargins(0, 0, 0, 0);

    searchingLabel = new KSqueezedTextLabel(i18n("Idle"), resultTab);
    resultLabelLayout->addWidget(searchingLabel);

    foundLabel = new QLabel("", resultTab);
    resultLabelLayout->addWidget(foundLabel);

    mainFeedToListBoxBtn = new QPushButton(QIcon::fromTheme("list-add"), i18n("Feed to listbox"), this);
    mainFeedToListBoxBtn->setEnabled(false);
    resultLabelLayout->addWidget(mainFeedToListBoxBtn);

    resultLayout->addLayout(resultLabelLayout, 3, 0);

    searcherTabs->addTab(resultTab, i18n("&Results"));

    searchBaseLayout->addWidget(searcherTabs, 0, 0);

    // signals and slots connections

    connect(mainSearchBtn, &QPushButton::clicked, this, &KrSearchDialog::startSearch);
    connect(mainStopBtn, &QPushButton::clicked, this, &KrSearchDialog::stopSearch);
    connect(mainCloseBtn, &QPushButton::clicked, this, &KrSearchDialog::closeDialog);
    connect(mainFeedToListBoxBtn, &QPushButton::clicked, this, &KrSearchDialog::feedToListBox);

    connect(profileManager, &ProfileManager::loadFromProfile, filterTabs, &FilterTabs::loadFromProfile);
    connect(profileManager, &ProfileManager::saveToProfile, filterTabs, &FilterTabs::saveToProfile);

    connect(resultView->op(), &KrViewOperator::currentChanged, this, &KrSearchDialog::currentChanged);
    connect(resultView->op(), &KrViewOperator::executed, this, &KrSearchDialog::executed);
    connect(resultView->op(), &KrViewOperator::contextMenu, this, &KrSearchDialog::contextMenu);

    // tab order

    setTabOrder(mainSearchBtn, mainCloseBtn);
    setTabOrder(mainCloseBtn, mainStopBtn);
    setTabOrder(mainStopBtn, searcherTabs);
    setTabOrder(searcherTabs, resultView->widget());

    sizeX = group.readEntry("Window Width",  -1);
    sizeY = group.readEntry("Window Height",  -1);

    if (sizeX != -1 && sizeY != -1)
        resize(sizeX, sizeY);

    if (group.readEntry("Window Maximized",  false))
        showMaximized();
    else
        show();

    generalFilter->searchFor->setFocus();

    // finally, load a profile of apply defaults:

    if (profile.isEmpty()) {
        // load the last used values
        generalFilter->searchFor->setEditText(lastSearchText);
        generalFilter->searchFor->lineEdit()->selectAll();
        generalFilter->ofType->setCurrentIndex(lastSearchType);
        generalFilter->searchForCase->setChecked(lastSearchForCase);
        generalFilter->containsWholeWord->setChecked(lastContainsWholeWord);
        generalFilter->containsTextCase->setChecked(lastContainsWithCase);
        generalFilter->containsRegExp->setChecked(lastContainsRegExp);
        generalFilter->searchInDirs->setChecked(lastSearchInSubDirs);
        generalFilter->searchInArchives->setChecked(lastSearchInArchives);
        generalFilter->followLinks->setChecked(lastFollowSymLinks);
        // the path in the active panel should be the default search location
        generalFilter->searchIn->lineEdit()->setText(ACTIVE_PANEL->virtualPath().toDisplayString(QUrl::PreferLocalFile));
    } else
        profileManager->loadProfile(profile);   // important: call this _after_ you've connected profileManager ot the loadFromProfile!!
}

KrSearchDialog::~KrSearchDialog()
{
    delete query;
    query = nullptr;
    delete resultView;
    resultView = nullptr;
}

void KrSearchDialog::closeDialog(bool isAccept)
{
    if(isBusy) {
        closed = true;
        return;
    }

    // stop the search if it's on-going
    if (searcher != nullptr) {
        delete searcher;
        searcher = nullptr;
    }

    // saving the searcher state

    KConfigGroup group(krConfig, "Search");

    group.writeEntry("Window Width", sizeX);
    group.writeEntry("Window Height", sizeY);
    group.writeEntry("Window Maximized", isMaximized());

    resultView->saveSettings(KConfigGroup(&group, "ResultView"));

    lastSearchText = generalFilter->searchFor->currentText();
    lastSearchType = generalFilter->ofType->currentIndex();
    lastSearchForCase = generalFilter->searchForCase->isChecked();
    lastContainsWholeWord = generalFilter->containsWholeWord->isChecked();
    lastContainsWithCase = generalFilter->containsTextCase->isChecked();
    lastContainsRegExp = generalFilter->containsRegExp->isChecked();
    lastSearchInSubDirs = generalFilter->searchInDirs->isChecked();
    lastSearchInArchives = generalFilter->searchInArchives->isChecked();
    lastFollowSymLinks = generalFilter->followLinks->isChecked();
    hide();

    SearchDialog = nullptr;
    if (isAccept)
        QDialog::accept();
    else
        QDialog::reject();

    this->deleteLater();
}

void KrSearchDialog::reject()
{
    closeDialog(false);
}

void KrSearchDialog::resizeEvent(QResizeEvent *e)
{
    if (!isMaximized()) {
        sizeX = e->size().width();
        sizeY = e->size().height();
    }
}

void KrSearchDialog::slotFound(const FileItem &file, const QString &foundText)
{
    result->addItem(file, foundText);
    foundLabel->setText(i18np("Found %1 match.", "Found %1 matches.", result->numFileItems()));
}

bool KrSearchDialog::gui2query()
{
    // prepare the query ...
    /////////////////// names, locations and greps
    if (query != nullptr) {
        delete query; query = nullptr;
    }
    query = new KRQuery();

    return filterTabs->fillQuery(query);
}

void KrSearchDialog::startSearch()
{
    if(isBusy)
        return;

    // prepare the query /////////////////////////////////////////////
    if (!gui2query()) return;

    // first, informative messages
    if (query->searchInArchives()) {
        KMessageBox::information(this, i18n("Since you chose to also search in archives, "
                                            "note the following limitations:\n"
                                            "You cannot search for text (grep) while doing"
                                            " a search that includes archives."), nullptr, "searchInArchives");
    }

    // prepare the gui ///////////////////////////////////////////////
    result->clear();
    resultView->setSortMode(KrViewProperties::NoColumn, 0);

    foundTextFrame->setVisible(!query->content().isEmpty());
    foundTextLabel->setText("");

    searchingLabel->setText("");
    foundLabel->setText(i18n("Found 0 matches."));
    mainFeedToListBoxBtn->setEnabled(false);

    mainSearchBtn->setEnabled(false);
    mainStopBtn->setEnabled(true);
    mainCloseBtn->setEnabled(false);

    searcherTabs->setCurrentIndex(2); // show the results page

    isBusy = true;

    qApp->processEvents();

    // start the search.
    if (searcher != nullptr)
        abort();
    searcher  = new KRSearchMod(query);
    connect(searcher, &KRSearchMod::searching, searchingLabel, &KSqueezedTextLabel::setText);
    connect(searcher, &KRSearchMod::found, this, &KrSearchDialog::slotFound);
    connect(searcher, &KRSearchMod::finished, this, &KrSearchDialog::stopSearch);

    searcher->start();

    isBusy = false;

    delete searcher;
    searcher = nullptr;

    // gui stuff
    mainSearchBtn->setEnabled(true);
    mainCloseBtn->setEnabled(true);
    mainStopBtn->setEnabled(false);
    if (result->numFileItems())
        mainFeedToListBoxBtn->setEnabled(true);
    searchingLabel->setText(i18n("Finished searching."));

    if (closed)
        closeDialog();
}

void KrSearchDialog::stopSearch()
{
    if (searcher != nullptr) {
        searcher->stop();
        disconnect(searcher, nullptr, nullptr, nullptr);
    }
}

void KrSearchDialog::executed(const QString &name)
{
    // 'name' is (local) file path or complete URL
    QString path = name;
    QString fileName;
    if(!name.endsWith('/')) {
        // not a directory, split filename and path
        int idx = name.lastIndexOf("/");
        fileName = name.mid(idx+1);
        path = name.left(idx);
    }
    QUrl url(path);
    if (url.scheme().isEmpty()) {
        url = QUrl::fromLocalFile(path);
    }
    ACTIVE_FUNC->openUrl(url, fileName);
    showMinimized();
}

void KrSearchDialog::currentChanged(KrViewItem *item)
{
    if(!item)
        return;
    QString text = result->foundText(item->getFileItem());
    if(!text.isEmpty()) {
        // ugly hack: find the <b> and </b> in the text, then
        // use it to set the are which we don't want squeezed
        int idx = text.indexOf("<b>") - 4; // take "<qt>" into account
        int length = text.indexOf("</b>") - idx + 4;
        foundTextLabel->setText(text, idx, length);
    }
}

void KrSearchDialog::closeEvent(QCloseEvent *e)
{                     /* if searching is in progress we must not close the window */
    if (isBusy)    /* because qApp->processEvents() is called by the searcher and */
    {                   /* at window destruction, the searcher object will be deleted */
        stopSearch();         /* instead we stop searching */
        closed = true;        /* and after stopping: startSearch can close the window */
        e->ignore();          /* ignoring the close event */
    } else
        QDialog::closeEvent(e);     /* if no searching, let QDialog handle the event */
}

void KrSearchDialog::keyPressEvent(QKeyEvent *e)
{
    // TODO: don't use hardcoded shortcuts

    if (isBusy && e->key() == Qt::Key_Escape) { /* at searching we must not close the window */
        stopSearch();         /* so we simply stop searching */
        return;
    }
    if (resultView->widget()->hasFocus()) {
        if ((e->key() | e->modifiers()) == (Qt::CTRL | Qt::Key_I)) {
            searchBar->showBar(KrSearchBar::MODE_FILTER);
        } else if (e->key() == Qt::Key_F4) {
            tryPlaceSearchQueryToClipboard();
            editCurrent();
            return;
        } else if (e->key() == Qt::Key_F3) {
            tryPlaceSearchQueryToClipboard();
            viewCurrent();
            return;
        } else if (e->key() == Qt::Key_F10) {
            compareByContent();
            return;
        } else if (KrGlobal::copyShortcut == QKeySequence(e->key() | e->modifiers())) {
            copyToClipBoard();
            return;
        }
    }
    QDialog::keyPressEvent(e);
}

void KrSearchDialog::editCurrent()
{
    KrViewItem *current = resultView->getCurrentKrViewItem();
    if (current)
        KrViewer::edit(current->getFileItem()->getUrl(), this);
}

void KrSearchDialog::viewCurrent()
{
    KrViewItem *current = resultView->getCurrentKrViewItem();
    if (current)
        KrViewer::view(current->getFileItem()->getUrl(), this);
}

void KrSearchDialog::compareByContent()
{
    KrViewItemList list;
    resultView->getSelectedKrViewItems(&list);
    if (list.count() != 2)
        return;

    SLOTS->compareContent(list[0]->getFileItem()->getUrl(),list[1]->getFileItem()->getUrl());
}

void KrSearchDialog::contextMenu(const QPoint &pos)
{
    // create the menu
    QMenu popup;
    popup.setTitle(i18n("Krusader Search"));

    QAction *actView = popup.addAction(i18n("View File (F3)"));
    QAction *actEdit = popup.addAction(i18n("Edit File (F4)"));
    QAction *actComp = popup.addAction(i18n("Compare by content (F10)"));
    if(resultView->numSelected() != 2)
        actComp->setEnabled(false);
    QAction *actClip = popup.addAction(i18n("Copy selected to clipboard"));

    QAction *result = popup.exec(pos);

    // check out the user's option
    if (result == actView)
        viewCurrent();
    else if (result == actEdit)
        editCurrent();
    else if (result == actClip)
        copyToClipBoard();
    else if (result == actComp)
        compareByContent();
}

void KrSearchDialog::feedToListBox()
{
    VirtualFileSystem virtFilesystem;
    virtFilesystem.scanDir(QUrl::fromLocalFile("/"));

    KConfigGroup group(krConfig, "Search");
    int listBoxNum = group.readEntry("Feed To Listbox Counter", 1);
    QString queryName;
    if(query) {
        const QString where = KrServices::toStringList(query->searchInDirs()).join(", ");
        queryName = query->content().isEmpty() ?
                        i18n("Search results for \"%1\" in %2", query->nameFilter(), where) :
                        i18n("Search results for \"%1\" containing \"%2\" in %3", query->nameFilter(), query->content(), where);
    }
    QString fileSystemName;
    do {
        fileSystemName = i18n("Search results") + QString(" %1").arg(listBoxNum++);
    } while (virtFilesystem.getFileItem(fileSystemName) != nullptr);
    group.writeEntry("Feed To Listbox Counter", listBoxNum);

    KConfigGroup ga(krConfig, "Advanced");
    if (ga.readEntry("Confirm Feed to Listbox",  _ConfirmFeedToListbox)) {
        bool ok;
        fileSystemName = QInputDialog::getText(this, i18n("Query name"), i18n("Here you can name the file collection"),
                                        QLineEdit::Normal, fileSystemName, &ok);
        if (! ok)
            return;
    }

    QList<QUrl> urlList;
    foreach(FileItem *fileitem, result->fileItems())
        urlList.push_back(fileitem->getUrl());

    mainSearchBtn->setEnabled(false);
    mainCloseBtn->setEnabled(false);
    mainFeedToListBoxBtn->setEnabled(false);

    isBusy = true;

    const QUrl url = QUrl(QString("virt:/") + fileSystemName);
    virtFilesystem.scanDir(url);
    virtFilesystem.addFiles(urlList);
    virtFilesystem.setMetaInformation(queryName);
    //ACTIVE_FUNC->openUrl(url);
    ACTIVE_MNG->slotNewTab(url);

    isBusy = false;

    closeDialog();
}

void KrSearchDialog::copyToClipBoard()
{
    QList<QUrl> urls;
    foreach(FileItem *fileitem, result->fileItems())
        urls.push_back(fileitem->getUrl());

    if (urls.count() == 0)
        return;

    auto *mimeData = new QMimeData;
    mimeData->setImageData(FileListIcon("file").pixmap());
    mimeData->setUrls(urls);

    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

void KrSearchDialog::tryPlaceSearchQueryToClipboard()
{
    if (searchTextToClipboard->isChecked()
            && !generalFilter->containsText->currentText().isEmpty()
            && QApplication::clipboard()->text() != generalFilter->containsText->currentText()) {
        QApplication::clipboard()->setText(generalFilter->containsText->currentText());
    }
}
