#ifndef POPULARURLS_H
#define POPULARURLS_H

#include <qobject.h>
#include <kurl.h>
#include <qdict.h>

// the class holds a list of most popular links in a dual data structure
// * linked list, with head and tail: for fast append/prepend support
// * dictionary that maps urls to list nodes: to save the need to iterate
//   over the list in order to find the correct node for each new url
typedef struct _UrlNode* UrlNodeP;
typedef struct _UrlNode {
	UrlNodeP prev;
	KURL url;
	int rank;
	UrlNodeP next;
} UrlNode;

class PopularUrls : public QObject {
Q_OBJECT
public:
	PopularUrls(QObject *parent = 0, const char *name = 0);
	~PopularUrls();
	
	void addUrl(const KURL& url);
	KURL::List getMostPopularUrls(int max);

protected:
	void appendNode(UrlNodeP node);
	void insertNode(UrlNodeP node, UrlNodeP after);
	void removeNode(UrlNodeP node);
	void relocateIfNeeded(UrlNodeP node);
	void dumpList();
	
private:
	UrlNodeP head, tail;
	QDict<UrlNode> ranks; // actually holds UrlNode*
	int count;
};

#endif
