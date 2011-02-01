/***************************************************************************
                                 krsearchdialog.cpp
                             -------------------
    copyright            : (C) 2001 by Shie Erlich & Rafi Yanai
    email                : krusader@users.sourceforge.net
    web site   : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "krsearchdialog.h"

#include <QtCore/QRegExp>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QGridLayout>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QtGui/QCursor>
#include <QtGui/QClipboard>
#include <QDrag>
#include <QMimeData>
#include <QtGui/QResizeEvent>
#include <QtGui/QGridLayout>
#include <QtGui/QLabel>
#include <QtGui/QKeyEvent>
#include <QtGui/QCloseEvent>
#include <QtGui/QTabWidget>

#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kmenu.h>
#include <KGlobal>
#include <KLocale>

#include "krsearchmod.h"
#include "../krglobal.h"
#include "../kractions.h"
#include "../krslots.h"
#include "../defaults.h"
#include "../panelmanager.h"
#include "../kicons.h"
#include "../krusaderview.h"
#include "../Dialogs/krdialogs.h"
#include "../Dialogs/krspecialwidgets.h"
#include "../Dialogs/krsqueezedtextlabel.h"
#include "../VFS/virt_vfs.h"
#include "../VFS/krquery.h"
#include "../KViewer/krviewer.h"
#include "../Panel/krview.h"
#include "../Panel/krviewfactory.h"
#include "../Panel/quickfilter.h"
#include "../Panel/krpanel.h"
#include "../Panel/panelfunc.h"
#include "../Filter/filtertabs.h"
#include "../Filter/generalfilter.h"


#define RESULTVIEW_TYPE 0

class SearchResultContainer : public VfileContainer
{
public:
    SearchResultContainer(QObject *parent) : VfileContainer(parent) {}
    virtual ~SearchResultContainer() {
        clear();
    }

    virtual QList<vfile*> vfiles() {
        return _vfiles;
    }
    virtual unsigned long numVfiles() {
        return _vfiles.count();
    }
    virtual bool isRoot() {
        return true;
    }

    void clear() {
        emit cleared();
        foreach(vfile *vf, _vfiles)
            delete vf;
        _vfiles.clear();
        _foundText.clear();
    }

    void addItem(QString path, KIO::filesize_t size, time_t mtime, QString perm, QString foundText)
    {
        vfile *vf = new vfile(path, size, perm, mtime, false/*FIXME*/, 0, 0, QString(), QString(), 0);
        vf->vfile_setUrl(KUrl(path));
        _vfiles << vf;
        if(!foundText.isEmpty())
            _foundText[vf] = foundText;
        emit addedVfile(vf);
    }

    QString foundText(const vfile *vf) {
        return _foundText[vf];
    }

private:
    QList<vfile*> _vfiles;
    QHash<const vfile*, QString> _foundText;
};


KrSearchDialog *KrSearchDialog::SearchDialog = 0;

QString KrSearchDialog::lastSearchText = "*";
int KrSearchDialog::lastSearchType = 0;
bool KrSearchDialog::lastSearchForCase = false;
bool KrSearchDialog::lastRemoteContentSearch = false;
bool KrSearchDialog::lastContainsWholeWord = false;
bool KrSearchDialog::lastContainsWithCase = true;
bool KrSearchDialog::lastSearchInSubDirs = true;
bool KrSearchDialog::lastSearchInArchives = false;
bool KrSearchDialog::lastFollowSymLinks = false;
bool KrSearchDialog::lastContainsRegExp = false;

bool _left = 0; // dummy - needed by krView

// class starts here /////////////////////////////////////////
KrSearchDialog::KrSearchDialog(QString profile, QWidget* parent)
        : QDialog(parent), query(0), searcher(0), isBusy(false), closed(false)
{
    KConfigGroup group(krConfig, "Search");

    setWindowTitle(i18n("Krusader::Search"));

    QGridLayout* searchBaseLayout = new QGridLayout(this);
    searchBaseLayout->setSpacing(6);
    searchBaseLayout->setContentsMargins(11, 11, 11, 11);

    // creating the dialog buttons ( Search, Stop, Close )

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    buttonsLayout->setSpacing(6);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);

    profileManager = new ProfileManager("SearcherProfile", this);
    buttonsLayout->addWidget(profileManager);

    QSpacerItem* spacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttonsLayout->addItem(spacer);

    mainFeedToListBoxBtn = new QPushButton(this);
    mainFeedToListBoxBtn->setText(i18n("Feed to listbox"));
    mainFeedToListBoxBtn->setEnabled(false);
    buttonsLayout->addWidget(mainFeedToListBoxBtn);

    mainSearchBtn = new QPushButton(this);
    mainSearchBtn->setText(i18n("Search"));
    mainSearchBtn->setDefault(true);
    buttonsLayout->addWidget(mainSearchBtn);

    mainStopBtn = new QPushButton(this);
    mainStopBtn->setEnabled(false);
    mainStopBtn->setText(i18n("Stop"));
    buttonsLayout->addWidget(mainStopBtn);

    mainCloseBtn = new QPushButton(this);
    mainCloseBtn->setText(i18n("Close"));
    buttonsLayout->addWidget(mainCloseBtn);

    searchBaseLayout->addLayout(buttonsLayout, 1, 0);

    // creating the searcher tabs

    searcherTabs = new KTabWidget(this);

    filterTabs = FilterTabs::addTo(searcherTabs, FilterTabs::Default | FilterTabs::HasRemoteContentSearch);
    generalFilter = (GeneralFilter *)filterTabs->get("GeneralFilter");

    QWidget* resultTab = new QWidget(searcherTabs);
    QGridLayout* resultLayout = new QGridLayout(resultTab);
    resultLayout->setSpacing(6);
    resultLayout->setContentsMargins(11, 11, 11, 11);

    // creating the result tab

    QHBoxLayout* resultLabelLayout = new QHBoxLayout();
    resultLabelLayout->setSpacing(6);
    resultLabelLayout->setContentsMargins(0, 0, 0, 0);

    foundLabel = new QLabel(resultTab);
    QSizePolicy foundpolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    foundpolicy.setHeightForWidth(foundLabel->sizePolicy().hasHeightForWidth());
    foundLabel->setSizePolicy(foundpolicy);
    foundLabel->setFrameShape(QLabel::StyledPanel);
    foundLabel->setFrameShadow(QLabel::Sunken);
    foundLabel->setText(i18n("Found 0 matches."));
    resultLabelLayout->addWidget(foundLabel);

    searchingLabel = new KSqueezedTextLabel(resultTab);
    searchingLabel->setFrameShape(QLabel::StyledPanel);
    searchingLabel->setFrameShadow(QLabel::Sunken);
    searchingLabel->setText("");
    resultLabelLayout->addWidget(searchingLabel);

    resultLayout->addLayout(resultLabelLayout, 3, 0);

    // creating the result list view
    result = new SearchResultContainer(this);
    // quicksearch
    KrQuickSearch *quickSearch = new KrQuickSearch(this);
    quickSearch->hide();
    resultLayout->addWidget(quickSearch, 1, 0);
    // quickfilter
    QuickFilter *quickFilter = new QuickFilter(this);
    quickFilter->hide();
    resultLayout->addWidget(quickFilter, 2, 0);
    // the view
    resultView = KrViewFactory::createView(RESULTVIEW_TYPE, resultTab, _left, krConfig);
    resultView->init();
    resultView->restoreSettings(KConfigGroup(&group, "ResultView"));
    resultView->enableUpdateDefaultSettings(false);
    resultView->setMainWindow(this);
    resultView->op()->setQuickSearch(quickSearch);
    resultView->op()->setQuickFilter(quickFilter);
    resultView->prepareForActive();
    resultView->refreshColors();
    resultView->setFiles(result);
    resultView->refresh();
    resultLayout->addWidget(resultView->widget(), 0, 0);

    QHBoxLayout* foundTextLayout = new QHBoxLayout();
    foundTextLayout->setSpacing(6);
    foundTextLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *l1 = new QLabel(resultTab);
    QSizePolicy l1policy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    l1policy.setHeightForWidth(l1->sizePolicy().hasHeightForWidth());
    l1->setSizePolicy(l1policy);
    l1->setFrameShape(QLabel::StyledPanel);
    l1->setFrameShadow(QLabel::Sunken);
    l1->setText(i18n("Text found:"));
    foundTextLayout->addWidget(l1);

    foundTextLabel = new KrSqueezedTextLabel(resultTab);
    foundTextLabel->setFrameShape(QLabel::StyledPanel);
    foundTextLabel->setFrameShadow(QLabel::Sunken);
    foundTextLabel->setText("");
    foundTextLayout->addWidget(foundTextLabel);
    resultLayout->addLayout(foundTextLayout, 4, 0);

    searcherTabs->addTab(resultTab, i18n("&Results"));

    searchBaseLayout->addWidget(searcherTabs, 0, 0);

    // signals and slots connections

    connect(mainSearchBtn, SIGNAL(clicked()), this, SLOT(startSearch()));
    connect(mainStopBtn, SIGNAL(clicked()), this, SLOT(stopSearch()));
    connect(mainCloseBtn, SIGNAL(clicked()), this, SLOT(closeDialog()));
    connect(mainFeedToListBoxBtn, SIGNAL(clicked()), this, SLOT(feedToListBox()));

    connect(profileManager, SIGNAL(loadFromProfile(QString)), filterTabs, SLOT(loadFromProfile(QString)));
    connect(profileManager, SIGNAL(saveToProfile(QString)), filterTabs, SLOT(saveToProfile(QString)));

    connect(resultView->op(), SIGNAL(currentChanged(KrViewItem*)), SLOT(currentChanged(KrViewItem*)));
    connect(resultView->op(), SIGNAL(executed(const QString&)), SLOT(executed(const QString&)));
    connect(resultView->op(), SIGNAL(contextMenu(const QPoint&)), SLOT(contextMenu(const QPoint &)));

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

    // finaly, load a profile of apply defaults:

    if (profile.isEmpty()) {
        // load the last used values
        generalFilter->searchFor->setEditText(lastSearchText);
        generalFilter->searchFor->lineEdit()->selectAll();
        generalFilter->ofType->setCurrentIndex(lastSearchType);
        generalFilter->searchForCase->setChecked(lastSearchForCase);
        generalFilter->remoteContentSearch->setChecked(lastRemoteContentSearch);
        generalFilter->containsWholeWord->setChecked(lastContainsWholeWord);
        generalFilter->containsTextCase->setChecked(lastContainsWithCase);
        generalFilter->containsRegExp->setChecked(lastContainsRegExp);
        generalFilter->searchInDirs->setChecked(lastSearchInSubDirs);
        generalFilter->searchInArchives->setChecked(lastSearchInArchives);
        generalFilter->followLinks->setChecked(lastFollowSymLinks);
        // the path in the active panel should be the default search location
        generalFilter->searchIn->lineEdit()->setText(ACTIVE_PANEL->virtualPath().prettyUrl());
    } else
        profileManager->loadProfile(profile);   // important: call this _after_ you've connected profileManager ot the loadFromProfile!!
}

KrSearchDialog::~KrSearchDialog()
{
    delete query;
    query = 0;
    delete resultView;
    resultView = 0;
}

void KrSearchDialog::closeDialog(bool isAccept)
{
    if(isBusy) {
        closed = true;
        return;
    }

    // stop the search if it's on-going
    if (searcher != 0) {
        delete searcher;
        searcher = 0;
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
    lastRemoteContentSearch = generalFilter->remoteContentSearch->isChecked();
    lastContainsWholeWord = generalFilter->containsWholeWord->isChecked();
    lastContainsWithCase = generalFilter->containsTextCase->isChecked();
    lastContainsRegExp = generalFilter->containsRegExp->isChecked();
    lastSearchInSubDirs = generalFilter->searchInDirs->isChecked();
    lastSearchInArchives = generalFilter->searchInArchives->isChecked();
    lastFollowSymLinks = generalFilter->followLinks->isChecked();
    hide();

    SearchDialog = 0;
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

void KrSearchDialog::found(QString what, QString where, KIO::filesize_t size, time_t mtime, QString perm, QString foundText)
{
    where = where.replace(QRegExp("\\\\"), "#"); //FIXME ? why is that done ?
    QString path =  where.endsWith('/') ? (where + what) : (where + "/" + what);
    if(perm[0] == 'd' && !path.endsWith('/')) // file is a directory
        path += '/';
    result->addItem(path, size, mtime, perm, foundText);
    foundLabel->setText(i18n("Found %1 matches.", result->numVfiles()));
}

bool KrSearchDialog::gui2query()
{
    // prepare the query ...
    /////////////////// names, locations and greps
    if (query != 0) {
        delete query; query = 0;
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
                                            " a search that includes archives."), 0, "searchInArchives");
    }

    // prepare the gui ///////////////////////////////////////////////
    mainSearchBtn->setEnabled(false);
    mainCloseBtn->setEnabled(false);
    mainStopBtn->setEnabled(true);
    mainFeedToListBoxBtn->setEnabled(false);
    result->clear();
    resultView->setSortMode(KrViewProperties::NoColumn, 0);
    searchingLabel->setText("");
    foundLabel->setText(i18n("Found 0 matches."));
    searcherTabs->setCurrentIndex(2); // show the results page
    foundTextLabel->setText("");

    isBusy = true;

    qApp->processEvents();

    // start the search.
    if (searcher != 0)
        abort();
    searcher  = new KRSearchMod(query);
    connect(searcher, SIGNAL(searching(const QString&)),
            searchingLabel, SLOT(setText(const QString&)));
    connect(searcher, SIGNAL(found(QString, QString, KIO::filesize_t, time_t, QString, QString)),
            this, SLOT(found(QString, QString, KIO::filesize_t, time_t, QString, QString)));
    connect(searcher, SIGNAL(finished()), this, SLOT(stopSearch()));

    searcher->start();

    isBusy = false;

    delete searcher;
    searcher = 0;

    // gui stuff
    mainSearchBtn->setEnabled(true);
    mainCloseBtn->setEnabled(true);
    mainStopBtn->setEnabled(false);
    if (result->numVfiles())
        mainFeedToListBoxBtn->setEnabled(true);
    searchingLabel->setText(i18n("Finished searching."));

    if (closed)
        closeDialog();
}

void KrSearchDialog::stopSearch()
{
    if (searcher != 0) {
        searcher->stop();
        disconnect(searcher, 0, 0, 0);
    }
}

void KrSearchDialog::executed(const QString &name)
{
    QString path = name;
    QString fileName;
    if(!name.endsWith('/')) {
        int idx = name.lastIndexOf("/");
        fileName = name.mid(idx+1);
        path = name.left(idx);
    }
    ACTIVE_FUNC->openUrl(KUrl(path), fileName);
    showMinimized();
}

void KrSearchDialog::currentChanged(KrViewItem *item)
{
    if(!item)
        return;
    QString text = result->foundText(item->getVfile());
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
    {                   /* at window desruction, the searcher object will be deleted */
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
        if ((e->key() | e->modifiers()) == (Qt::CTRL | Qt::Key_I))
            resultView->op()->startQuickFilter();
        if (e->key() == Qt::Key_F4) {
            if (!generalFilter->containsText->currentText().isEmpty() && QApplication::clipboard()->text() != generalFilter->containsText->currentText())
                QApplication::clipboard()->setText(generalFilter->containsText->currentText());
            editCurrent();
            return;
        } else if (e->key() == Qt::Key_F3) {
            if (!generalFilter->containsText->currentText().isEmpty() && QApplication::clipboard()->text() != generalFilter->containsText->currentText())
                QApplication::clipboard()->setText(generalFilter->containsText->currentText());
            viewCurrent();
            return;
        } else if (e->key() == Qt::Key_F10) {
            compareByContent();
            return;
        } else if (KrGlobal::copyShortcut.contains(QKeySequence(e->key() | e->modifiers()))) {
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
        KrViewer::edit(current->getVfile()->vfile_getUrl(), this);
}

void KrSearchDialog::viewCurrent()
{
    KrViewItem *current = resultView->getCurrentKrViewItem();
    if (current) 
        KrViewer::view(current->getVfile()->vfile_getUrl(), this);
}

void KrSearchDialog::compareByContent()
{
    KrViewItemList list;
    resultView->getSelectedKrViewItems(&list);
    if (list.count() != 2)
        return;

    SLOTS->compareContent(list[0]->getVfile()->vfile_getUrl(), list[1]->getVfile()->vfile_getUrl());
}

void KrSearchDialog::contextMenu(const QPoint &pos)
{
    // create the menu
    KMenu popup;
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
    virt_vfs v(0, true);
    v.vfs_refresh(KUrl("/"));

    KConfigGroup group(krConfig, "Search");
    int listBoxNum = group.readEntry("Feed To Listbox Counter", 1);
    QString queryName;
    if(query) {
        QString where = query->searchInDirs().toStringList().join(", ");
        if(query->content().isEmpty())
            queryName = i18n("Search results for \"%1\" in %2", query->nameFilter(), where);
        else
            queryName = i18n("Search results for \"%1\" containing \"%2\" in %3", query->nameFilter(), query->content(), where);
    }
    QString vfsName;
    do {
        vfsName = i18n("Search results") + QString(" %1").arg(listBoxNum++);
    } while (v.vfs_search(vfsName) != 0);
    group.writeEntry("Feed To Listbox Counter", listBoxNum);

    KConfigGroup ga(krConfig, "Advanced");
    if (ga.readEntry("Confirm Feed to Listbox",  _ConfirmFeedToListbox)) {
        bool ok;
        vfsName = KInputDialog::getText(
                        i18n("Query name"),  // Caption
                        i18n("Here you can name the file collection"), // Questiontext
                        vfsName, // Default
                        &ok, this);
        if (! ok)
            return;
    }

    KUrl::List urlList;
    foreach(vfile *vf, result->vfiles())
        urlList.push_back(vf->vfile_getUrl());

    mainSearchBtn->setEnabled(false);
    mainCloseBtn->setEnabled(false);
    mainFeedToListBoxBtn->setEnabled(false);

    isBusy = true;

    KUrl url = KUrl(QString("virt:/") + vfsName);
    v.vfs_refresh(url);
    v.vfs_addFiles(&urlList, KIO::CopyJob::Copy, 0);
    v.setMetaInformation(queryName);
    //ACTIVE_FUNC->openUrl(url);
    ACTIVE_MNG->slotNewTab(url.prettyUrl());

    isBusy = false;

    closeDialog();
}

void KrSearchDialog::copyToClipBoard()
{
    KUrl::List urls;
    foreach(vfile *vf, result->vfiles())
        urls.push_back(vf->vfile_getUrl());

    if (urls.count() == 0)
        return;

    QMimeData *mimeData = new QMimeData;
    mimeData->setImageData(FL_LOADICON("file"));
    urls.populateMimeData(mimeData);

    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

#include "krsearchdialog.moc"
