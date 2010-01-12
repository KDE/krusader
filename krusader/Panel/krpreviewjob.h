/***************************************************************************
                   krpreviewjob.h
                 -------------------
copyright            : (C) 2009 by Jan Lepper
e-mail               : krusader@users.sourceforge.net
web site             : http://krusader.sourceforge.net
---------------------------------------------------------------------------
Description
***************************************************************************

A

db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                         H e a d e r    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef KRPREVIEWJOB_H
#define KRPREVIEWJOB_H

#include <QPixmap>
#include <QVector>
#include <QHash>
#include <QTimer>

#include <kio/previewjob.h>
#include <kfileitem.h>

#include "krpreviews.h"


class vfile;
class KrView;
class KrViewItem;
class KrPreviews;

class KrPreviewJob : public KJob
{
friend class KrPreviews;
    Q_OBJECT
public:
    virtual void start() {}

protected slots:
    void slotStartJob();
    void slotJobResult(KJob *job);
    void slotGotPreview(const KFileItem & item, const QPixmap & preview);
    void slotFailed(const KFileItem & item);

protected:
    QList<KrViewItem*> _scheduled;
    QHash<const KFileItem, KrViewItem*> _hash;
    KIO::PreviewJob *_job;
    QTimer _timer;
    KrPreviews *_parent;

    KrPreviewJob(KrPreviews *parent);
    ~KrPreviewJob();
    void scheduleItem(KrViewItem *item);
    void removeItem(KrViewItem *item);

    void sort();
    virtual bool doKill();
};

#endif // __krpreviewjob__
