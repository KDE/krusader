/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
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

#include "popularurls.h"

#include <QtGui/QPushButton>
#include <QGridLayout>
#include <QtCore/QList>
#include <qheaderview.h>
#include <QtGui/QLayout>
#include <QtGui/QLabel>

#include <kmessagebox.h>
#include <klocale.h>
#include <kiconloader.h>
#include <ktreewidgetsearchline.h>

#include "../krglobal.h"
#include "../krslots.h"
#include "../GUI/krtreewidget.h"

#define STARTING_RANK 20
#define INCREASE   2
#define DECREASE   1

PopularUrls::PopularUrls(QObject *parent) : QObject(parent),
        head(0), tail(0), count(0)
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
    head = tail = 0;
}

void PopularUrls::save()
{
    KConfigGroup svr(krConfig, "Private");
    // prepare the string list containing urls and int list with ranks
    QStringList urlList;
    QList<int> rankList;
    UrlNodeP p = head;
    while (p) {
        urlList << p->url.prettyUrl();
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
        KMessageBox::error(krMainWindow, i18n("Saved 'Popular Urls' are invalid. List will be cleared"));
        return;
    }
    clearList();
    count = 0;
    // iterate through both lists and
    QStringList::Iterator uit;
    QList<int>::Iterator rit;
    for (uit = urlList.begin(), rit = rankList.begin(); uit != urlList.end() && rit != rankList.end(); ++uit, ++rit) {
        UrlNodeP node = new UrlNode;
        node->url = KUrl(*uit);
        node->rank = *rit;
        appendNode(node);
        ranks.insert(*uit, node);
    }
}


// returns a url list with the 'max' top popular urls
KUrl::List PopularUrls::getMostPopularUrls(int max)
{
    // get at most 'max' urls
    KUrl::List list;
    UrlNodeP p = head;
    int tmp = 0;
    if (maxUrls < max) max = maxUrls; // don't give more than maxUrls
    while (p && tmp < max) {
        list << p->url;
        p = p->next;
        ++tmp;
    }

    return list;
}

// adds a url to the list, or increase rank of an existing url, making
// sure to bump it up the list if needed
void PopularUrls::addUrl(const KUrl& url)
{
    KUrl tmpurl = url;
    tmpurl.adjustPath(KUrl::AddTrailingSlash); // make a uniform trailing slash policy
    UrlNodeP pnode;

    decreaseRanks();
    if (!head) { // if the list is empty ... (assumes dict to be empty as well)
        pnode = new UrlNode;
        pnode->rank = STARTING_RANK;
        pnode->url = tmpurl;
        appendNode(pnode);
        ranks.insert(tmpurl.url(), head);
    } else {
        if (ranks.find(tmpurl.url()) == ranks.end()) {  // is the added url new? if so, append it
            pnode = new UrlNode;
            pnode->rank = STARTING_RANK;
            pnode->url = tmpurl;
            appendNode(pnode);
            ranks.insert(tmpurl.url(), pnode);
        } else {
            pnode = ranks[ tmpurl.url()];
            pnode->rank += INCREASE;
        }
    }

    // do we need to change location for this one?
    relocateIfNeeded(pnode);

    // too many urls?
    if (count > maxUrls) removeNode(tail);

    //dumpList();
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
            else tmp = tmp->prev;
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
            else p->rank = 0;
            p = p->next;
        }
    }
}

// removes a node from the list, but doesn't free memory!
// note: this will be buggy in case the list becomes empty (which should never happen)
void PopularUrls::removeNode(UrlNodeP node)
{
    if (node->prev) {
        if (tail == node) tail = node->prev;
        node->prev->next = node->next;
    }
    if (node->next) {
        if (head == node) head = node->next;
        node->next->prev = node->prev;
    }
    --count;
}

void PopularUrls::insertNode(UrlNodeP node, UrlNodeP after)
{
    if (!after) { // make node head
        node->next = head;
        node->prev = 0;
        head->prev = node;
        head = node;
    } else {
        if (tail == after) tail = node;
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
        node->prev = node->next = 0;
    } else {
        node->next = 0;
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
    KUrl::List list = getMostPopularUrls(maxUrls);
    dlg->run(list);
    if (dlg->result() == -1) return;
    SLOTS->refresh(list[dlg->result()]);
    //printf("running %s\n", list[dlg->result()].url().toLatin1());fflush(stdout);
}

// ===================================== PopularUrlsDlg ======================================
PopularUrlsDlg::PopularUrlsDlg():
        KDialog(krMainWindow)
{
    setButtons(KDialog::Close);
    setDefaultButton(KDialog::NoDefault);
    setWindowTitle(i18n("Popular Urls"));
    setWindowModality(Qt::WindowModal);

    QWidget * widget = new QWidget(this);
    QGridLayout *layout = new QGridLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    // listview to contain the urls
    urls = new KrTreeWidget(widget);
    urls->setHeaderLabel("");
    urls->header()->hide();
    urls->setSortingEnabled(false);
    urls->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    // quick search
    QToolButton *btn = new QToolButton(widget);
    btn->setIcon(SmallIcon("edit-clear-locationbar-ltr"));
    search = new KTreeWidgetSearchLine(widget, urls);
    search->setTrapReturnKey(true);
    QLabel *lbl = new QLabel(i18n(" &Search: "), widget);
    lbl->setBuddy(search);

    layout->addWidget(btn, 0, 0);
    layout->addWidget(lbl, 0, 1);
    layout->addWidget(search, 0, 2);
    layout->addWidget(urls, 1, 0, 1, 3);
    setMaximumSize(600, 500);

    setMainWidget(widget);

    setTabOrder(search, urls);
    setTabOrder((QWidget *)urls, (QWidget *)button(KDialog::Close));

    connect(urls, SIGNAL(activated(const QModelIndex &)),
            this, SLOT(slotItemSelected(const QModelIndex &)));
    connect(btn, SIGNAL(clicked()), search, SLOT(clear()));
    connect(search, SIGNAL(hiddenChanged(QTreeWidgetItem *, bool)),
            this, SLOT(slotVisibilityChanged()));
}

void PopularUrlsDlg::slotItemSelected(const QModelIndex & ndx)
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
    urls->setCurrentItem(0);

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

void PopularUrlsDlg::run(KUrl::List list)
{
    // populate the listview
    urls->clear();
    KUrl::List::Iterator it;

    QTreeWidgetItem * lastItem = 0;

    for (it = list.begin(); it != list.end(); ++it) {
        QTreeWidgetItem *item = new QTreeWidgetItem(urls, lastItem);
        lastItem = item;
        item->setText(0, (*it).isLocalFile() ? (*it).path() : (*it).prettyUrl());
        item->setIcon(0, (*it).isLocalFile() ? SmallIcon("folder") : SmallIcon("folder-html"));
    }

    setMinimumSize(urls->sizeHint().width() + 45, 400);

    search->clear();
    search->setFocus();
    selection = -1;
    slotVisibilityChanged();
    exec();
}

#include "popularurls.moc"
