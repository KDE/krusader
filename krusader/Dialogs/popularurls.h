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

#ifndef POPULARURLS_H
#define POPULARURLS_H

#include <QtCore/QObject>
#include <kurl.h>
#include <QtCore/QHash>
#include <kdialog.h>

// the class holds a list of most popular links in a dual data structure
// * linked list, with head and tail: for fast append/prepend support
// * dictionary that maps urls to list nodes: to save the need to iterate
//   over the list in order to find the correct node for each new url
//
// also, the class holds a maximum number of urls. two variables affect this:
// * maxUrls - the num. of urls the user can see
// * hardLimit - the actual number of urls kept.
// when the number of urls reaches hardLimit, a garbage collection is done and
// the bottom (hardLimit-maxUrls) entries are removed from the list
typedef struct _UrlNode* UrlNodeP;
typedef struct _UrlNode {
    UrlNodeP prev;
    KUrl url;
    int rank;
    UrlNodeP next;
} UrlNode;

class PopularUrlsDlg;

class PopularUrls : public QObject
{
    Q_OBJECT
public:
    PopularUrls(QObject *parent = 0);
    ~PopularUrls();
    void save();
    void load();
    void addUrl(const KUrl& url);
    KUrl::List getMostPopularUrls(int max);

public slots:
    void showDialog();

protected:
    // NOTE: the following methods append/insert/remove a node to the list
    // but NEVER free memory or allocate memory!
    void appendNode(UrlNodeP node);
    void insertNode(UrlNodeP node, UrlNodeP after);
    void removeNode(UrlNodeP node);
    void relocateIfNeeded(UrlNodeP node);
    void clearList();
    void dumpList();
    void decreaseRanks();

private:
    UrlNodeP head, tail;
    QHash<QString, UrlNode *> ranks; // actually holds UrlNode*
    int count;
    static const int maxUrls = 30;
    PopularUrlsDlg *dlg;
};

class KrTreeWidget;
class KTreeWidgetSearchLine;
class QModelIndex;
class QTreeWidgetItem;

class PopularUrlsDlg: public KDialog
{
    Q_OBJECT
public:
    PopularUrlsDlg();
    ~PopularUrlsDlg();
    void run(KUrl::List list); // use this to open the dialog
    inline int result() const {
        return selection;
    } // returns index 0 - topmost, or -1


protected slots:
    void slotVisibilityChanged();
    void slotItemSelected(const QModelIndex &);

private:
    KrTreeWidget *urls;
    KTreeWidgetSearchLine *search;
    int selection;
};

#endif
