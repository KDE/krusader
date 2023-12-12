/*
    SPDX-FileCopyrightText: 2009 Jan Lepper <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2009-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krpreviews.h"
#include "krpreviewjob.h"

#include "../FileSystem/fileitem.h"
#include "../defaults.h"
#include "PanelView/krview.h"
#include "PanelView/krviewitem.h"
#include "krcolorcache.h"

#include <stdio.h>

#define ASSERT(what)                                                                                                                                           \
    if (!(what))                                                                                                                                               \
        abort();

KrPreviews::KrPreviews(KrView *view)
    : _job(nullptr)
    , _view(view)
{
    _dim = KrColorCache::getColorCache().getDimSettings(_dimColor, _dimFactor);
    connect(&KrColorCache::getColorCache(), &KrColorCache::colorsRefreshed, this, &KrPreviews::slotRefreshColors);
}

KrPreviews::~KrPreviews()
{
    clear();
}

void KrPreviews::clear()
{
    if (_job) {
        _job->kill(KJob::EmitResult);
        _job = nullptr;
    }
    _previews.clear();
    _previewsInactive.clear();
}

void KrPreviews::update()
{
    if (_job)
        return;
    for (KrViewItem *it = _view->getFirst(); it != nullptr; it = _view->getNext(it)) {
        if (!_previews.contains(it->getFileItem()))
            updatePreview(it);
    }
}

void KrPreviews::deletePreview(KrViewItem *item)
{
    if (_job) {
        _job->removeItem(item);
    }
    removePreview(item->getFileItem());
}

void KrPreviews::updatePreview(KrViewItem *item)
{
    if (!_job) {
        _job = new KrPreviewJob(this);
        connect(_job, &KrPreviewJob::result, this, &KrPreviews::slotJobResult);
        _view->op()->emitPreviewJobStarted(_job);
    }
    _job->scheduleItem(item);
}

bool KrPreviews::getPreview(const FileItem *file, QPixmap &pixmap, bool active)
{
    if (active || !_dim)
        pixmap = _previews.value(file);
    else
        pixmap = _previewsInactive.value(file);

    return !pixmap.isNull();
}

void KrPreviews::slotJobResult(KJob *job)
{
    (void)job;
    _job = nullptr;
}

void KrPreviews::slotRefreshColors()
{
    clear();
    _dim = KrColorCache::getColorCache().getDimSettings(_dimColor, _dimFactor);
    update();
}

void KrPreviews::addPreview(const FileItem *file, const QPixmap &preview)
{
    QPixmap active, inactive;

    if (preview.isNull()) {
        active = KrView::getIcon(const_cast<FileItem *>(file), true, _view->fileIconSize());
        if (_dim)
            inactive = KrView::getIcon(const_cast<FileItem *>(file), false, _view->fileIconSize());
    } else {
        active = KrView::processIcon(preview, false, _dimColor, _dimFactor, file->isSymLink());
        if (_dim)
            inactive = KrView::processIcon(preview, true, _dimColor, _dimFactor, file->isSymLink());
    }

    _previews.insert(file, active);
    if (_dim)
        _previewsInactive.insert(file, inactive);
}

void KrPreviews::removePreview(const FileItem *file)
{
    _previews.remove(file);
    _previewsInactive.remove(file);
}
