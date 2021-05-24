/*
    SPDX-FileCopyrightText: 2009 Jan Lepper <krusader@users.sourceforge.net>
    SPDX-FileCopyrightText: 2009-2020 Krusader Krew [https://krusader.org]

    This file is part of Krusader [https://krusader.org].

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRPREVIEWS_H
#define KRPREVIEWS_H

// QtCore
#include <QList>
#include <QHash>
// QtGui
#include <QPixmap>
#include <QColor>

class KJob;
class KrView;
class KrViewItem;
class KrPreviewJob;
class FileItem;

class KrPreviews: public QObject
{
friend class KrPreviewJob;
    Q_OBJECT
public:
    explicit KrPreviews(KrView *view);
    ~KrPreviews() override;

    bool getPreview(const FileItem* file, QPixmap &pixmap, bool active);
    void updatePreview(KrViewItem *item);
    void deletePreview(KrViewItem *item);
    //updates all items for which no preview has been loaded yet
    void update();
    void clear();

protected slots:
    void slotRefreshColors();
    void slotJobResult(KJob *job);

protected:
    void addPreview(const FileItem *file, const QPixmap &preview);
    void removePreview(const FileItem* file);

    KrPreviewJob *_job;
    bool _dim;
    QColor _dimColor;
    int _dimFactor;
    QHash<const FileItem*, QPixmap> _previews;
    QHash<const FileItem*, QPixmap> _previewsInactive;
    KrView *_view;
};

#endif // __krpreviews__
