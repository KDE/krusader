#include <kmessagebox.h>
#include <klocale.h>
#include "../krusader.h"
#include "popularurls.h"

PopularUrls::PopularUrls(QObject *parent, const char *name) : QObject(parent, name), 
	head(0), tail(0), count(0) {
}

PopularUrls::~PopularUrls() {
	clearList();
}

void PopularUrls::clearList() {
	if (head) {
		UrlNodeP p=head, tmp;
		while (p) {
			tmp = p;
			p=p->next;
			delete tmp;
		}
	}
	ranks.clear();
	head = tail = 0;
}

void PopularUrls::save() {
	KConfigGroupSaver svr(krConfig, "Private");
	// prepare the string list containing urls and int list with ranks
	QStringList urlList;
	QValueList<int> rankList;
	UrlNodeP p = head;
	while (p) {
		urlList << p->url.url();
		rankList << p->rank;
		p = p->next;
	}
	krConfig->writeEntry("PopularUrls", urlList);
	krConfig->writeEntry("PopularUrlsRank", rankList);
}

void PopularUrls::load() {
	KConfigGroupSaver svr(krConfig, "Private");
	QStringList urlList = krConfig->readListEntry("PopularUrls");
	QValueList<int> rankList = krConfig->readIntListEntry("PopularUrlsRank");
	if (urlList.count() != rankList.count()) {
		KMessageBox::error(krApp, i18n("Saved 'Popular Urls' are invalid. List will be cleared"));
		return;
	}
	clearList();
	// iterate through both lists and
	QStringList::Iterator uit;
	QValueList<int>::Iterator rit;
	for (uit=urlList.begin(), rit=rankList.begin(); uit!=urlList.end() && rit!=rankList.end(); ++uit, ++rit) {
		UrlNodeP node = new UrlNode;
		node->url = *uit;
		node->rank = *rit;
		appendNode(node);
		ranks.insert(*uit, node);
	}
}


// returns a url list with the 'max' top popular urls 
KURL::List PopularUrls::getMostPopularUrls(int max) {
	// get at most 'max' urls
	KURL::List list;
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
void PopularUrls::addUrl(const KURL& url) {
	KURL tmpurl = url;
	tmpurl.adjustPath(1); // make a uniform trailing slash policy
	UrlNodeP pnode;
	
	if (!head) { // if the list is empty ... (assumes dict to be empty as well)
		pnode = new UrlNode;
		pnode->rank = 1;
		pnode->url = tmpurl;
		appendNode(pnode);
		ranks.insert(tmpurl.url(), head);
	} else {
		pnode = ranks.find(tmpurl.url());
		if (!pnode) { // is the added url new? if so, append it
			pnode = new UrlNode;
			pnode->rank = 1;
			pnode->url = tmpurl;
			appendNode(pnode);
			ranks.insert(tmpurl.url(), pnode);
		} else {
			pnode->rank++;
			// do we need to change location for this one?
			relocateIfNeeded(pnode);
		}
	}
	//dumpList();
}

// once we have 'hardLimit' urls, remove the bottom urls
// until we have only 'maxUrls' urls left
void PopularUrls::collectGarbage() {
	UrlNodeP n;
	while (count > maxUrls) {
		n = tail;
		removeNode(n);
		delete n;
		--count;
	}
}

// checks if 'node' needs to be bumped-up the ranking list and does it if needed
void PopularUrls::relocateIfNeeded(UrlNodeP node) {
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

// removes a node from the list, but doesn't free memory!
// note: this will be buggy in case the list becomes empty (which should never happen)
void PopularUrls::removeNode(UrlNodeP node) {
	if (node->prev) {
		if (tail == node) tail = node->prev;
		node->prev->next = node->next;
	}
	if (node->next) {
		if (head == node) head = node->next;
		node->next->prev = node->prev;
	}
}

void PopularUrls::insertNode(UrlNodeP node, UrlNodeP after) {
	if (!after) { // make node head
		node->next = head;
		node->prev = 0;
		head->prev = node;
		head = node;
	} else {
		if (tail == after) tail = node;
		node->prev = after;
		node->next = after->next;
		after->next->prev = node;
		after->next = node;
	}
}

// appends 'node' to the end of the list, collecting garbage if needed
void PopularUrls::appendNode(UrlNodeP node) {
	if (!tail) { // creating the first element
		head = tail = node;
		node->prev = node->next = 0;
	} else {
		node->next = 0;
		node->prev = tail;
		tail->next = node;
		tail = node;
	}
	if (++count == hardLimit) collectGarbage();
}

void PopularUrls::dumpList() {
	UrlNodeP p = head;
	printf("====start====\n");
	while (p) {
		printf("%d : %s\n", p->rank, p->url.url().latin1());
		p = p->next;
	}
	fflush(stdout);
}

#include "popularurls.moc"
