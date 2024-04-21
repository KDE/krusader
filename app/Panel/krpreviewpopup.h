/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KRPREVIEWPOPUP_H
#define KRPREVIEWPOPUP_H

// QtCore
#include <QUrl>
// QtGui
#include <QPixmap>
// QtWidgets
#include <QMenu>

#include <KFileItem>

class KrPreviewPopup : public QMenu
{
    Q_OBJECT

public:
    KrPreviewPopup();

    void setUrls(const QList<QUrl> &urls);
public slots:
    void addPreview(const KFileItem &file, const QPixmap &preview);
    void view(QAction *);

protected:
    void showEvent(QShowEvent *event) override;

    QAction *prevNotAvailAction;
    QList<KFileItem> files;
    bool jobStarted;

private:
    static const int MAX_SIZE = 400;
    static const short MARGIN = 5;

    class ProxyStyle;
};

#endif
