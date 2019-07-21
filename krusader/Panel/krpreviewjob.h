/*****************************************************************************
 * Copyright (C) 2009 Jan Lepper <krusader@users.sourceforge.net>            *
 * Copyright (C) 2009-2019 Krusader Krew [https://krusader.org]              *
 *                                                                           *
 * This file is part of Krusader [https://krusader.org].                     *
 *                                                                           *
 * Krusader is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Krusader is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with Krusader.  If not, see [http://www.gnu.org/licenses/].         *
 *****************************************************************************/

#ifndef KRPREVIEWJOB_H
#define KRPREVIEWJOB_H

// QtCore
#include <QVector>
#include <QHash>
#include <QTimer>
// QtGui
#include <QPixmap>

#include <KIO/PreviewJob>
#include <KIOCore/KFileItem>

class KrViewItem;
class KrPreviews;

class KrPreviewJob : public KJob
{
friend class KrPreviews;
    Q_OBJECT
public:
    void start() Q_DECL_OVERRIDE {}

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

    explicit KrPreviewJob(KrPreviews *parent);
    ~KrPreviewJob() override;
    void scheduleItem(KrViewItem *item);
    void removeItem(KrViewItem *item);

    void sort();
    bool doKill() Q_DECL_OVERRIDE;
};

#endif // __krpreviewjob__
