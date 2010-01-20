/***************************************************************************
                            krpreviews.cpp
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

                                                 S o u r c e    F i l e

***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "krpreviews.h"
#include "krpreviewjob.h"

#include "krcolorcache.h"
#include "krview.h"
#include "krviewitem.h"
#include "../VFS/vfile.h"
#include "../defaults.h"

#include <stdio.h>

#define ASSERT(what) if(!what) abort();


KrPreviews::KrPreviews(KrView *view) :  _view(view), _job(0)
{
    _dim = KrColorCache::getColorCache().getDimSettings(_dimColor, _dimFactor);
    connect(&KrColorCache::getColorCache(), SIGNAL(colorsRefreshed()), SLOT(slotRefreshColors()));
}

KrPreviews::~KrPreviews()
{
    clear();
}

void KrPreviews::clear()
{
    if(_job) {
        _job->kill(KJob::EmitResult);
        _job = 0;
    }
    _previews.clear();
    _previewsInactive.clear();
}

void KrPreviews::update()
{
    if(_job)
        return;
    for (KrViewItem *it = _view->getFirst(); it != 0; it = _view->getNext(it)) {
        if(!_previews.contains(it->getVfile()))
            updatePreview(it);
    }
}

void KrPreviews::deletePreview(KrViewItem *item)
{
    if(_job) {
        _job->removeItem(item);
    }
    removePreview(item->getVfile());
}

void KrPreviews::updatePreview(KrViewItem *item)
{
    if(!_job) {
        _job = new KrPreviewJob(this);
        connect(_job, SIGNAL(result(KJob*)), SLOT(slotJobResult(KJob*)));
        _view->op()->emitPreviewJobStarted(_job);
    }
    _job->scheduleItem(item);
}

bool KrPreviews::getPreview(const vfile* file, QPixmap &pixmap, bool active)
{
    if(active || !_dim)
        pixmap = _previews.value(file);
    else
        pixmap = _previewsInactive.value(file);

    return !pixmap.isNull();
}

void KrPreviews::slotJobResult(KJob *job)
{
    (void) job;
    _job = 0;
}

void KrPreviews::slotRefreshColors()
{
    clear();
    _dim = KrColorCache::getColorCache().getDimSettings(_dimColor, _dimFactor);
    update();
}

void KrPreviews::addPreview(const vfile *file, const QPixmap &preview)
{
    QPixmap active, inactive;

    if(preview.isNull()) {
        active = KrView::getIcon((vfile*)file, true, _view->fileIconSize());
        if(_dim)
            inactive = KrView::getIcon((vfile*)file, false, _view->fileIconSize());
    } else {
        active = KrView::processIcon(preview, false, _dimColor, _dimFactor, file->vfile_isSymLink());
        if(_dim)
            inactive = KrView::processIcon(preview, true, _dimColor, _dimFactor, file->vfile_isSymLink());
    }

    _previews.insert(file, active);
    if(_dim)
        _previewsInactive.insert(file, inactive);
}

void KrPreviews::removePreview(const vfile* file)
{
    _previews.remove(file);
    _previewsInactive.remove(file);
}
