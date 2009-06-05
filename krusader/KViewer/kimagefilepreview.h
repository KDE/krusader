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

#ifndef KIMAGEFILEPREVIEW_H
#define KIMAGEFILEPREVIEW_H

#include <QtGui/QPixmap>
#include <QResizeEvent>
#include <QLabel>

#include <kurl.h>
#include <kpreviewwidgetbase.h>
#include <kio/previewjob.h>

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
		virtual void showPreview( const KUrl &url );
		virtual void clearPreview();

	protected slots:
		void showPreview();
		void showPreview( const KUrl& url, bool force );

		virtual void gotPreview( const KFileItem&, const QPixmap& );

	protected:
		virtual void resizeEvent( QResizeEvent *e );
		virtual KIO::PreviewJob * createJob( const KUrl& url,
		                                     int w, int h );

	private slots:
		void slotResult( KJob * );
		virtual void slotFailed( const KFileItem& );

	private:
		KUrl currentURL;
		QTimer *timer;
		QLabel *imageLabel;
		QLabel *infoLabel;
		KIO::PreviewJob *m_job;
	private:
		class KrusaderImageFilePreviewPrivate;
		KrusaderImageFilePreviewPrivate *d;
};

#endif // KrusaderImageFilePreview_H
