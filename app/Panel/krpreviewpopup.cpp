/*
    SPDX-FileCopyrightText: 2002 Shie Erlich <erlich@users.sourceforge.net>
    SPDX-FileCopyrightText: 2002 Rafi Yanai <yanai@users.sourceforge.net>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "krpreviewpopup.h"

#include <algorithm>

// QtGui
#include <QPainter>
#include <QPixmap>
// QtWidgets
#include <QApplication>
#include <QProxyStyle>
#include <QStyleOptionMenuItem>

#include <KIO/PreviewJob>
#include <KLocalizedString>

#include "../KViewer/krviewer.h"

class KrPreviewPopup::ProxyStyle : public QProxyStyle
{
public:
    ProxyStyle()
        : QProxyStyle(QApplication::style())
    {
    }

    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget = nullptr) const override
    {
        if (type == QStyle::CT_MenuItem) {
            const auto *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option);

            QFontMetrics fontMetrics(menuItem->font);
            QSize iconSize = menuItem->icon.actualSize(QSize(MAX_SIZE, MAX_SIZE));
            QSize textSize = QSize(fontMetrics.boundingRect(menuItem->text).width(), fontMetrics.height());

            return QSize(std::max(iconSize.width(), textSize.width()) + MARGIN * 2, iconSize.height() + textSize.height() + MARGIN * 2);
        } else
            return QProxyStyle::sizeFromContents(type, option, contentsSize, widget);
    }

    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override
    {
        if (element == QStyle::CE_MenuItem) {
            painter->save();

            const auto *menuItem = qstyleoption_cast<const QStyleOptionMenuItem *>(option);

            bool active = menuItem->state & State_Selected;

            QRect rect = menuItem->rect;

            if (active)
                painter->fillRect(rect, menuItem->palette.brush(QPalette::Highlight));

            rect.adjust(MARGIN, MARGIN, -MARGIN, -MARGIN);

            int textHeight = QFontMetrics(menuItem->font).height();

            QRect previewRect = rect;
            previewRect.setHeight(rect.height() - textHeight);
            QPixmap pixmap = menuItem->icon.pixmap(menuItem->icon.actualSize(QSize(MAX_SIZE, MAX_SIZE)));
            QProxyStyle::drawItemPixmap(painter, previewRect, Qt::AlignCenter, pixmap);

            QRect textRect = rect;
            textRect.setTop(previewRect.bottom() + 1);
            painter->setPen(active ? menuItem->palette.highlightedText().color() : menuItem->palette.buttonText().color());
            int textFlags = Qt::TextShowMnemonic | Qt::TextDontClip | Qt::TextSingleLine | Qt::AlignCenter;
            painter->drawText(textRect, textFlags, menuItem->text);

            painter->restore();
        } else
            QProxyStyle::drawControl(element, option, painter, widget);
    }
};

KrPreviewPopup::KrPreviewPopup()
    : jobStarted(false)
{
    prevNotAvailAction = addAction(i18n("Preview not available"));

    setStyle(new ProxyStyle());

    connect(this, &KrPreviewPopup::triggered, this, &KrPreviewPopup::view);
}

void KrPreviewPopup::showEvent(QShowEvent *event)
{
    QMenu::showEvent(event);

    if (!jobStarted) {
        QStringList allPlugins = KIO::PreviewJob::availablePlugins();
        KIO::PreviewJob *pjob = new KIO::PreviewJob(files, QSize(MAX_SIZE, MAX_SIZE), &allPlugins);
        pjob->setScaleType(KIO::PreviewJob::ScaledAndCached);
        connect(pjob, &KIO::PreviewJob::gotPreview, this, &KrPreviewPopup::addPreview);
        jobStarted = true;
    }
}

void KrPreviewPopup::setUrls(const QList<QUrl> &urls)
{
    for (const QUrl &url : urls) {
        files.push_back(KFileItem(url));
    }
}

void KrPreviewPopup::addPreview(const KFileItem &file, const QPixmap &preview)
{
    if (prevNotAvailAction) {
        removeAction(prevNotAvailAction);
        delete prevNotAvailAction;
        prevNotAvailAction = nullptr;
    }

    QAction *act = addAction(file.text());
    act->setIcon(QIcon(preview));
    act->setData(QVariant::fromValue(file.url()));
}

void KrPreviewPopup::view(QAction *clicked)
{
    if (clicked && clicked->data().canConvert<QUrl>())
        KrViewer::view(clicked->data().value<QUrl>());
}
