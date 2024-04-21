/*
    SPDX-FileCopyrightText: 2009 Jan Lepper <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2009-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRPREVIEWJOB_H
#define KRPREVIEWJOB_H

// QtCore
#include <QHash>
#include <QTimer>
#include <QVector>
// QtGui
#include <QPixmap>

#include <KFileItem>
#include <KIO/PreviewJob>

class KrViewItem;
class KrPreviews;

class KrPreviewJob : public KJob
{
    friend class KrPreviews;
    Q_OBJECT
public:
    void start() override
    {
    }

protected slots:
    void slotStartJob();
    void slotJobResult(KJob *job);
    void slotGotPreview(const KFileItem &item, const QPixmap &preview);
    void slotFailed(const KFileItem &item);

protected:
    QList<KrViewItem *> _scheduled;
    QHash<const KFileItem, KrViewItem *> _hash;
    KIO::PreviewJob *_job;
    QTimer _timer;
    KrPreviews *_parent;

    explicit KrPreviewJob(KrPreviews *parent);
    ~KrPreviewJob() override;
    void scheduleItem(KrViewItem *item);
    void removeItem(KrViewItem *item);

    void sort();
    bool doKill() override;
};

#endif // __krpreviewjob__
