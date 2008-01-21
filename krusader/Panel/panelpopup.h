#ifndef _PANELPOPUP_H
#define _PANELPOPUP_H

#include <qwidget.h>
#include <qstackedwidget.h>
#include <qpixmap.h>
//Added by qt3to4:
#include <QDropEvent>
#include <QLabel>
#include <kfileitem.h>
#include <qpointer.h>
#include <kio/previewjob.h>
#include <kurl.h>

class Q3ButtonGroup;
class QLabel;
class Q3ListViewItem;
class QSplitter;
class K3FileTreeView;
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
	inline int currentPage() const { return stack->currentWidget()->property( "KrusaderWidgetId" ).toInt(); }

	void saveSizes();

public slots:
   void update(KUrl url);
	void show();
	void hide();
	
signals:
	void selection(const KUrl &url);
	void hideMe();
   
protected slots:	
	virtual void setFocus();
	void tabSelected(int id);
	void treeSelection(Q3ListViewItem*);
	void slotDroppedOnTree(QWidget *widget, QDropEvent *e, KUrl::List &lst, KUrl &);
	void handleOpenUrlRequest(const KUrl &url);
	void quickSelect();
	void quickSelect(const QString &);
        void quickSelectStore();

protected:
	bool _left;
	bool _hidden;
	QStackedWidget *stack;
	KrusaderImageFilePreview *viewer;
	KrSqueezedTextLabel *dataLine;
	QPointer<KIO::PreviewJob> pjob;
	K3FileTreeView *tree;
	QToolButton *treeBtn, *previewBtn, *quickBtn, *viewerBtn, *duBtn;
	Q3ButtonGroup *btns;
	KLineEdit *quickFilter;
	KComboBox *quickSelectCombo;
	PanelViewer *panelviewer;
	DiskUsageViewer *diskusage;
	QWidget *quickPanel;
	QList<int> splitterSizes;
	QSplitter *splitter;
};

#endif // _PANELPOPUP_H
