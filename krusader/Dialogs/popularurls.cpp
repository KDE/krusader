#include "popularurls.h"

PopularUrls::PopularUrls(QObject *parent, const char *name) : QObject(parent, name), 
	head(0), tail(0), count(0) {
}

PopularUrls::~PopularUrls() {
}

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
}

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

void PopularUrls::removeNode(UrlNodeP node) {
	if (node->prev) {
		node->prev->next = node->next;
	}
	if (node->next) {
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
		node->prev = after;
		node->next = after->next;
		after->next->prev = node;
		after->next = node;
	}
}

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
	++count;
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
