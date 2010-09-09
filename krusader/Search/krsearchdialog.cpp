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
#include <QtGui/QFontMetrics>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QGridLayout>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QtGui/QCursor>
#include <QtGui/QClipboard>
#include <qheaderview.h>
#include <QDrag>
#include <QMimeData>

#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kmenu.h>

#include "krsearchmod.h"
#include "../krglobal.h"
#include "../kractions.h"
#include "../krslots.h"
#include "../defaults.h"
#include "../panelmanager.h"
#include "../VFS/vfs.h"
#include "../krusaderview.h"
#include "../Panel/krpanel.h"
#include "../Panel/panelfunc.h"
#include "../Dialogs/krdialogs.h"
#include "../VFS/virt_vfs.h"
#include "../KViewer/krviewer.h"
#include "../kicons.h"
#include "../GUI/krtreewidget.h"

class SearchListView : public KrTreeWidget
{
public:
    SearchListView(QWidget * parent) : KrTreeWidget(parent) {
        setColumnCount(5);

        QStringList labels;
        labels << i18n("Name");
        labels << i18n("Location");
        labels << i18n("Size");
        labels << i18n("Date");
        labels << i18n("Permissions");

        setHeaderLabels(labels);

        sortItems(1, Qt::AscendingOrder);
        setSelectionMode(QAbstractItemView::ExtendedSelection);

        // fix the results list
        // => make the results font smaller
        QFont resultsFont(font());
        resultsFont.setPointSize(resultsFont.pointSize() - 1);
        setFont(resultsFont);

        int i = QFontMetrics(font()).width("W");
        int j = QFontMetrics(font()).width("0");
        j = (i > j ? i : j);

        KConfigGroup group(krConfig, "Search");
        if (group.hasKey("Last State"))
            header()->restoreState(group.readEntry("Last State", QByteArray()));
        else {
            setColumnWidth(0, j*14);
            setColumnWidth(1, j*25);
            setColumnWidth(2, j*6);
            setColumnWidth(3, j*7);
            setColumnWidth(4, j*7);
        }

        header()->setResizeMode(0, QHeaderView::Interactive);
        header()->setResizeMode(1, QHeaderView::Interactive);
        header()->setResizeMode(2, QHeaderView::Interactive);
        header()->setResizeMode(3, QHeaderView::Interactive);
        header()->setResizeMode(4, QHeaderView::Interactive);

        setStretchingColumn(1);

        setDragEnabled(true);
    }

    virtual void startDrag(Qt::DropActions supportedActs) {
        KUrl::List urls;
        QList<QTreeWidgetItem *> list = selectedItems() ;

        QListIterator<QTreeWidgetItem *> it(list);

        while (it.hasNext()) {
            QTreeWidgetItem * item = it.next();

            QString name = item->text(1);
            name += (name.endsWith('/') ? item->text(0) : '/' + item->text(0));
            urls.push_back(KUrl(name));
        }

        if (urls.count() == 0)
            return;

        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        mimeData->setImageData(FL_LOADICON("file"));
        urls.populateMimeData(mimeData);
        drag->setMimeData(mimeData);
        drag->start();
    }
};


class SearchListViewItem : public QTreeWidgetItem
{
public:
    SearchListViewItem(QTreeWidget *resultsList, QString name, QString where, KIO::filesize_t size,
                       QDateTime date, QString perm) : QTreeWidgetItem(resultsList) {
        setText(0, name);
        setText(1, where);
        setText(2, KRpermHandler::parseSize(size));
        setText(3, KGlobal::locale()->formatDateTime(date));
        setText(4, perm);

        setTextAlignment(2, Qt::AlignRight);
        fileSize = size;
        fileDate = date;
    }

    void setFoundText(QString text) {
        _foundText = text;
    }
    const QString& foundText() const {
        return _foundText;
    }

    virtual bool operator<(const QTreeWidgetItem &other) const {
        int column = treeWidget() ? treeWidget()->sortColumn() : 0;

        switch (column) {
        case 2:
            return fileSize < ((SearchListViewItem &)other).fileSize;
        case 3:
            return fileDate < ((SearchListViewItem &)other).fileDate;
        default:
            return text(column) < other.text(column);
        }
    }

private:
    KIO::filesize_t fileSize;
    QDateTime       fileDate;
    QString         _foundText;
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

// class starts here /////////////////////////////////////////
KrSearchDialog::KrSearchDialog(QString profile, QWidget* parent)
        : QDialog(parent), query(0), searcher(0)
{
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

    resultTab = new QWidget(searcherTabs);
    resultLayout = new QGridLayout(resultTab);
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

    resultLayout->addLayout(resultLabelLayout, 2, 0);

    // creating the result list view

    resultsList = new SearchListView(resultTab);

    resultLayout->addWidget(resultsList, 0, 0);

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
    resultLayout->addLayout(foundTextLayout, 1, 0);

    searcherTabs->addTab(resultTab, i18n("&Results"));

    searchBaseLayout->addWidget(searcherTabs, 0, 0);

    // signals and slots connections

    connect(mainSearchBtn, SIGNAL(clicked()), this, SLOT(startSearch()));
    connect(mainStopBtn, SIGNAL(clicked()), this, SLOT(stopSearch()));
    connect(resultsList, SIGNAL(itemActivated(QTreeWidgetItem*, int)), this,
            SLOT(resultDoubleClicked(QTreeWidgetItem*)));
    connect(resultsList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this,
            SLOT(resultDoubleClicked(QTreeWidgetItem*)));
    connect(resultsList, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this,
            SLOT(resultClicked(QTreeWidgetItem*)));
    connect(resultsList, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this,
            SLOT(resultClicked(QTreeWidgetItem*)));
    connect(resultsList, SIGNAL(itemRightClicked(QTreeWidgetItem*, const QPoint &, int)), this, SLOT(rightClickMenu(QTreeWidgetItem*, const QPoint &)));
    connect(mainCloseBtn, SIGNAL(clicked()), this, SLOT(closeDialog()));
    connect(mainFeedToListBoxBtn, SIGNAL(clicked()), this, SLOT(feedToListBox()));

    connect(profileManager, SIGNAL(loadFromProfile(QString)), filterTabs, SLOT(loadFromProfile(QString)));
    connect(profileManager, SIGNAL(saveToProfile(QString)), filterTabs, SLOT(saveToProfile(QString)));

    // tab order

    setTabOrder(mainSearchBtn, mainCloseBtn);
    setTabOrder(mainCloseBtn, mainStopBtn);
    setTabOrder(mainStopBtn, searcherTabs);
    setTabOrder(searcherTabs, resultsList);

    KConfigGroup group(krConfig, "Search");
    sizeX = group.readEntry("Window Width",  -1);
    sizeY = group.readEntry("Window Height",  -1);

    if (sizeX != -1 && sizeY != -1)
        resize(sizeX, sizeY);

    if (group.readEntry("Window Maximized",  false))
        showMaximized();
    else
        show();

    generalFilter->searchFor->setFocus();

    isSearching = closed = false;

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
}

void KrSearchDialog::closeDialog(bool isAccept)
{
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

    group.writeEntry("Last State", resultsList->header()->saveState());

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
    // convert the time_t to struct tm
    struct tm* t = localtime((time_t *) & mtime);
    QDateTime tmp(QDate(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday), QTime(t->tm_hour, t->tm_min));
    SearchListViewItem *it = new SearchListViewItem(resultsList, what,
            where.replace(QRegExp("\\\\"), "#"), size, tmp, perm);
    QString totals = QString(i18n("Found %1 matches.", resultsList->topLevelItemCount()));
    foundLabel->setText(totals);

    if (!foundText.isEmpty()) it->setFoundText(foundText);
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
    resultsList->clear();
    searchingLabel->setText("");
    foundLabel->setText(i18n("Found 0 matches."));
    searcherTabs->setCurrentIndex(2); // show the results page
    foundTextLabel->setText("");
    qApp->processEvents();

    // start the search.
    if (searcher != 0) {
        delete searcher;
        searcher = 0;
    }
    searcher  = new KRSearchMod(query);
    connect(searcher, SIGNAL(searching(const QString&)),
            searchingLabel, SLOT(setText(const QString&)));
    connect(searcher, SIGNAL(found(QString, QString, KIO::filesize_t, time_t, QString, QString)),
            this, SLOT(found(QString, QString, KIO::filesize_t, time_t, QString, QString)));
    connect(searcher, SIGNAL(finished()), this, SLOT(stopSearch()));

    isSearching = true;
    searcher->start();
    isSearching = false;
    if (closed)
        emit closeDialog();
}

void KrSearchDialog::stopSearch()
{
    if (searcher != 0) {
        searcher->stop();
        disconnect(searcher, 0, 0, 0);
    }

    // gui stuff
    mainSearchBtn->setEnabled(true);
    mainCloseBtn->setEnabled(true);
    mainStopBtn->setEnabled(false);
    if (resultsList->topLevelItemCount())
        mainFeedToListBoxBtn->setEnabled(true);
    searchingLabel->setText(i18n("Finished searching."));
}

void KrSearchDialog::resultDoubleClicked(QTreeWidgetItem* i)
{
    ACTIVE_FUNC->openUrl(KUrl(i->text(1)), i->text(0));
    showMinimized();
}

void KrSearchDialog::resultClicked(QTreeWidgetItem* i)
{
    SearchListViewItem *it = dynamic_cast<SearchListViewItem*>(i);
    if (it == 0)
        return;
    if (!it->foundText().isEmpty()) {
        // ugly hack: find the <b> and </b> in the text, then
        // use it to set the are which we don't want squeezed
        int idx = it->foundText().indexOf("<b>") - 4; // take "<qt>" into account
        int length = it->foundText().indexOf("</b>") - idx + 4;
        foundTextLabel->setText(it->foundText(), idx, length);
    }
}

void KrSearchDialog::closeEvent(QCloseEvent *e)
{                     /* if searching is in progress we must not close the window */
    if (isSearching)    /* because qApp->processEvents() is called by the searcher and */
    {                   /* at window desruction, the searcher object will be deleted */
        stopSearch();         /* instead we stop searching */
        closed = true;        /* and after stopping: startSearch can close the window */
        e->ignore();          /* ignoring the close event */
    } else
        QDialog::closeEvent(e);     /* if no searching, let QDialog handle the event */
}

void KrSearchDialog::keyPressEvent(QKeyEvent *e)
{
    if (isSearching && e->key() == Qt::Key_Escape) { /* at searching we must not close the window */
        stopSearch();         /* so we simply stop searching */
        return;
    }
    if (resultsList->hasFocus()) {
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
        } else if (krCopy->shortcut().contains(QKeySequence(e->key() | e->modifiers()))) {
            copyToClipBoard();
            return;
        }
    }

    QDialog::keyPressEvent(e);
}

void KrSearchDialog::editCurrent()
{
    QTreeWidgetItem *current = resultsList->currentItem();
    if (current) {
        QString name = current->text(1);
        name += (name.endsWith('/') ? current->text(0) : '/' + current->text(0));
        KUrl url = KUrl(name);
        KrViewer::edit(url, this);
    }
}

void KrSearchDialog::viewCurrent()
{
    QTreeWidgetItem *current = resultsList->currentItem();
    if (current) {
        QString name = current->text(1);
        name += (name.endsWith('/') ? current->text(0) : '/' + current->text(0));
        KUrl url = KUrl(name);
        KrViewer::view(url, this);
    }
}

void KrSearchDialog::compareByContent()
{
    QList<QTreeWidgetItem *> list = resultsList->selectedItems();
    if (list.count() != 2)
        return;

    QString name1 = list[ 0 ]->text(1);
    name1 += (name1.endsWith('/') ? list[ 0 ]->text(0) : '/' + list[ 0 ]->text(0));
    KUrl url1 = KUrl(name1);

    QString name2 = list[ 1 ]->text(1);
    name2 += (name2.endsWith('/') ? list[ 1 ]->text(0) : '/' + list[ 1 ]->text(0));
    KUrl url2 = KUrl(name2);

    SLOTS->compareContent(url1, url2);
}

void KrSearchDialog::rightClickMenu(QTreeWidgetItem * item, const QPoint &pos)
{
    if (!item)
        return;

    // create the menu
    KMenu popup;
    popup.setTitle(i18n("Krusader Search"));

    QAction *actView = popup.addAction(i18n("View File (F3)"));
    QAction *actEdit = popup.addAction(i18n("Edit File (F4)"));
    QAction *actComp = popup.addAction(i18n("Compare by content (F10)"));
    if (resultsList->selectedItems().count() != 2)
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
/*    if(query) {
        QString where = query->searchInDirs().toStringList().join(", ");
        if(query->content().isEmpty())
            queryName = i18n("Search results for \"%1\" in %2", query->nameFilter(), where);
        else
            queryName = i18n("Search results for \"%1\" containing \"%2\" in %3", query->nameFilter(), query->content(), where);
    }*/
    do {
        queryName = i18n("Search results") + QString(" %1").arg(listBoxNum++);
    } while (v.vfs_search(queryName) != 0);
    group.writeEntry("Feed To Listbox Counter", listBoxNum);

    KConfigGroup ga(krConfig, "Advanced");
    if (ga.readEntry("Confirm Feed to Listbox",  _ConfirmFeedToListbox)) {
        bool ok;
        queryName = KInputDialog::getText(
                        i18n("Query name"),  // Caption
                        i18n("Here you can name the file collection"), // Questiontext
                        queryName, // Default
                        &ok, this);
        if (! ok)
            return;
    }

    KUrl::List urlList;

    QTreeWidgetItemIterator it(resultsList);
    while (*it) {
        QTreeWidgetItem * item = *it;

        QString name = item->text(1);
        name += (name.endsWith('/') ? item->text(0) : '/' + item->text(0));
        urlList.push_back(KUrl(name));

        ++it;
    }

    KUrl url = KUrl(QString("virt:/") + queryName);
    v.vfs_refresh(url);
    v.vfs_addFiles(&urlList, KIO::CopyJob::Copy, 0);
    //ACTIVE_FUNC->openUrl(url);
    ACTIVE_MNG->slotNewTab(url.prettyUrl());
    closeDialog();
}

void KrSearchDialog::copyToClipBoard()
{
    KUrl::List urls;


    QTreeWidgetItemIterator it(resultsList);
    while (*it) {
        QTreeWidgetItem * item = *it;

        if (item->isSelected()) {
            QString name = item->text(1);
            name += (name.endsWith('/') ? item->text(0) : '/' + item->text(0));
            urls.push_back(KUrl(name));
        }

        ++it;
    }

    if (urls.count() == 0)
        return;

    QMimeData *mimeData = new QMimeData;
    mimeData->setImageData(FL_LOADICON("file"));
    urls.populateMimeData(mimeData);

    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

#include "krsearchdialog.moc"
