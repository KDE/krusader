#ifndef _SUPERVIEW_H
#define _SUPERVIEW_H

#include <kurl.h>
#include <qstring.h>
#include <qwidgetstack.h>
#include <kparts/part.h>
#include <kio/job.h>
#include <ktempfile.h>
#include <qdict.h>
#include <qlabel.h>

#include "krviewer.h"


class PanelViewerBase: public QWidgetStack {
	Q_OBJECT

public:
	PanelViewerBase( QWidget *parent = 0 );
	virtual ~PanelViewerBase();
	inline KURL url() const { return curl; }
	inline KParts::ReadOnlyPart* part() const { return cpart; }
	virtual bool isModified() { return false; }

public slots:
	virtual KParts::ReadOnlyPart* openURL( const KURL&, KrViewer::Mode=KrViewer::Generic ){ return 0;} 
	virtual bool closeURL(){ return false; }
	virtual bool queryClose() { return true; }

signals:
	void openURLRequest( const KURL &url );

protected:
	QDict<KParts::ReadOnlyPart> *mimes;
	KParts::ReadOnlyPart *cpart;

	QString cmimetype;
	KURL curl;
	QLabel *fallback;

};

class PanelViewer: public PanelViewerBase {
	Q_OBJECT
public slots:
	KParts::ReadOnlyPart* openURL( const KURL &url, KrViewer::Mode mode=KrViewer::Generic );
	bool closeURL();

public:
	PanelViewer( QWidget *parent = 0 );
	~PanelViewer();

protected:
	KParts::ReadOnlyPart *getPart( QString mimetype );
	KParts::ReadOnlyPart*  getHexPart();
	void oldHexViewer(KTempFile& tmpFile);
};

class PanelEditor: public PanelViewerBase {
	Q_OBJECT
public:
	virtual bool isModified();

public slots:
	KParts::ReadOnlyPart* openURL( const KURL &url, KrViewer::Mode mode=KrViewer::Generic );
	bool closeURL();
	bool queryClose();
	void slotStatResult( KIO::Job* job );

public:
	PanelEditor( QWidget *parent = 0 );
	~PanelEditor();

protected:
	KParts::ReadWritePart* getPart( QString mimetype );

	bool busy;
	KIO::UDSEntry entry;
};

#endif
