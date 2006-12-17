#ifndef _PANELPOPUP_H
#define _PANELPOPUP_H

#include <qwidget.h>
#include <qwidgetstack.h>
#include <qpixmap.h>
#include <qvaluelist.h>
#include <kfileitem.h>
#include <qguardedptr.h>
#include <kio/previewjob.h>
#include <kurl.h>

class QButtonGroup;
class QLabel;
class QListViewItem;
class QSplitter;
class KFileTreeView;
class QToolButton;
class KrSqueezedTextLabel;
class KLineEdit;
class KComboBox;
class KrusaderImageFilePreview;
class PanelViewer;
class DiskUsageViewer;

class PanelPopup: public QWidget {
   Q_OBJECT
   enum Parts { Tree, Preview, QuickPanel, View, DskUsage, Last=0xFF };
public:
   PanelPopup( QSplitter *splitter, bool left );
   ~PanelPopup();
	inline int currentPage() const { return stack->id(stack->visibleWidget()); }

	void saveSizes();

public slots:
   void update(KURL url);
	void show();
	void hide();
	
signals:
	void selection(const KURL &url);
	void hideMe();
   
protected slots:	
	void tabSelected(int id);
	void treeSelection(QListViewItem*);
	void slotDroppedOnTree(QWidget *widget, QDropEvent *e, KURL::List &lst, KURL &);
	void handleOpenURLRequest(const KURL &url);
	void quickSelect();
	void quickSelect(const QString &);
        void quickSelectStore();

protected:
	bool _left;
	bool _hidden;
	QWidgetStack *stack;
	KrusaderImageFilePreview *viewer;
	KrSqueezedTextLabel *dataLine;
	QGuardedPtr<KIO::PreviewJob> pjob;
	KFileTreeView *tree;
	QToolButton *treeBtn, *previewBtn, *quickBtn, *viewerBtn, *duBtn;
	QButtonGroup *btns;
	KLineEdit *quickFilter;
	KComboBox *quickSelectCombo;
	PanelViewer *panelviewer;
	DiskUsageViewer *diskusage;
	QValueList<int> splitterSizes;
	QSplitter *splitter;
};

#endif // _PANELPOPUP_H
