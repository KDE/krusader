#ifndef _PANELPOPUP_H
#define _PANELPOPUP_H

#include <qwidget.h>
#include <qstackedwidget.h>
#include <QtGui/QPixmap>
#include <QDropEvent>
#include <QLabel>
#include <kfileitem.h>
#include <QtCore/QPointer>
#include <kio/previewjob.h>
#include <kurl.h>
#include <qtreeview.h>

class QButtonGroup;
class QLabel;
class QSplitter;
class QToolButton;
class KrSqueezedTextLabel;
class KLineEdit;
class KComboBox;
class KrusaderImageFilePreview;
class PanelViewer;
class DiskUsageViewer;
class KrFileTreeView;
class KDirModel;
class KDirSortFilterProxyModel;
class QMimeData;
class QPoint;

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
	void treeSelection();
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
	KrFileTreeView *tree;
	QToolButton *treeBtn, *previewBtn, *quickBtn, *viewerBtn, *duBtn;
	QButtonGroup *btns;
	KLineEdit *quickFilter;
	KComboBox *quickSelectCombo;
	PanelViewer *panelviewer;
	DiskUsageViewer *diskusage;
	QWidget *quickPanel;
	QList<int> splitterSizes;
	QSplitter *splitter;
};


class KrFileTreeView : public QTreeView
{
    friend class KrDirModel;
    Q_OBJECT

    public:
        KrFileTreeView(QWidget *parent = 0);
        virtual ~KrFileTreeView() {}

        KUrl currentUrl() const;
        KUrl selectedUrl() const;
        KUrl::List selectedUrls() const;
        KUrl rootUrl() const;

    public Q_SLOTS:
        void setDirOnlyMode(bool enabled);
        void setShowHiddenFiles(bool enabled);
        void setCurrentUrl(const KUrl &url);
        void setRootUrl(const KUrl &url);

    Q_SIGNALS:
        void activated(const KUrl &url);
        void currentChanged(const KUrl &url);

private Q_SLOTS:
        void slotActivated(const QModelIndex&);
        void slotCurrentChanged(const QModelIndex&, const QModelIndex&);
        void slotExpanded(const QModelIndex&);

private:
        KUrl urlForProxyIndex(const QModelIndex &index) const;
        void dropMimeData ( const KUrl::List & lst, const KUrl & url, const QModelIndex & ind );

        KDirModel *mSourceModel;
        KDirSortFilterProxyModel *mProxyModel;
};

#endif // _PANELPOPUP_H
