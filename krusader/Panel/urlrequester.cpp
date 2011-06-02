/*****************************************************************************
 * Copyright (C) 2010 Jan Lepper <dehtris@yahoo.de>                          *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This package is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this package; if not, write to the Free Software               *
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA *
 *****************************************************************************/

#include "urlrequester.h"

#include "../defaults.h"
#include "../krglobal.h"

#include <KLineEdit>
#include <ksqueezedtextlabel.h>
#include <QPainter>
#include <QFont>
#include <QEvent>

class UrlRequester::PathLabel : public KSqueezedTextLabel
{
public:
    PathLabel (QWidget *parent, QLineEdit *le) : KSqueezedTextLabel(parent), _lineEdit(le) {}

    virtual void mousePressEvent(QMouseEvent *event) {
        hide();
        _lineEdit->show();
        _lineEdit->setFocus();
    }

    virtual void enterEvent(QEvent *event) {
        update();
    }

    virtual void leaveEvent(QEvent *event) {
        update();
    }

    virtual void paintEvent(QPaintEvent *event) {
        QPainter p(this);
        QStyleOption opt;
        opt.rect = rect();
        QRectF textRect;
        QRect r  = rect();
        r.adjust(0, 0, -5, 0); // reserve some space
        p.drawText(r, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, text(), &textRect);
        int lineX = (textRect.right()) < r.right() ? textRect.right() : r.right() + 3;

        if(underMouse()) {
            p.drawLine(lineX, textRect.top(), lineX, textRect.bottom());
            lineX++;
            p.drawLine(lineX, textRect.top(), lineX, textRect.bottom());
        }
    }

    QLineEdit *_lineEdit;
};


UrlRequester::UrlRequester(KLineEdit *le, QWidget *parent) : KUrlRequester(le, parent), _path(0)
{
    setMinimumHeight(sizeHint().height());

    if(KConfigGroup(krConfig, "Look&Feel").readEntry("FlatOriginBar", _FlatOriginBar)) {
        _path = new PathLabel(this, le);
        _path->setCursor(Qt::IBeamCursor);
        le->hide();
        connect(le, SIGNAL(textChanged ( const QString & )), SLOT(slotTextChanged(const QString&)));
    }
}

void UrlRequester::setActive(bool active) {
    if(_path) {
        QFont f = _path->font();
        f.setWeight(active ? QFont::Bold : QFont::Normal);
        _path->setFont(f);
    }
}

void UrlRequester::edit()
{
    if(_path)
        _path->hide();
    lineEdit()->show();
    lineEdit()->setFocus();
}

bool UrlRequester::eventFilter(QObject * watched, QEvent * e)
{
    if(_path && watched == lineEdit() && e->type() == QEvent::FocusOut) {
        lineEdit()->hide();
        _path->show();
    }

    return KUrlRequester::eventFilter(watched, e);
}

void UrlRequester::slotTextChanged(const QString &text) {
    if(_path)
        _path->setText(' ' + text + ' ');
}
