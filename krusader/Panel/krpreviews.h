/***************************************************************************
                   krpreviews.h
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

#ifndef KRPREVIEWS_H
#define KRPREVIEWS_H

#include <QPixmap>
#include <QColor>
#include <QList>
#include <QHash>

#include <kurl.h>

class KJob;
class KrView;
class KrViewItem;
class KrPreviewJob;
class vfile;

class KrPreviews: public QObject
{
friend class KrPreviewJob;
    Q_OBJECT
public:
    KrPreviews(KrView *view);
    ~KrPreviews();

    bool getPreview(const vfile* file, QPixmap &pixmap, bool active);
    void updatePreview(KrViewItem *item);
    void deletePreview(KrViewItem *item);
    //updates all items for which no preview has been loaded yet
    void update();
    void clear();

protected slots:
    void slotRefreshColors();
    void slotJobResult(KJob *job);

protected:
    void addPreview(const vfile *file, const QPixmap &preview);
    void removePreview(const vfile* file);

    KrPreviewJob *_job;
    bool _dim;
    QColor _dimColor;
    int _dimFactor;
    QHash<const vfile*, QPixmap> _previews;
    QHash<const vfile*, QPixmap> _previewsInactive;
    KrView *_view;
};

#endif // __krpreviews__
