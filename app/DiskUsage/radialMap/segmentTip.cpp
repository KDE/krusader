/*
    SPDX-FileCopyrightText: 2003-2004 Max Howell <max.howell@methylblue.com>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "segmentTip.h"

// QtCore
#include <QEvent>
// QtGui
#include <QPainter>
// QtWidgets
#include <QApplication> //installing eventFilters
#include <QScreen>
#include <QToolTip> //for its palette

#include <KLocalizedString>

#include "../compat.h"
#include "fileTree.h"

namespace RadialMap
{

SegmentTip::SegmentTip(uint h)
    : QWidget(nullptr, Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint)
    , m_cursorHeight(-h)
{
    setAttribute(Qt::WA_NoSystemBackground, true);
}

void SegmentTip::moveto(QPoint p, QWidget &canvas, bool placeAbove)
{
    //**** this function is very slow and seems to be visibly influenced by operations like mapFromGlobal() (who knows why!)
    //  ** so any improvements are much desired

    // TODO uints could improve the class
    p.rx() -= rect().center().x();
    p.ry() -= (placeAbove ? 8 + height() : m_cursorHeight - 8);

    const QRect screen = QApplication::primaryScreen()->geometry();

    const int x = p.x();
    const int y = p.y();
    const int x2 = x + width();
    const int y2 = y + height(); // how's it ever gunna get below screen height?! (well you never know I spose)
    const int sw = screen.width();
    const int sh = screen.height();

    if (x < 0)
        p.setX(0);
    if (y < 0)
        p.setY(0);
    if (x2 > sw)
        p.rx() -= x2 - sw;
    if (y2 > sh)
        p.ry() -= y2 - sh;

    // I'm using this QPoint to determine where to offset the bitBlt in m_pixmap
    QPoint offset = canvas.mapToGlobal(QPoint()) - p;
    if (offset.x() < 0)
        offset.setX(0);
    if (offset.y() < 0)
        offset.setY(0);

    const QRect alphaMaskRect(canvas.mapFromGlobal(p), size());
    const QRect intersection(alphaMaskRect.intersected(canvas.rect()));

    m_pixmap = QPixmap(size()); // move to updateTip once you are sure it can never be null
    // bitBlt( &m_pixmap, offset, &canvas, intersection, Qt::CopyROP );
    QPainter(&m_pixmap).drawPixmap(offset, canvas.grab(intersection));

    QColor col = QToolTip::palette().color(QPalette::Active, QPalette::Window);
    col.setAlpha(153); // 0.6

    QRect rct = rect();
    if (rct.width())
        rct.setWidth(rct.width() - 1);
    if (rct.height())
        rct.setHeight(rct.height() - 1);

    QPainter paint(&m_pixmap);
    paint.setPen(Qt::black);
    paint.setBrush(col);
    paint.drawRect(rct);
    paint.end();

    paint.begin(&m_pixmap);
    paint.drawText(rect(), Qt::AlignCenter, m_text);
    paint.end();

    p += screen.topLeft(); // for Xinerama users

    move(x, y);
    show();
    repaint();
}

void SegmentTip::updateTip(const File *const file, const Directory *const root)
{
    const QString s1 = file->fullPath(root);
    QString s2 = file->humanReadableSize();
    QLocale loc;
    const uint MARGIN = 3;
    const FileSize pc = 100 * file->size() / root->size();
    uint maxw = 0;
    uint h = fontMetrics().height() * 2 + 2 * MARGIN;

    if (pc > 0)
        s2 += QString(" (%1%)").arg(loc.toString(pc));

    m_text = s1;
    m_text += '\n';
    m_text += s2;

    if (file->isDir()) {
        double files = dynamic_cast<const Directory *>(file)->fileCount();
        const auto pc = uint((100 * files) / (double)root->fileCount());
        QString s3 = i18n("Files: %1", loc.toString(files, 'f', 0));

        if (pc > 0)
            s3 += QString(" (%1%)").arg(loc.toString(pc));

        maxw = fontMetrics().horizontalAdvance(s3);
        h += fontMetrics().height();
        m_text += '\n';
        m_text += s3;
    }

    uint w = fontMetrics().horizontalAdvance(s1);
    if (w > maxw)
        maxw = w;
    w = fontMetrics().horizontalAdvance(s2);
    if (w > maxw)
        maxw = w;

    resize(maxw + 2 * MARGIN, h);
}

bool SegmentTip::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::Show:
        qApp->installEventFilter(this);
        break;
    case QEvent::Hide:
        qApp->removeEventFilter(this);
        break;
    case QEvent::Paint: {
        // bitBlt( this, 0, 0, &m_pixmap );
        QPainter(this).drawPixmap(0, 0, m_pixmap);
        return true;
    }
    default:;
    }

    return false /*QWidget::event( e )*/;
}

bool SegmentTip::eventFilter(QObject *, QEvent *e)
{
    switch (e->type()) {
    case QEvent::Leave:
        //    case QEvent::MouseButtonPress:
        //    case QEvent::MouseButtonRelease:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::FocusIn:
    case QEvent::FocusOut:
    case QEvent::Wheel:
        hide(); // FALL THROUGH
    default:
        return false; // allow this event to passed to target
    }
}

} // namespace RadialMap
