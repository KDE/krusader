#ifndef _PANELPOPUP_H
#define _PANELPOPUP_H

#include <qwidget.h>
#include <qpixmap.h>
#include <kfileitem.h>
#include <qguardedptr.h>
#include <kio/previewjob.h>


class QLabel;
class QWidgetStack;
class QListViewItem;
class KMultiTabBar;
class KFileTreeView;

class PanelPopup: public QWidget {
   Q_OBJECT
   enum Parts { Tree, Preview, Last=0xFF };
public:
   PanelPopup( QWidget *parent );
   ~PanelPopup();

public slots:
   void update(KURL url);
	
signals:
	void selection(const KURL &url);
	void hideMe();
   
protected slots:	
	void view( const KFileItem* kfi, const QPixmap& pix );
	void failedToView(const KFileItem* kfi);
	void tabSelected(int id);
	void treeSelection(QListViewItem*);

protected:
   QWidgetStack *stack;
	QLabel *viewer;
	QGuardedPtr<KIO::PreviewJob> pjob;
	KMultiTabBar *tabbar;
	KFileTreeView *tree;
};

#endif // _PANELPOPUP_H
