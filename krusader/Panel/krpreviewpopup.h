/*****************************************************************************
 * Copyright (C) 2002 Shie Erlich <erlich@users.sourceforge.net>             *
 * Copyright (C) 2002 Rafi Yanai <yanai@users.sourceforge.net>               *
 * Copyright (C) 2004-2018 Krusader Krew [https://krusader.org]              *
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

#ifndef KRPREVIEWPOPUP_H
#define KRPREVIEWPOPUP_H

// QtCore
#include <QUrl>
// QtGui
#include <QPixmap>
// QtWidgets
#include <QMenu>

#include <KIOCore/KFileItem>

class KrPreviewPopup : public QMenu
{
    Q_OBJECT

public:
    KrPreviewPopup();

    void setUrls(const QList<QUrl> &urls);
public slots:
    void addPreview(const KFileItem& file, const QPixmap& preview);
    void view(QAction *);

protected:
    virtual void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

    QAction * prevNotAvailAction;
    QList<KFileItem> files;
    bool jobStarted;

private:
    static const int MAX_SIZE =400;
    static const short MARGIN =5;

    class ProxyStyle;
};

#endif
