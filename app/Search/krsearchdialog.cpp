/*
    SPDX-FileCopyrightText: 2001 Shie Erlich <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2001 Rafi Yanai <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krsearchdialog.h"

// QtCore
#include <QMimeData>
#include <QRegExp>
// QtGui
#include <QClipboard>
#include <QCloseEvent>
#include <QCursor>
#include <QDrag>
#include <QKeyEvent>
#include <QResizeEvent>
// QtWidgets
#include <QApplication>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMenu>

#include <KConfig>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardAction>

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
#include "../filelisticon.h"
#include "../kractions.h"
#include "../krglobal.h"
#include "../krservices.h"
#include "../krslots.h"
#include "../krusaderview.h"
#include "../panelmanager.h"
#include "krsearchmod.h"

#define RESULTVIEW_TYPE 0

class SearchResultContainer : public DirListerInterface
{
public:
    explicit SearchResultContainer(QObject *parent)
        : DirListerInterface(parent)
    {
    }
    ~SearchResultContainer() override
    {
        clear();
    }

    QList<FileItem *> fileItems() const override
    {
        return _fileItems;
    }
    unsigned long numFileItems() const override
    {
        return _fileItems.count();
    }
    bool isRoot() const override
    {
        return true;
    }

    void clear()
    {
        emit cleared();
        for (FileItem *fileitem : std::as_const(_fileItems))
            delete fileitem;
        _fileItems.clear();
        _foundText.clear();
    }

    void addItem(const FileItem &file, const QString &foundText)
    {
        const QString path = file.getUrl().toDisplayString(QUrl::PreferLocalFile);
        FileItem *fileitem = FileItem::createCopy(file, path);
        _fileItems << fileitem;
        if (!foundText.isEmpty())
            _foundText[fileitem] = foundText;
        emit addedFileItem(fileitem);
    }

    QString foundText(const FileItem *fileitem)
    {
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
KrSearchDialog::KrSearchDialog(const QString &profile, QWidget *parent)
    : QDialog(parent)
    , query(nullptr)
    , searcher(nullptr)
    , isBusy(false)
    , closed(false)
{
    KConfigGroup group(krConfig, "Search");

    setWindowTitle(i18n("Krusader::Search"));
    setWindowIcon(Icon("system-search"));

    auto *searchBaseLayout = new QGridLayout(this);
    searchBaseLayout->setSpacing(6);
    searchBaseLayout->setContentsMargins(11, 11, 11, 11);

    // creating the dialog buttons ( Search, Stop, Close )

    auto *buttonsLayout = new QHBoxLayout();
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

    auto *spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
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
    generalFilter = dynamic_cast<GeneralFilter *>(filterTabs->get("GeneralFilter"));

    // creating the result tab

    QWidget *resultTab = new QWidget(searcherTabs);
    auto *resultLayout = new QGridLayout(resultTab);
    resultLayout->setSpacing(6);
    resultLayout->setContentsMargins(6, 6, 6, 6);

    // actions
    viewAction = new QAction(Icon("document-edit-preview"), i18n("View File"), this);
    viewAction->setShortcut(Qt::Key_F3);
    connect(viewAction, &QAction::triggered, this, &KrSearchDialog::viewCurrent);
    addAction(viewAction);

    editAction = new QAction(Icon("document-edit-sign"), i18n("Edit File"), this);
    editAction->setShortcut(Qt::Key_F4);
    connect(editAction, &QAction::triggered, this, &KrSearchDialog::editCurrent);
    addAction(editAction);

    compareAction = new QAction(i18n("Compare by content"), this);
    compareAction->setShortcut(Qt::Key_F10);
    connect(compareAction, &QAction::triggered, this, &KrSearchDialog::compareByContent);
    addAction(compareAction);

    copyAction = KStandardAction::create(KStandardAction::Copy, this, SLOT(copyToClipBoard()), this);
    copyAction->setText(i18n("Copy to Clipboard"));
    addAction(copyAction);

    connect(searcherTabs, &QTabWidget::currentChanged, this, [=](int index) {
        bool resultTabShown = index == searcherTabs->indexOf(resultTab);
        viewAction->setEnabled(resultTabShown);
        editAction->setEnabled(resultTabShown);
        compareAction->setEnabled(resultTabShown);
        copyAction->setEnabled(resultTabShown);
    });

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

    auto *foundTextLayout = new QHBoxLayout();
    foundTextLayout->setSpacing(6);

    QLabel *textFoundLabel = new QLabel(i18n("Text found:"), resultTab);
    foundTextLayout->addWidget(textFoundLabel);
    foundTextLabel = new KrSqueezedTextLabel(resultTab);
    foundTextLayout->addWidget(foundTextLabel);

    foundTextFrame->setLayout(foundTextLayout);
    foundTextFrame->hide();

    resultLayout->addWidget(foundTextFrame, 2, 0);

    // result info row
    auto *resultLabelLayout = new QHBoxLayout();
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

    sizeX = group.readEntry("Window Width", -1);
    sizeY = group.readEntry("Window Height", -1);

    if (sizeX != -1 && sizeY != -1)
        resize(sizeX, sizeY);

    if (group.readEntry("Window Maximized", false))
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
        profileManager->loadProfile(profile); // important: call this _after_ you've connected profileManager ot the loadFromProfile!!
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
    if (isBusy) {
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
        delete query;
        query = nullptr;
    }
    query = new KrQuery();

    return filterTabs->fillQuery(query);
}

void KrSearchDialog::startSearch()
{
    if (isBusy)
        return;

    // prepare the query /////////////////////////////////////////////
    if (!gui2query())
        return;

    // first, informative messages
    if (query->searchInArchives()) {
        KMessageBox::information(this,
                                 i18n("Since you chose to also search in archives, "
                                      "note the following limitations:\n"
                                      "You cannot search for text (grep) while doing"
                                      " a search that includes archives."),
                                 nullptr,
                                 "searchInArchives");
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
    searcher = new KrSearchMod(query);
    connect(searcher, &KrSearchMod::searching, searchingLabel, &KSqueezedTextLabel::setText);
    connect(searcher, &KrSearchMod::found, this, &KrSearchDialog::slotFound);
    connect(searcher, &KrSearchMod::finished, this, &KrSearchDialog::stopSearch);

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

    // go to the first result. Note: `getFirst()` doesn't cause problems
    // if the results list is empty
    resultView->setCurrentKrViewItem(resultView->getFirst());

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
    if (!name.endsWith('/')) {
        // not a directory, split filename and path
        qsizetype idx = name.lastIndexOf("/");
        fileName = name.mid(idx + 1);
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
    if (!item)
        return;
    QString text = result->foundText(item->getFileItem());
    if (!text.isEmpty()) {
        // ugly hack: find the <b> and </b> in the text, then
        // use it to set the are which we don't want squeezed
        qsizetype idx = text.indexOf("<b>") - 4; // take "<qt>" into account
        qsizetype length = text.indexOf("</b>") - idx + 4;
        foundTextLabel->setText(text, idx, length);
    }
}

void KrSearchDialog::closeEvent(QCloseEvent *e)
{ /* if searching is in progress we must not close the window */
    if (isBusy) /* because qApp->processEvents() is called by the searcher and */
    { /* at window destruction, the searcher object will be deleted */
        stopSearch(); /* instead we stop searching */
        closed = true; /* and after stopping: startSearch can close the window */
        e->ignore(); /* ignoring the close event */
    } else
        QDialog::closeEvent(e); /* if no searching, let QDialog handle the event */
}

void KrSearchDialog::keyPressEvent(QKeyEvent *e)
{
    // TODO: don't use hardcoded shortcuts

    if (isBusy && e->key() == Qt::Key_Escape) { /* at searching we must not close the window */
        stopSearch(); /* so we simply stop searching */
        return;
    }
    if (resultView->widget()->hasFocus()) {
        if (e->keyCombination() == QKeyCombination(Qt::CTRL, Qt::Key_I)) {
            searchBar->showBar(KrSearchBar::MODE_FILTER);
        }
        if (e->key() == Qt::Key_Menu) {
            contextMenu(QCursor::pos());
        }
    }
    QDialog::keyPressEvent(e);
}

void KrSearchDialog::editCurrent()
{
    tryPlaceSearchQueryToClipboard();
    KrViewItem *current = resultView->getCurrentKrViewItem();
    if (current)
        KrViewer::edit(current->getFileItem()->getUrl(), this);
}

void KrSearchDialog::viewCurrent()
{
    tryPlaceSearchQueryToClipboard();
    KrViewItem *current = resultView->getCurrentKrViewItem();
    if (current)
        KrViewer::view(current->getFileItem()->getUrl(), this);
}

void KrSearchDialog::compareByContent()
{
    const KrViewItemList list = resultView->getSelectedKrViewItems();
    if (list.count() != 2)
        return;

    SLOTS->compareContent(list[0]->getFileItem()->getUrl(), list[1]->getFileItem()->getUrl());
}

void KrSearchDialog::contextMenu(const QPoint &pos)
{
    // create the menu
    QMenu popup;
    popup.setTitle(i18n("Krusader Search"));

    popup.addAction(viewAction);
    popup.addAction(editAction);
    popup.addAction(compareAction);
    compareAction->setEnabled(resultView->numSelected() == 2);
    popup.addSeparator();
    popup.addAction(copyAction);

    popup.exec(pos);
}

void KrSearchDialog::feedToListBox()
{
    VirtualFileSystem virtFilesystem;
    virtFilesystem.scanDir(QUrl::fromLocalFile("/"));

    KConfigGroup group(krConfig, "Search");
    int listBoxNum = group.readEntry("Feed To Listbox Counter", 1);
    QString queryName;
    if (query) {
        const QString where = KrServices::toStringList(query->searchInDirs()).join(", ");
        queryName = query->content().isEmpty() ? i18n("Search results for \"%1\" in %2", query->nameFilter(), where)
                                               : i18n("Search results for \"%1\" containing \"%2\" in %3", query->nameFilter(), query->content(), where);
    }
    QString fileSystemName;
    do {
        fileSystemName = i18n("Search results") + QString(" %1").arg(listBoxNum++);
    } while (virtFilesystem.getFileItem(fileSystemName) != nullptr);
    group.writeEntry("Feed To Listbox Counter", listBoxNum);

    KConfigGroup ga(krConfig, "Advanced");
    if (ga.readEntry("Confirm Feed to Listbox", _ConfirmFeedToListbox)) {
        bool ok;
        fileSystemName = QInputDialog::getText(this, i18n("Query name"), i18n("Here you can name the file collection"), QLineEdit::Normal, fileSystemName, &ok);
        if (!ok)
            return;
    }

    QList<QUrl> urlList;
    const auto fileItems = result->fileItems();
    for (FileItem *fileitem : fileItems)
        urlList.push_back(fileitem->getUrl());

    mainSearchBtn->setEnabled(false);
    mainCloseBtn->setEnabled(false);
    mainFeedToListBoxBtn->setEnabled(false);

    isBusy = true;

    const QUrl url = QUrl(QString("virt:/") + fileSystemName);
    virtFilesystem.scanDir(url);
    virtFilesystem.addFiles(urlList);
    virtFilesystem.setMetaInformation(queryName);
    // ACTIVE_FUNC->openUrl(url);
    ACTIVE_MNG->slotNewTab(url);

    isBusy = false;

    closeDialog();
}

void KrSearchDialog::copyToClipBoard()
{
    const KrViewItemList selectedItems = resultView->getSelectedKrViewItems();
    QList<QUrl> urls;
    for (KrViewItem *item : selectedItems) {
        urls.append(item->getFileItem()->getUrl());
    }

    if (urls.count() == 0)
        return;

    auto *mimeData = new QMimeData;
    mimeData->setImageData(FileListIcon("file").pixmap());
    mimeData->setUrls(urls);

    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

void KrSearchDialog::tryPlaceSearchQueryToClipboard()
{
    if (searchTextToClipboard->isChecked() && !generalFilter->containsText->currentText().isEmpty()
        && QApplication::clipboard()->text() != generalFilter->containsText->currentText()) {
        QApplication::clipboard()->setText(generalFilter->containsText->currentText());
    }
}
