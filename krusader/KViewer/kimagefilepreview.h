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

// QtCore
#include <QUrl>
// QtGui
#include <QPixmap>
#include <QResizeEvent>
// QtWidgets
#include <QLabel>

# include <KIOFileWidgets/KPreviewWidgetBase>
#include <KIO/PreviewJob>

class QLabel;
class QTimer;

class KFileItem;

class KrusaderImageFilePreview : public KPreviewWidgetBase
{
    Q_OBJECT

public:
    KrusaderImageFilePreview(QWidget *parent);
    ~KrusaderImageFilePreview();

    virtual QSize sizeHint() const Q_DECL_OVERRIDE;

public slots:
    virtual void showPreview(const QUrl&) Q_DECL_OVERRIDE;
    virtual void clearPreview() Q_DECL_OVERRIDE;

protected slots:
    void showPreview();
    void showPreview(const QUrl &url, bool force);

    virtual void gotPreview(const KFileItem&, const QPixmap&);

protected:
    virtual void resizeEvent(QResizeEvent *e) Q_DECL_OVERRIDE;
    virtual KIO::PreviewJob * createJob(const QUrl &url,
                                        int w, int h);

private slots:
    void slotResult(KJob *);
    virtual void slotFailed(const KFileItem&);

private:
    QUrl currentURL;
    QTimer *timer;
    QLabel *imageLabel;
    QLabel *infoLabel;
    KIO::PreviewJob *m_job;
private:
    class KrusaderImageFilePreviewPrivate;
    KrusaderImageFilePreviewPrivate *d;
};

#endif // KrusaderImageFilePreview_H
