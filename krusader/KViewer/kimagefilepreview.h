/*
*
* This file is part of the KDE project.
* Copyright (C) 2001 Martin R. Jones <mjones@kde.org>
*               2001 Carsten Pfeiffer <pfeiffer@kde.org>
*
* Modified for Krusader by Shie Erlich, October 2004
*
* You can Freely distribute this program under the GNU Library General Public
* License. See the file "COPYING" for the exact licensing terms.
*/

#ifndef KrusaderImageFilePreview_H
#define KrusaderImageFilePreview_H

#include <qpixmap.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <QLabel>

#include <kurl.h>
#include <kpreviewwidgetbase.h>

class QCheckBox;
class QPushButton;
class QLabel;
class QTimer;

class KFileDialog;
class KFileItem;

class KrusaderImageFilePreview : public KPreviewWidgetBase {
		Q_OBJECT

	public:
		KrusaderImageFilePreview( QWidget *parent );
		~KrusaderImageFilePreview();

		virtual QSize sizeHint() const;

	public slots:
		virtual void showPreview( const KURL &url );
		virtual void clearPreview();

	protected slots:
		void showPreview();
		void showPreview( const KURL& url, bool force );

		virtual void gotPreview( const KFileItem*, const QPixmap& );

	protected:
		virtual void resizeEvent( QResizeEvent *e );
		virtual KIO::PreviewJob * createJob( const KURL& url,
		                                     int w, int h );

	private slots:
		void slotResult( KIO::Job * );
		virtual void slotFailed( const KFileItem* );

	private:
		KURL currentURL;
		QTimer *timer;
		QLabel *imageLabel;
		QLabel *infoLabel;
		KIO::PreviewJob *m_job;
	protected:
		virtual void virtual_hook( int id, void* data );
	private:
		class KrusaderImageFilePreviewPrivate;
		KrusaderImageFilePreviewPrivate *d;
};

#endif // KrusaderImageFilePreview_H
