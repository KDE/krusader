/*
    SPDX-FileCopyrightText: 2003-2004 Max Howell <max.howell@methylblue.com>
    SPDX-FileCopyrightText: 2004-2022 Krusader Krew <https://krusader.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "../../icon.h"
#include "fileTree.h"
#include "radialMap.h" //class Segment
#include "widget.h"

#include <cmath>

// QtCore
#include <QTimer> //::resizeEvent()
// QtGui
#include <QPaintEvent>
#include <QPainter>
// QtWidgets
#include <QApplication> //QApplication::setOverrideCursor()
#include <QMenu>

#include <KIO/DeleteJob>
#include <KIO/JobUiDelegate>
#include <KLocalizedString>
#include <KMessageBox>
#include <kio_version.h>
#include <kservice_version.h>

#if KIO_VERSION >= QT_VERSION_CHECK(5, 71, 0)
#include <KIO/OpenUrlJob>
#endif

#if KSERVICE_VERSION >= QT_VERSION_CHECK(5, 83, 0)
#include <KTerminalLauncherJob>
#else
#include <KToolInvocation>
#endif

void RadialMap::Widget::resizeEvent(QResizeEvent *)
{
    if (m_map.resize(rect())) {
        m_timer.setSingleShot(true);
        m_timer.start(500); // will cause signature to rebuild for new size
    }

    // always do these as they need to be initialised on creation
    m_offset.rx() = (width() - m_map.width()) / 2;
    m_offset.ry() = (height() - m_map.height()) / 2;
}

void RadialMap::Widget::paintEvent(QPaintEvent *)
{
    // bltBit for some Qt setups will bitBlt _after_ the labels are painted. Which buggers things up!
    // shame as bitBlt is faster, possibly Qt bug? Should report the bug? - seems to be race condition
    // bitBlt( this, m_offset, &m_map );

    QPainter paint(this);

    paint.drawPixmap(m_offset, m_map);

    // vertical strips
    if (m_map.width() < width()) {
        paint.eraseRect(0, 0, m_offset.x(), height());
        paint.eraseRect(m_map.width() + m_offset.x(), 0, m_offset.x() + 1, height());
    }
    // horizontal strips
    if (m_map.height() < height()) {
        paint.eraseRect(0, 0, width(), m_offset.y());
        paint.eraseRect(0, m_map.height() + m_offset.y(), width(), m_offset.y() + 1);
    }

    // exploded labels
    if (!m_map.isNull() && !m_timer.isActive())
        paintExplodedLabels(paint);
}

const RadialMap::Segment *RadialMap::Widget::segmentAt(QPoint &e) const
{
    // determine which segment QPoint e is above

    e -= m_offset;

    if (e.x() <= m_map.width() && e.y() <= m_map.height()) {
        // transform to cartesian coords
        e.rx() -= m_map.width() / 2; // should be an int
        e.ry() = m_map.height() / 2 - e.y();

        double length = std::hypot(e.x(), e.y());

        if (length >= m_map.m_innerRadius) { // not hovering over inner circle
            uint depth = ((int)length - m_map.m_innerRadius) / m_map.m_ringBreadth;

            if (depth <= m_map.m_visibleDepth) { //**** do earlier since you can //** check not outside of range
                // vector calculation, reduces to simple trigonometry
                // cos angle = (aibi + ajbj) / albl
                // ai = x, bi=1, aj=y, bj=0
                // cos angle = x / (length)

                auto a = (uint)(acos((double)e.x() / length) * 916.736); // 916.7324722 = #radians in circle * 16

                // acos only understands 0-180 degrees
                if (e.y() < 0)
                    a = 5760 - a;

#define ring (m_map.m_signature + depth)
                for (ConstIterator<Segment> it = ring->constIterator(); it != ring->end(); ++it)
                    if ((*it)->intersects(a))
                        return *it;
#undef ring
            }
        } else
            return m_rootSegment; // hovering over inner circle
    }

    return nullptr;
}

void RadialMap::Widget::mouseMoveEvent(QMouseEvent *e)
{
    // set m_focus to what we hover over, update UI if it's a new segment

    Segment const *const oldFocus = m_focus;
    QPoint p = e->pos();

    m_focus = segmentAt(p); // NOTE p is passed by non-const reference

    if (m_focus && m_focus->file() != m_tree) {
        if (m_focus != oldFocus) { // if not same as last time
            setCursor(QCursor(Qt::PointingHandCursor));
            m_tip.updateTip(m_focus->file(), m_tree);
            emit mouseHover(m_focus->file()->fullPath());

            // repaint required to update labels now before transparency is generated
            repaint();
        }

        // updates tooltip pseudo-transparent background
        m_tip.moveto(e->globalPosition().toPoint(), *this, (p.y() < 0));
    } else if (oldFocus && oldFocus->file() != m_tree) {
        unsetCursor();
        m_tip.hide();
        update();

        emit mouseHover(QString());
    }
}

void RadialMap::Widget::mousePressEvent(QMouseEvent *e)
{
    // m_tip is hidden already by event filter
    // m_focus is set correctly (I've been strict, I assure you it is correct!)

    if (m_focus && !m_focus->isFake()) {
        const QUrl url = Widget::url(m_focus->file());
        const bool isDir = m_focus->file()->isDir();

        if (e->button() == Qt::RightButton) {
            QMenu popup;
            popup.setTitle(m_focus->file()->fullPath(m_tree));

            QAction *actKonq = nullptr, *actKonsole = nullptr, *actViewMag = nullptr, *actFileOpen = nullptr, *actEditDel = nullptr;

            if (isDir) {
                actKonq = popup.addAction(Icon("system-file-manager"), i18n("Open File Manager Here"));
                if (url.scheme() == "file")
                    actKonsole = popup.addAction(Icon("utilities-terminal"), i18n("Open Terminal Here"));

                if (m_focus->file() != m_tree) {
                    popup.addSeparator();
                    actViewMag = popup.addAction(Icon("zoom-original"), i18n("&Center Map Here"));
                }
            } else
                actFileOpen = popup.addAction(Icon("document-open"), i18n("&Open"));

            popup.addSeparator();
            actEditDel = popup.addAction(Icon("edit-delete"), i18n("&Delete"));

            QAction *result = popup.exec(e->globalPosition().toPoint());
            if (result == nullptr)
                result = (QAction *)-1; // sanity

            if (result == actKonq) {
                // KJob jobs will delete themselves when they finish (see kjob.h for more info)
                auto *job = new KIO::OpenUrlJob(url, this);
                job->start();
            } else if (result == actKonsole) {
#if KSERVICE_VERSION >= QT_VERSION_CHECK(5, 83, 0)
                auto *job = new KTerminalLauncherJob(QString());
                job->setWorkingDirectory(url.url());
                job->start();
#elif KSERVICE_VERSION >= QT_VERSION_CHECK(5, 79, 0)
                KToolInvocation::invokeTerminal(QString(), QStringList(), url.url());
#else
                KToolInvocation::invokeTerminal(QString(), url.url());
#endif
            } else if (result == actViewMag || result == actFileOpen) {
                goto sectionTwo;
            } else if (result == actEditDel) {
                const QUrl url = Widget::url(m_focus->file());
                const QString message =
                    (m_focus->file()->isDir()
                         ? i18n("<qt>The folder at <i>'%1'</i> will be <b>recursively</b> and <b>permanently</b> deleted.</qt>", url.toDisplayString())
                         : i18n("<qt><i>'%1'</i> will be <b>permanently</b> deleted.</qt>", url.toDisplayString()));
                const int userIntention = KMessageBox::warningContinueCancel(this, message, QString(), KStandardGuiItem::del());

                if (userIntention == KMessageBox::Continue) {
                    KIO::Job *job = KIO::del(url);
                    auto *ui = dynamic_cast<KIO::JobUiDelegate *>(job->uiDelegate());
                    ui->setWindow(this);
                    connect(job, &KIO::Job::result, this, &Widget::deleteJobFinished);
                    QApplication::setOverrideCursor(Qt::BusyCursor);
                }
            } else {
                // ensure m_focus is set for new mouse position
                sendFakeMouseEvent();
            }

        } else {
        sectionTwo:

            const QPoint pos = e->position().toPoint();
            const QRect rect(pos.x() - 20, pos.y() - 20, 40, 40);

            m_tip.hide(); // user expects this

            if (!isDir || e->button() == Qt::MiddleButton) {
                auto *job = new KIO::OpenUrlJob(url, this);
                job->start();
            } else if (m_focus->file() != m_tree) { // is left mouse button
                emit activated(url); // activate first, this will cause UI to prepare itself
                if (m_focus)
                    createFromCache(dynamic_cast<const Directory *>(m_focus->file()));
            }
        }
    }
}

void RadialMap::Widget::deleteJobFinished(KJob *job)
{
    QApplication::restoreOverrideCursor();
    if (!job->error())
        invalidate();
    else
        job->uiDelegate()->showErrorMessage();
}
