/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "popularurls.h"

#include <stdio.h>

// QtCore
#include <QList>
// QtWidgets
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QToolButton>

#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>
#include <KTreeWidgetSearchLine>

#include "../GUI/krtreewidget.h"
#include "../icon.h"
#include "../krglobal.h"
#include "../krslots.h"

#define STARTING_RANK 20
#define INCREASE 2
#define DECREASE 1

PopularUrls::PopularUrls(QObject *parent)
    : QObject(parent)
    , head(nullptr)
    , tail(nullptr)
    , count(0)
{
    dlg = new PopularUrlsDlg();
}

PopularUrls::~PopularUrls()
{
    clearList();
    delete dlg;
}

void PopularUrls::clearList()
{
    if (head) {
        UrlNodeP p = head, tmp;
        while (p) {
            tmp = p;
            p = p->next;
            delete tmp;
        }
    }
    ranks.clear();
    head = tail = nullptr;
}

void PopularUrls::save()
{
    KConfigGroup svr(krConfig, "Private");
    // prepare the string list containing urls and int list with ranks
    QStringList urlList;
    QList<int> rankList;
    UrlNodeP p = head;
    while (p) {
        urlList << p->url.toDisplayString();
        rankList << p->rank;
        p = p->next;
    }
    svr.writeEntry("PopularUrls", urlList);
    svr.writeEntry("PopularUrlsRank", rankList);
}

void PopularUrls::load()
{
    KConfigGroup svr(krConfig, "Private");
    QStringList urlList = svr.readEntry("PopularUrls", QStringList());
    QList<int> rankList = svr.readEntry("PopularUrlsRank", QList<int>());
    if (urlList.count() != rankList.count()) {
        KMessageBox::error(krMainWindow, i18n("The saved 'Popular URLs' are invalid. The list will be cleared."));
        return;
    }
    clearList();
    count = 0;
    // iterate through both lists and
    QStringList::Iterator uit;
    QList<int>::Iterator rit;
    for (uit = urlList.begin(), rit = rankList.begin(); uit != urlList.end() && rit != rankList.end(); ++uit, ++rit) {
        auto node = new UrlNode;
        node->url = QUrl(*uit);
        node->rank = *rit;
        appendNode(node);
        ranks.insert(*uit, node);
    }
}

// returns a url list with the 'max' top popular urls
QList<QUrl> PopularUrls::getMostPopularUrls(int max)
{
    // get at most 'max' urls
    QList<QUrl> list;
    UrlNodeP p = head;
    int tmp = 0;
    if (maxUrls < max)
        max = maxUrls; // don't give more than maxUrls
    while (p && tmp < max) {
        list << p->url;
        p = p->next;
        ++tmp;
    }

    return list;
}

// adds a url to the list, or increase rank of an existing url, making
// sure to bump it up the list if needed
void PopularUrls::addUrl(const QUrl &url)
{
    QUrl tmpurl = url;
    tmpurl.setPassword(QString()); // make sure no passwords are permanently stored
    if (!tmpurl.path().endsWith('/')) // make a uniform trailing slash policy
        tmpurl.setPath(tmpurl.path() + '/');
    UrlNodeP pnode;

    decreaseRanks();
    if (!head) { // if the list is empty ... (assumes dict to be empty as well)
        pnode = new UrlNode;
        pnode->rank = STARTING_RANK;
        pnode->url = tmpurl;
        appendNode(pnode);
        ranks.insert(tmpurl.url(), head);
    } else {
        if (ranks.find(tmpurl.url()) == ranks.end()) { // is the added url new? if so, append it
            pnode = new UrlNode;
            pnode->rank = STARTING_RANK;
            pnode->url = tmpurl;
            appendNode(pnode);
            ranks.insert(tmpurl.url(), pnode);
        } else {
            pnode = ranks[tmpurl.url()];
            pnode->rank += INCREASE;
        }
    }

    // do we need to change location for this one?
    relocateIfNeeded(pnode);

    // too many urls?
    if (count > maxUrls)
        removeNode(tail);

    // dumpList();
}

// checks if 'node' needs to be bumped-up the ranking list and does it if needed
void PopularUrls::relocateIfNeeded(UrlNodeP node)
{
    if (node->prev && (node->prev->rank < node->rank)) {
        // iterate until we find the correct place to put it
        UrlNodeP tmp = node->prev->prev;
        while (tmp) {
            if (tmp->rank >= node->rank)
                break; // found it!
            else
                tmp = tmp->prev;
        }
        // now, if tmp isn't null, we need to move node to tmp->next
        // else move it to become head
        removeNode(node);
        insertNode(node, tmp);
    }
}

// iterate over the list, decreasing each url's rank
// this is very naive, but a 1..30 for loop is acceptable (i hope)
void PopularUrls::decreaseRanks()
{
    if (head) {
        UrlNodeP p = head;
        while (p) {
            if (p->rank - DECREASE >= 0)
                p->rank -= DECREASE;
            else
                p->rank = 0;
            p = p->next;
        }
    }
}

// removes a node from the list, but doesn't free memory!
// note: this will be buggy in case the list becomes empty (which should never happen)
void PopularUrls::removeNode(UrlNodeP node)
{
    if (node->prev) {
        if (tail == node)
            tail = node->prev;
        node->prev->next = node->next;
    }
    if (node->next) {
        if (head == node)
            head = node->next;
        node->next->prev = node->prev;
    }
    --count;
}

void PopularUrls::insertNode(UrlNodeP node, UrlNodeP after)
{
    if (!after) { // make node head
        node->next = head;
        node->prev = nullptr;
        head->prev = node;
        head = node;
    } else {
        if (tail == after)
            tail = node;
        node->prev = after;
        node->next = after->next;
        if (node->next) {
            after->next->prev = node;
            after->next = node;
        }
    }
    ++count;
}

// appends 'node' to the end of the list, collecting garbage if needed
void PopularUrls::appendNode(UrlNodeP node)
{
    if (!tail) { // creating the first element
        head = tail = node;
        node->prev = node->next = nullptr;
    } else {
        node->next = nullptr;
        node->prev = tail;
        tail->next = node;
        tail = node;
    }
    ++count;
}

void PopularUrls::dumpList()
{
    UrlNodeP p = head;
    printf("====start %d====\n", count);
    while (p) {
        printf("%d : %s\n", p->rank, p->url.url().toLatin1().data());
        p = p->next;
    }
    fflush(stdout);
}

void PopularUrls::showDialog()
{
    QList<QUrl> list = getMostPopularUrls(maxUrls);
    dlg->run(list);
    if (dlg->result() == -1)
        return;
    SLOTS->refresh(list[dlg->result()]);
    // printf("running %s\n", list[dlg->result()].url().toLatin1());fflush(stdout);
}

// ===================================== PopularUrlsDlg ======================================
PopularUrlsDlg::PopularUrlsDlg()
    : QDialog(krMainWindow)
{
    setWindowTitle(i18n("Popular URLs"));
    setWindowModality(Qt::WindowModal);

    auto *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    auto *layout = new QGridLayout;
    layout->setContentsMargins(0, 0, 0, 0);

    // listview to contain the urls
    urls = new KrTreeWidget(this);
    urls->header()->hide();
    urls->setSortingEnabled(false);
    urls->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    // quick search
    search = new KTreeWidgetSearchLine(this, urls);
    QLabel *lbl = new QLabel(i18n("&Search:"), this);
    lbl->setBuddy(search);

    layout->addWidget(lbl, 0, 0);
    layout->addWidget(search, 0, 1);
    layout->addWidget(urls, 1, 0, 1, 2);

    mainLayout->addLayout(layout);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    mainLayout->addWidget(buttonBox);

    setTabOrder(search, urls);
    setTabOrder((QWidget *)urls, buttonBox->button(QDialogButtonBox::Close));

    connect(buttonBox, &QDialogButtonBox::rejected, this, &PopularUrlsDlg::reject);
    connect(urls, &KrTreeWidget::activated, this, &PopularUrlsDlg::slotItemSelected);
    connect(search, &KTreeWidgetSearchLine::hiddenChanged, this, &PopularUrlsDlg::slotVisibilityChanged);
}

void PopularUrlsDlg::slotItemSelected(const QModelIndex &ndx)
{
    selection = ndx.row();
    accept();
}

void PopularUrlsDlg::slotVisibilityChanged()
{
    // select the first visible item
    QList<QTreeWidgetItem *> list = urls->selectedItems();
    if (list.count() > 0 && !list[0]->isHidden())
        return;

    urls->clearSelection();
    urls->setCurrentItem(nullptr);

    QTreeWidgetItemIterator it(urls);
    while (*it) {
        if (!(*it)->isHidden()) {
            (*it)->setSelected(true);
            urls->setCurrentItem(*it);
            break;
        }
        it++;
    }
}

PopularUrlsDlg::~PopularUrlsDlg()
{
    delete search;
    delete urls;
}

void PopularUrlsDlg::run(QList<QUrl> list)
{
    // populate the listview
    urls->clear();
    QList<QUrl>::Iterator it;

    QTreeWidgetItem *lastItem = nullptr;

    for (it = list.begin(); it != list.end(); ++it) {
        auto *item = new QTreeWidgetItem(urls, lastItem);
        lastItem = item;
        item->setText(0, (*it).isLocalFile() ? (*it).path() : (*it).toDisplayString());
        item->setIcon(0, (*it).isLocalFile() ? Icon("folder") : Icon("folder-html"));
    }

    setMinimumSize(urls->sizeHint().width() + 45, 400);

    search->clear();
    search->setFocus();
    selection = -1;
    slotVisibilityChanged();
    exec();
}
